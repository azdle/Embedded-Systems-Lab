#include <p18f4520.h>
//#define PART_ONE  // Comment this out for part II of the lab

// PORTD =		1	E	R/W		RS		DB7	DB6	DB5	DB4	
// RD7 has to be 1 to be powered ON
// LCD 4-bit interface. High nibble transfered first
// check busy flag before sending an instruction

// LCD 1st line =  0 <-> F, 2nd line = 0x40 -> 0x4F


/*
I/D = 1: Increment
I/D = 0: Decrement
S = 1: Accompanies display shift
S/C = 1:  Display shift
S/C = 0: Cursor move
R/L = 1: Shift to the right
R/L = 0: Shift to the left
DL = 1: 8 bits, DL = 0:  4 bits
N = 1: 2 lines, N = 0:  1 line
F = 1: 5 × 10 dots, F = 0:  5 × 8 dots
BF = 1: Internally operating
BF = 0:  Instructions acceptable
DDRAM: Display data RAM
CGRAM: Character generator
RAM
ACG: CGRAM address
ADD: DDRAM address (corresponds to cursor address)
AC: Address counter used for both DD and CGRAM addresses

*/
/*
to  correctly  read  data,
execute either the address set instruction or cursor shift instruction (only with DDRAM), then just
before  reading  the  desired  data,  execute  the  read  instruction  from  the  second  time  the  read
instruction is sent.
*/

// ----------------------------------- DEFINITIONS -----------------------------------

/*---------- LCD ------------*/
#define ON 		PORTDbits.RD7
#define EN 		PORTDbits.RD6
#define RW	 	PORTDbits.RD5
#define RS 		PORTDbits.RD4
#define DB4 	PORTDbits.RD0
#define DB5 	PORTDbits.RD1
#define DB6 	PORTDbits.RD2
#define DB7 	PORTDbits.RD3
/*---------------------------*/

/*--- EEPROM, using MSSP ----*/

#ifdef PART_ONE
	#define SS 		PORTAbits.RA5
	#define SDI 	PORTCbits.RC4
	#define SDO 	PORTCbits.RC5
	#define SCK 	PORTCbits.RC3
	
	#define TRIS_SS 	TRISAbits.TRISA5
	#define TRIS_SCK 	TRISCbits.TRISC3
	#define TRIS_SDI 	TRISCbits.TRISC4
	#define TRIS_SDO 	TRISCbits.TRISC5
#else
	#define SS 		PORTBbits.RB3
	#define SDI 	PORTBbits.RB2
	#define SDO 	PORTBbits.RB1
	#define SCK 	PORTBbits.RB0
	
	#define TRIS_SS 	TRISBbits.TRISB3
	#define TRIS_SDI 	TRISBbits.TRISB2
	#define TRIS_SDO 	TRISBbits.TRISB1
	#define TRIS_SCK 	TRISBbits.TRISB0
#endif
	
	#define WREN	0b00000110          // WREN command
	#define WRDI    0b00000100          // WRDI command
	#define WRITE   0b00000010          // WRITE command
	#define READ    0b00000011          // READ command
	#define WRSR    0b00000001          // WRSR command
	#define RDSR    0b00000101          // RDSR command

/*---------------------------*/

// ----------------------------------- VARIABLES --------------------------------------------
unsigned int eeprom_address = 0;

char inbuffer[16];
unsigned char infront = 0;
unsigned char inback = 0;

char outbuffer[16];
unsigned char outfront = 0;
unsigned char outback = 0;

unsigned char cursor = 0; // cursor position

void hi_isr(void); 
void lo_isr(void); 
void setup(void);


// ----------------------------------- FUNCTIONS ----------------------------------------------

/* USART functions */
unsigned char receiveChar(void);
void sendChar(unsigned char c);

/* SPI functions */
unsigned char readEEPROM(unsigned char command, unsigned short int address);
void writeEEPROM(unsigned char command, unsigned short int address, unsigned char data);
void writeEnable(void);
void byteout(unsigned char byte);
unsigned char bytein(void);

/* LCD functions */
void enable(void);
void initLCD(void);
void insertChar(unsigned char c);
void checkBusy(void);
void endLineCheck(void);

/* Delay functions */
void wait1ms(void);
void waitms(unsigned short int ms);

// ---------------------------------------- CODE -----------------------------------

#pragma code hi_isr_entry=8
void hi_isr_entry(void){
    _asm goto hi_isr _endasm
}
	
#pragma code lo_isr_entry=0x18
void lo_isr_entry(void){
    _asm goto lo_isr _endasm
}

#pragma interrupt hi_isr
void hi_isr(void){

    inbuffer[infront++] = RCREG; // clears the flag that caused the interrupt, too
    infront &= 0x0f;

}

#pragma interruptlow lo_isr
void lo_isr(void){
	if (outfront == outback){
		PIE1bits.TXIE = 0;
	}
	else{
	  	TXREG = outbuffer[outback++]; // clears TXREG, temporarily
	   	outback &= 0x0f;
	}
}

void enable(void){
	EN = 1;
	_asm
		nop
		nop
	_endasm
	EN = 0;
}

/* initialization based on p.213 in LCD datasheet */
void initLCD(void){ 
	ON = 1;
	waitms(20);
	PORTD = 0b10000011; // Function set (interface 8-bit)
	enable();
	waitms(8);
	enable();
	waitms(2);
	PORTD = 0b10000010; // Function set  (Set interface to be 4 bits long.) Interface is 8 bits in length.
	enable();
	PORTD = 0b10000010; // 2 lines, 5x8 dots
	enable();
	PORTD = 0b10001000; // 
	enable();
	PORTD = 0b10000000; // Display off
	enable();
	PORTD = 0b10001000; // 
	enable();
	PORTD = 0b10000000; // Display clear
	enable();
	PORTD = 0b10000001; // 
	enable();
	PORTD = 0b10000000; // Cursor incrementing, no shift
	enable();
	PORTD = 0b10000110; // 
	enable();
	waitms(3);
	PORTD = 0b10000000; // Display on, cursor on, blinking cursor
	enable();
	PORTD = 0b10001110; // 
	enable();
	waitms(5);
}

void insertChar(unsigned char c){
	unsigned char lowNibble = c & 0b00001111;

	PORTD = 0b10010000 | (c>>4); // High order byte
	enable();
	PORTD = 0b10010000 | lowNibble; // Low order byte
	enable();
	waitms(5);
}

/* determines the position of the cursor */
void endLineCheck(void){
	if (cursor == 0x10){ // end of first line. Cursor jumps to the second line (0x40)
		PORTD = 0b10001100; 
		enable();
		PORTD = 0b10000000; 
		enable();
		waitms(5);
		cursor = 0x41;
	}
	else if	(cursor >= 0x50){ // end of second line. Display cleared and cursor jumps to starting position
		PORTD = 0b10000000; 
		enable();
		PORTD = 0b10000001; 
		enable();
		waitms(5);
		cursor = 0x01;
	}
	else cursor++;
	waitms(5);
}
// delay 1 ms
void wait1ms(void)
{
	unsigned long int u = 169;

	while (u--) ;
}

// delays ms number of milliseconds
void waitms(unsigned short int ms)
{
	while (ms--)
	{
		wait1ms();
	}
}

void setup(void){
	
	unsigned char tmp;
	
	// Set LCD interface
	TRISD = 0;
	PORTD = 0;
	initLCD();
	// ---------------------------
	
	// UART settings
	RCSTA = 0x90; // assume 8 bits, enable USART, enable rcvr.
	TXSTA = 0x24; // enable xmitter, set BRGH	
	BAUDCONbits.BRG16 = 1; 	
	// set Baud rate
	// Fosc/(4*(n+1))
	SPBRGH = 0;
	SPBRG = 207;   // Baud 19200	
	if (PIR1bits.RCIF) tmp = RCREG;// clears flag;
	// ----------------------------
	
	// interrupts
	IPR1bits.RCIP = 1; // receive will be high priority
	IPR1bits.TXIP = 0; // transmit will be low priority
	
	RCONbits.IPEN = 1; // enable prioritized interrupts    
	
	PIE1bits.RCIE = 1;
	PIE1bits.TXIE = 1; 
	
	INTCONbits.GIEH = 1; 
	INTCONbits.GIEL = 1; 
	// -----------------------------

	// SPI

	TRIS_SDO = 0;	//SDO
	TRIS_SDI = 1;	//SDI 
	TRIS_SCK = 0;	//SCK
	TRIS_SS = 0; 	// /ss

	#ifdef PART_ONE
		SSPCON1 = 0b00110000;	// fastest clock master mode
		SSPSTATbits.CKE = 0;
	#else
		SDO = 0;
		SCK = 0;
		SS = 1;
	#endif
	// -----------------------------
	
}

unsigned char receiveChar(void){
	unsigned char tmp;
	
	while (infront == inback) ; // no character yet.
	tmp = inbuffer[inback++];
	inback &= 0x0f;
	return tmp;
}

void sendChar(unsigned char c){
	outbuffer[outfront++] = c;
	outfront &= 0x0f;
	PIE1bits.TXIE = 1;
}

void writeEnable(void){
	unsigned char garbage;

	SS = 0;		// chip select
	#ifdef PART_ONE
		PIR1bits.SSPIF = 0;
		SSPBUF = WREN;	
		while(!PIR1bits.SSPIF) ;
		garbage = SSPBUF;
	#else         
    	byteout(WREN);                  
	#endif
	SS =1 ;
	waitms(10);
}

unsigned char readEEPROM(unsigned char command, unsigned short int address){

	unsigned char data;

	SS = 0;
	
	#ifdef PART_ONE
		// command
		PIR1bits.SSPIF = 0;
		SSPBUF = command;	
		while(!PIR1bits.SSPIF) ;
		data = SSPBUF;	// data is garbage at this point
		// ------------
		
		// address
		if(address != -1){
			PIR1bits.SSPIF = 0;
			SSPBUF = address >> 8;	// MSB
			while(!PIR1bits.SSPIF) ;
			data = SSPBUF; // data is garbage at this point
			
			PIR1bits.SSPIF = 0;
			SSPBUF = address;		// LSB
			while(!PIR1bits.SSPIF) ;
			data = SSPBUF; // data is garbage at this point
		}
		// -------------
	
		// data
		PIR1bits.SSPIF = 0;
		SSPBUF = data;	// sending garbage
		while(!PIR1bits.SSPIF) ;
		data = SSPBUF;
		// -------------
	#else
	    byteout(command);               // Output command & MSb of address
	    byteout((unsigned char)(address >> 8));          // Output MSB of address
		byteout((unsigned char)address); 				// Output LSB of address
	    data = bytein();        
	#endif
	SS = 1 ;

	return data;
}
// use pass in address = -1 to write status register
void writeEEPROM(unsigned char command, unsigned short int address, unsigned char data){

	unsigned char garbage;
	
	writeEnable();
	SS = 0;
	
	#ifdef PART_ONE
		// command
		PIR1bits.SSPIF = 0;
		SSPBUF = command;	
		while(!PIR1bits.SSPIF) ;
		garbage = SSPBUF;
		// ------------
		
		// address
		if(address != -1){
			PIR1bits.SSPIF = 0;
			SSPBUF = address >> 8;	// MSB
			while(!PIR1bits.SSPIF) ;
			garbage = SSPBUF;
			
			PIR1bits.SSPIF = 0;
			SSPBUF = address;		// LSB
			while(!PIR1bits.SSPIF) ;
			garbage = SSPBUF;
		}
		// -------------
	
		// data
		PIR1bits.SSPIF = 0;
		SSPBUF = data;	
		while(!PIR1bits.SSPIF) ;
		garbage = SSPBUF;
		// -------------
	#else
	    byteout(command);               
	    byteout((unsigned char)(address >> 8));          // Output MSB of address
		byteout((unsigned char)address); 				// Output LSB of address
	    byteout(data);                  				// Write byte of data
		waitms(10);
	#endif

	SS = 1 ;
}

void byteout(unsigned char byte)
{
    static unsigned char i;         // Loop counter

    SCK = 0;                        // Ensure SCK is low
    for (i = 0; i < 8; i++)         // Loop through each bit
    {
        if (byte & 0x80)            // Check if next bit is a 1
        {
            SDO = 1;                 // If a 1, pull SO high
        }
        else
        {
            SDO = 0;                 // If a 0, pull SO low
        }
        SCK = 1;                    // Bring SCK high to latch bit
        _asm nop _endasm;       
        SCK = 0;                    // Bring SCK low for next bit
        byte = byte << 1;           // Shift byte left for next bit
    }
} 

unsigned char bytein(void)
{
    static unsigned char i;         // Loop counter
    static unsigned char retval;    // Return value

    retval = 0;
    SCK = 0;                        // Ensure SCK is low
    for (i = 0; i < 8; i++)         // Loop through each bit
    {
        retval = retval << 1;       // Shift byte left for next bit
        SCK = 1;                    // Bring SCK high
        if (SDI == 1)                // Check if next bit is a 1
        {
            retval |= 0x01;         // If a 1, set next bit to 1
        }
        else
        {
            retval &= 0xFE;         // If a 0, set next bit to 0
        }
        SCK = 0;                    // Bring SCK low for next bit
    }

    return retval;
} 

void main(void){
	unsigned char c;
	unsigned char i;
    unsigned char eeprom_data = 0;

    setup();

	// reading from eeprom at RESET
	for(i=0; i<30;i++){
			eeprom_data = readEEPROM(READ,i);
			endLineCheck();
			insertChar(eeprom_data); // to LCD
	}

	// Receiving chars, echoing them in LCD and hyperterminal
	while (1){
		
		c = receiveChar();
	
		writeEEPROM(WRITE, eeprom_address, c);
		waitms(5);
		//eeprom_data = readEEPROM(READ,eeprom_address);
		eeprom_address++;

		//endLineCheck();
		//insertChar(eeprom_data); // to LCD

		sendChar(c); // to hyperterminal
	}
		
}
