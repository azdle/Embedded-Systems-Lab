#include <p18f4520.h>

//#pragma config IESO=ON, WDT=OFF, FCMEN=OFF
//#pragma config BOREN=OFF, PWRT=ON, LVP=OFF, OSC=HSPLL
//#pragma config MCLRE=ON, STVREN=ON, LPT1OSC=ON, PBADEN=OFF

//######### Pin Mappings #########################
#define LCD_E		PORTDbits.RD6
#define LCD_RW		PORTDbits.RD6
#define LCD_RS		PORTDbits.RD6
#define	LCD_POWER	PORTDbits.RD6
#define LCD_DB4		PORTDbits.RD6
#define LCD_DB5		PORTDbits.RD6
#define LCD_DB6		PORTDbits.RD6
#define LCD_DB7		PORTDbits.RD6

//######### Function Declarations ################
void high_isr(void);
void low_isr(void);
void setup(void);

//######### Variables ############################
volatile unsigned char txbuf;
volatile unsigned char rxbuf;

//######### Interrupts ###########################
#pragma code high_isr_entry=8
void high_isr_entry(void){
	_asm goto high_isr _endasm
}

#pragma code low_isr_entry=0x18
void low_isr_entry(void){
	_asm goto low_isr _endasm
}
#pragma code

#pragma interrupt high_isr
void high_isr(void){
	if(PIR1bits.RCIF == 1){
		//Receive byte
		rxbuf = RCREG;
		PIR1bits.RCIF == 0;
	}
	
	if(PIR1bits.TXIF == 1 && rxbuf != 0){
		//Trasnmit Byte1
		TXREG = rxbuf;
		rxbuf = 0;
		PIR1bits.TXIF == 0;
	}	
}

#pragma interruptlow low_isr
void low_isr(void){
	
}

//######### Functions ############################

void setup(){
//SYNC = 0, BRGH = 1, BRG16 = 1 (for 8Mhz)
	TXSTA = 0b00100100;
	RCSTA = 0b10010000;
	BAUDCON = 0b00001000;
	SPBRGH = 206 >> 8;
	SPBRG = 206;
	
	PIE1bits.RCIE = 1;
	PIE1bits.TXIE = 1;
	INTCONbits.PEIE = 1;
	INTCONbits.GIE = 1;
}


void main(void) { 
	//TRISB = 0x00;
	//PORTB = 0xFF; 
	//while(1);
	setup();
	while(1)	{
	}
}


