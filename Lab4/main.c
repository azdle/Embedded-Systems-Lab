#include <p18f4520.h>

/*--- SPI, using MSSP ----*/
#define SS 		PORTAbits.RA5
#define SDI 	PORTCbits.RC4
#define SDO 	PORTCbits.RC5
#define SCK 	PORTCbits.RC3

#define TRIS_SS 	TRISAbits.TRISA5
#define TRIS_SCK 	TRISCbits.TRISC3
#define TRIS_SDI 	TRISCbits.TRISC4
#define TRIS_SDO 	TRISCbits.TRISC5

#define CAN_RESET			0b11000000
#define CAN_READ			0b00000011
#define CAN_READ_RX_BUFFER	0b10010000	//0b10010nm0
#define CAN_WRITE			0b00000010
#define CAN_WRTIE_TX_BUFFER	0b01000000	//0b01000abc
#define CAN_RTS				0b10000000	//0b10000nnn
#define CAN_READ_STATUS		0b10100000
#define CAN_RX_STATUS		0b10110000
#define CAN_BIT_MODIFY		0b00000101
/*---------------------------*/

unsigned char CAN_Setup_Array[] =
{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x7e, 0x00, 0xff,
 0xff, 0x3c, 0x00, 0x80, 0x80, 0x7e, 0x20, 0xff, 0xff, 0x7e, 0x40,
 0xff, 0xff, 0x7e, 0x50, 0xff, 0xff, 0x00, 0x00, 0x80, 0x80, 0xff,
 0xff, 0xff, 0xff, 0x7e, 0x00, 0x00, 0x00, 0x02, 0x90, 0x03, 0x22,
 0x00, 0x00, 0x80, 0x80, 0x03, 0x7e, 0x00, 0x00, 0x00, 0x01, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x03, 0x7e,
 0xe0, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x80, 0x80, 0x03, 0x7e, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, 0x20, 0x00, 0x00,
 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
 0x80, 0x80, 0x20};

char inbuffer[16];
unsigned char infront = 0;
unsigned char inback = 0;

char outbuffer[16];
unsigned char outfront = 0;
unsigned char outback = 0;

void hi_isr(void); 
void lo_isr(void); 
void setup(void);

/* USART functions */
unsigned char receiveChar(void);
void sendChar(unsigned char c);

/* SPI functions */
unsigned char rwSPI( unsigned char data );
void setupCAN( void );

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
	}else{
	  	TXREG = outbuffer[outback++]; // clears TXREG, temporarily
	   	outback &= 0x0f;
	}
}

// delay 1 ms
void wait1ms(void){
	unsigned long int u = 169;
	while (u--) ;
}

// delays ms number of milliseconds
void waitms(unsigned short int ms){
	while (ms--){
		wait1ms();
	}
}

void setup(void){
	
	unsigned char tmp;
	
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
	TRIS_SDI = 1;	//SDI controller by SPI module
	TRIS_SCK = 0;	//SCK
	TRIS_SS = 0; 	// /ss

	SSPCON1 = 0b00110000;	// fastest clock master mode
	SSPSTATbits.CKE = 0;
	// -----------------------------

	setupCAN();
	
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

void setupCAN(void){
	unsigned int i;
	unsigned char data;
	SS = 0;
	rwSPI(CAN_RESET);
	SS = 1;

	SS = 0;
	rwSPI(CAN_WRITE);
	rwSPI(0x00);
	for(i = 0; i < (sizeof(CAN_Setup_Array) / sizeof(char)); i++){
		rwSPI(CAN_Setup_Array[i]);
	}
	SS = 1;

	SS = 0;
	rwSPI(CAN_WRITE);
	rwSPI(0x0F);
	rwSPI(0b01011000); // Loopback Mode, Abort Pending, OneShot Mode, No Clock Out
	SS = 1;

	// Read Just for Fun

	SS = 0;
	rwSPI(CAN_READ);
	rwSPI(0x0E);
	data = rwSPI(0x00); 
	SS = 1;
	sendChar(data);
}

unsigned char rwSPI( unsigned char data ){
	PIR1bits.SSPIF = 0;
	SSPBUF = data;	
	while(!PIR1bits.SSPIF) ;
	data = SSPBUF;
	return data;
}

void main(void){
	unsigned char c;

    setup();

	while (1){
		// Get Character from UART
		c = receiveChar();
	
		// Send Frame to CAN Bus

		// Wait for Reply from CAN Bus

		// Get Reply from CAN Bus

		// Send back to UART
		sendChar(c);
	}
		
}
