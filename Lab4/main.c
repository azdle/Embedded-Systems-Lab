#include <p18f4520.h>

/*++++++++++++++++++++++++++++++++++++  D E F I N I T I O N S +++++++++++++++++++++++++++++++++++*/

/*--- SPI, using MSSP ----*/
#define SS 		PORTAbits.RA5
#define SDI 	PORTCbits.RC4
#define SDO 	PORTCbits.RC5
#define SCK 	PORTCbits.RC3

#define TRIS_SS 	TRISAbits.TRISA5
#define TRIS_SCK 	TRISCbits.TRISC3
#define TRIS_SDI 	TRISCbits.TRISC4
#define TRIS_SDO 	TRISCbits.TRISC5
/*---------------------------*/


/*--- MCP2515, CAN ---*/
#define CAN_RESET			0b11000000
#define CAN_WRITE			0b00000010
#define CAN_RTS				0b10000000	//0b10000nnn
#define CAN_READ_STATUS     0b10100000


#define CAN_REG_CNF1		0x2a
#define CAN_REG_CNF2		0x29
#define CAN_REG_CNF3		0x28
#define CAN_TXB0CTRL		0x30
#define CAN_RXB0CTRL		0x60
#define CAN_REG_RXB0DLC		0x65	// rx data length
#define CAN_REG_TXB0DLC		0x3A	//tx data length
#define CAN_CANCTRL			0x0F
/*---------------------------*/

/*________________________________________________________________________________________________________*/

/*+++++++++++++++++++++++++++++++++  G L O B A L   V A R I A B L E S  ++++++++++++++++++++++++++++++++++++*/

/*--- USART ---*/
char inbuffer[16];
unsigned char infront = 0;
unsigned char inback = 0;

char outbuffer[16];
unsigned char outfront = 0;
unsigned char outback = 0;
/*---------------------------*/

/*________________________________________________________________________________________________________*/


/*++++++++++++++++++++++++++++++++++++  F U N C T I O N  D E F I N I T I O N S ++++++++++++++++++++++++++++++++*/

/* GENERAL */
void setup(void);

/* INTERRUPTS */
void hi_isr(void);
void lo_isr(void);

/* USART functions */
unsigned char receiveChar(void);
void sendChar(unsigned char c);

/* SPI functions */
unsigned char rwSPI( unsigned char data );
void setupCAN( void );
unsigned char CANStatus( );
void SPIWrite(unsigned char addr, unsigned char data);
void CANTransmit (unsigned char ch);
unsigned char CANReceive(void);

/* Delay functions */
void wait1ms(void);
void waitms(unsigned short int ms);
/*________________________________________________________________________________________________________________*/

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++  C O D E  +++++++++++++++++++++++++++++++++++++++++++++++*/

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

    SSPCON1 = 0x20;	// fastest clock master mode, SPI mode (0, 0)
    SSPSTAT = 0x40;	// CKE = 0, SMP = 0

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

    // Perform RESET and Wait
    SS = 0;
    rwSPI(CAN_RESET);
    SS = 1;

	SPIWrite(CAN_REG_CNF1,0x03);
	SPIWrite(CAN_REG_CNF2,0x90);
	SPIWrite(CAN_REG_CNF3,0x02);
	SPIWrite(CAN_REG_RXB0DLC, 0x01);
	SPIWrite(CAN_REG_TXB0DLC, 0x01);
    SPIWrite(CAN_TXB0CTRL, 0x00);
    SPIWrite(CAN_RXB0CTRL, 0x60);
    SPIWrite(CAN_CANCTRL, 0x40);        // LOOPBACK MODE

}

unsigned char CANStatus( ){
    unsigned char data;

    SS = 0;
    rwSPI(CAN_READ_STATUS);
    data = rwSPI(0);
    SS = 1;

    return data;
}

unsigned char rwSPI( unsigned char data ){
    SSPBUF = data;
    while(!SSPSTATbits.BF);
    data = SSPBUF;

    return data;
}

void SPIWrite(unsigned char addr, unsigned char data){

    SS = 0;
    rwSPI(CAN_WRITE);
    rwSPI(addr);
    rwSPI(data);
    SS = 1;
}

void CANTransmit (unsigned char c)
{
    SS = 0;
    rwSPI(0x41); // Load TXB0
    rwSPI(c);
    SS = 1;

    SS = 0;
    rwSPI(0x81);   //Request to Transfer TXB0
    SS = 1;
}

unsigned char CANReceive(void){
    unsigned char data;

    SS = 0;
    rwSPI(0x92); // Receive on RXB0
    data = rwSPI(0);
    SS = 1;

    return data;
}

void main(void){
    unsigned char c,x;

    setup();
    while (1){
        waitms(10);
        c = receiveChar();
        CANTransmit(c);
        c = CANStatus();

        while((c&0x01) == 0){
            c = CANStatus();
        }

        x = CANReceive();
        sendChar(x);
    }
}
