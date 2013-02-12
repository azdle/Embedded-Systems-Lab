#include <p18f4520.h>

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

// INSERT BUSY FLAG CHECK everywhere

#define ON PORTDbits.RD7
#define EN PORTDbits.RD6
#define READ PORTDbits.RD5
#define RS PORTDbits.RD4
#define DB4 PORTDbits.RD0
#define DB5 PORTDbits.RD1
#define DB6 PORTDbits.RD2
#define DB7 PORTDbits.RD3

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

/* USART functions */
unsigned char receiveChar(void);
void sendChar(unsigned char c);

/* LCD functions */
void enable(void);
void initLCD(void);
void insertChar(unsigned char c);
void checkBusy(void);
void endLineCheck(void);

/* Delay functions */
void wait1ms(void);
void waitms(unsigned short int ms);


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
	waitms(1);
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

	
	// set RCSTA bits
	RCSTA = 0x90; // assume 8 bits, enable USART, enable rcvr.
	
	// set TXSTA bits
	TXSTA = 0x24; // enable xmitter, set BRGH
	
	BAUDCONbits.BRG16 = 1; 
	
	// set Baud rate
	// Fosc/(4*(n+1))
	SPBRGH = 0;
	SPBRG = 207;   // Baud 19200
	
	if (PIR1bits.RCIF) tmp = RCREG;// clears flag;
	
	// enable device to generate interrupts
	
	IPR1bits.RCIP = 1; // receive will be high priority
	IPR1bits.TXIP = 0; // transmit will be low priority
	
	RCONbits.IPEN = 1; // enable prioritized interrupts    
	
	PIE1bits.RCIE = 1;
	PIE1bits.TXIE = 1; 
	
	INTCONbits.GIEH = 1; 
	INTCONbits.GIEL = 1; 
	
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

void main(void){
	unsigned char c;
    setup();
	while (1){
		c = receiveChar();
		endLineCheck();
		insertChar(c); // to LCD
		sendChar(c); // to hyperterminal
	}
		
}