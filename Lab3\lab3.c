/*
Participants: Quoc Anh Tran, Patrick Barrett
IDs:    451 7669, 381 3525
Course: EE4341
Date:  2/15/2013
*/

#include <p18f4520.h>
#include <i2c.h>
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

/* ------ */

#define ON PORTDbits.RD7
#define EN PORTDbits.RD6
#define READ PORTDbits.RD5
#define RS PORTDbits.RD4
#define DB4 PORTDbits.RD0
#define DB5 PORTDbits.RD1
#define DB6 PORTDbits.RD2
#define DB7 PORTDbits.RD3


unsigned char cursor = 0; // cursor position
unsigned char entryIndex = 1;
unsigned int counter = 0;
unsigned char data[6] = {0,0,0,0,0,0};
unsigned char digits[6] = {0,0,0,0,0,0};
unsigned char lookUp[10] = {0x30, 0x31, 0x32, 0x33, 0x34,
							0x35, 0x36, 0x37, 0x38, 0x39};

void hi_isr(void); 
void lo_isr(void); 
void setup(void);


/* I2C functions */
void HDByteWriteI2C( unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char data);
void HDByteReadI2C( unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char *data, unsigned char length );

/* EEPROM functions */
void updateDigits(void);

/* LCD functions */
void enable(void);
void initLCD(void);
void insertChar(unsigned char c);
void checkBusy(void);
void endLineCheck(void);
void setCursor(unsigned char position);

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
	
	HDByteWriteI2C(0xa0, 0, 0, lookUp[digits[0]]);
	HDByteWriteI2C(0xa0, 0, 1, lookUp[digits[1]]);	
	HDByteWriteI2C(0xa0, 0, 2, lookUp[digits[2]]);
	HDByteWriteI2C(0xa0, 0, 3, lookUp[digits[3]]);	
	HDByteWriteI2C(0xa0, 0, 4, lookUp[digits[4]]);
	HDByteWriteI2C(0xa0, 0, 5, lookUp[digits[5]]);		
}

#pragma interruptlow lo_isr
void lo_isr(void){

	static unsigned char interval = 0;
		
		PIR1bits.TMR2IF = 0;

		if (interval == 19){
			counter++;
			if(counter == 100) counter = 0;
			digits[1] = counter/10;
			digits[0] = counter-(10*digits[1]);

			setCursor(0);
			insertChar(lookUp[digits[1]]);
			insertChar(lookUp[digits[0]]); 

			interval = 0;
		} else interval++;	

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

	PORTD = 0b10010000 | (c>>4); // High order nibble
	enable();
	PORTD = 0b10010000 | lowNibble; // Low order nibble
	enable();
	waitms(5);
}

/* determines the position of the cursor */
void setCursor (unsigned char position){
	if(position == 0){
		PORTD = 0b10000000; 
		enable();
		PORTD = 0b10000010; 
		enable();
		waitms(5);
	}
	else{	
		PORTD = 0b10001000 | ((position & 0b01110000)>>4); 
		enable();
		PORTD = 0b10000000 | (position & 0b00001111); 
		enable();
		waitms(5);
	}
}
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
	TRISBbits.TRISB3 = 0;
	PORTBbits.RB3 = 0;

	RCONbits.IPEN = 1; // enable prioritized interrupts

	// Set LCD interface
	TRISD = 0;
	PORTD = 0;
	initLCD();	

	// Timer2
	PR2 = 250;
	TMR2 = 0;
	T2CON = 0x4b; // 256*16*10 = 40096 = 10ms
	IPR1bits.TMR2IP = 0;
	PIR1bits.TMR2IF = 0;
	PIE1bits.TMR2IE = 1;
	T2CONbits.TMR2ON = 1;

	// I2C config
	DDRCbits.RC3 = 1; //Configure SCL as Input
	DDRCbits.RC4 = 1; //Configure SDA as Input

 	
	SSPSTAT = 0x80;   //Disable SMBus 
  	SSPCON1 = 0x28;   //Enable MSSP Master
  	SSPADD = 0x27;    //Should be 0x18
 	SSPCON2 = 0x00;   //Clear MSSP Conrol Bits

	// Button settings
	TRISAbits.TRISA4 = 1;
	
	// HLVD settings
	PIR2bits.HLVDIF = 0;
	IPR2bits.HLVDIP = 1;
	PIE2bits.HLVDIE = 1;
	HLVDCON = 0x3E;
	
	// enable device to generate interrupts
	IPR1bits.RCIP = 1; // receive will be high priority
	IPR1bits.TXIP = 0; // transmit will be low priority
	PIE1bits.RCIE = 1;
	PIE1bits.TXIE = 1; 
	
	// General interrupt control registers
	INTCONbits.GIEH = 1; 
	INTCONbits.GIEL = 1;
	
}

void HDByteWriteI2C( unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char data )
{
  IdleI2C();                      // ensure module is idle
  StartI2C();                     // initiate START condition
  while ( SSPCON2bits.SEN );      // wait until start condition is over 
  WriteI2C( ControlByte );        // write 1 byte - R/W bit should be 0
  IdleI2C();                      // ensure module is idle
  WriteI2C( HighAdd );            // write address byte to EEPROM
  IdleI2C();                      // ensure module is idle
  WriteI2C( LowAdd );             // write address byte to EEPROM
  IdleI2C();                      // ensure module is idle
  WriteI2C ( data );              // Write data byte to EEPROM
  IdleI2C();                      // ensure module is idle
  StopI2C();                      // send STOP condition
  while ( SSPCON2bits.PEN );      // wait until stop condition is over 
  while (EEAckPolling(ControlByte));  //Wait for write cycle to complete
}

void HDByteReadI2C( unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char *data, unsigned char length )
{
  IdleI2C();                      // ensure module is idle
  StartI2C();                     // initiate START condition
  while ( SSPCON2bits.SEN );      // wait until start condition is over 
  WriteI2C( ControlByte );        // write 1 byte 
  IdleI2C();                      // ensure module is idle
  WriteI2C( HighAdd );            // WRITE word address to EEPROM
  IdleI2C();                      // ensure module is idle
  while ( SSPCON2bits.RSEN );     // wait until re-start condition is over 
  WriteI2C( LowAdd );             // WRITE word address to EEPROM
  IdleI2C();                      // ensure module is idle
  RestartI2C();                   // generate I2C bus restart condition
  while ( SSPCON2bits.RSEN );     // wait until re-start condition is over 
  WriteI2C( ControlByte | 0x01 ); // WRITE 1 byte - R/W bit should be 1 for read
  IdleI2C();                      // ensure module is idle
  getsI2C( data, length );       // read in multiple bytes
  NotAckI2C();                    // send not ACK condition
  while ( SSPCON2bits.ACKEN );    // wait until ACK sequence is over 
  StopI2C();                      // send STOP condition
  while ( SSPCON2bits.PEN );      // wait until stop condition is over 
}

void updateDigits(void){
		digits[2] = data[0]-0x30;
		digits[3] = data[1]-0x30;
		digits[4] = data[2]-0x30;
		digits[5] = data[3]-0x30;
}


void main(void){
	unsigned char c;
	unsigned char i;
    setup();

	HDByteReadI2C(0xa0, 0, 0, (unsigned char*)data, 6);
	counter = 10*(data[1]-0x30) + data[0]-0x30;

	updateDigits();
	

	while (1){
		while(PORTAbits.RA4 == 1);
		waitms(10);
		INTCONbits.GIEL = 0;
		setCursor(0x40);
		switch(entryIndex){
			case 1: 
				insertChar(data[1]);
				insertChar(data[0]); 	
				entryIndex = 2;
				break;
			case 2: 
				insertChar(data[3]);
				insertChar(data[2]);
				entryIndex = 3;
				break;
			case 3: 
				insertChar(data[5]);
				insertChar(data[4]);
				entryIndex = 1;
				break;
		}
		INTCONbits.GIEL = 1;
		while (PORTAbits.RA4 == 0);
		waitms(10);

	}
		
}
