/*
Participants: Quoc Anh Tran, Patrick Barrett
IDs: 451 7669, 381 3525
Course: EE4341
Date: 3/11/2013
*/
#define MY_I2C // Comment this line out to use provided i2c functions.

#include <p18f4520.h>
#ifndef MY_I2C
#include <i2c.h>
#endif

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
unsigned char counter = 0;
unsigned char counts[4] = {0};

void hi_isr(void);
void lo_isr(void);
void setup(void);


/* I2C functions */
void ByteWriteI2C( unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char data);
void ByteReadI2C( unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char *data, unsigned char length );

#ifdef MY_I2C
void IdleI2C();
void StartI2C();
void RestartI2C();
void StopI2C();
void WriteI2C( unsigned char data );
void ReadI2C();
void NotAckI2C();

volatile unsigned char I2CIdleFlag = 0;
#endif


/* LCD functions */
void enable(void);
void initLCD(void);
void insertChar(unsigned char c);
void checkBusy(void);
void endLineCheck(void);
void resetCursor(void);

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
    ByteWriteI2C(0xa0, 0x01, 0, counter);
    //while(1); //Don't do anything stupid.
}

#pragma interruptlow lo_isr
void lo_isr(void){
    static unsigned char interval = 0;

    PIR1bits.TMR2IF = 0;

    if (interval == 19){
    counter++;
    if(counter >= 100) counter = 0;

    resetCursor();
    insertChar((counter/10) + 0x30);
    insertChar((counter - (counter/10)*10) + 0x30);

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
    PORTD = 0b10000010; // Function set (Set interface to be 4 bits long.) Interface is 8 bits in length.
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
void resetCursor (void){
    PORTD = 0b10000000;
    enable();
    PORTD = 0b10000010;
    enable();
    waitms(5);
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
    SSPSTAT = 0x80; //Disable SMBus
    SSPCON1 = 0b00101011; //Enable MSSP Master
    SSPADD = 0x28; //Should be 0x18
    SSPCON2 = 0x00; //Clear MSSP Conrol Bits


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

#ifdef MY_I2C
void IdleI2C(){
    // Wait for Idle Status
     while ((SSPCON2 & 0x1F) | (SSPSTATbits.R_W));
}

void StartI2C(){
    //Send Start Condition
    SSPCON2bits.SEN = 1;
    while ( SSPCON2bits.SEN ); // wait until start condition is over
}

void RestartI2C(){
    SSPCON2bits.RSEN = 1;
    while(SSPCON2bits.RSEN == 1);
}

void StopI2C(){
    SSPCON2bits.PEN = 1;
    while(SSPCON2bits.PEN == 1);
}

void WriteI2C( unsigned char data ){
    SSPCON2bits.ACKSTAT = 1;
    SSPBUF = data;
    while(SSPCON2bits.ACKSTAT == 1);
}
void AckI2C(){
    SSPCON2bits.ACKDT = 0; //NotAck
    SSPCON2bits.ACKEN = 1; //Send NotAck
    while(SSPCON2bits.ACKEN);
}
void NotAckI2C(){
    SSPCON2bits.ACKDT = 1; //NotAck
    SSPCON2bits.ACKEN = 1; //Send NotAck
    while(SSPCON2bits.ACKEN);
}

int getsI2C(unsigned char* data, unsigned char length){
    int i;
    for(i = 0; i < length; i++){
        SSPCON2bits.RCEN = 1; //Do Recive
        while(SSPCON2bits.RCEN == 1);
        data[i] = SSPBUF;
        if(i < length-1){
            AckI2C();
        }
    }
}

unsigned char AckPolling(unsigned char data ){
    StartI2C();
    SSPCON2bits.ACKSTAT = 1;
    SSPBUF = data;
    IdleI2C();
    return !SSPCON2bits.ACKSTAT;
}
#endif

void ByteWriteI2C( unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char data )
{
    IdleI2C(); // ensure module is idle
    StartI2C(); // initiate START condition
    WriteI2C( ControlByte ); // write 1 byte - R/W bit should be 0
    IdleI2C(); // ensure module is idle
    WriteI2C( HighAdd ); // write address byte to EEPROM
    IdleI2C(); // ensure module is idle
    WriteI2C( LowAdd ); // write address byte to EEPROM
    IdleI2C(); // ensure module is idle
    WriteI2C ( data ); // Write data byte to EEPROM
    IdleI2C(); // ensure module is idle
    StopI2C(); // send STOP condition
    while (!AckPolling(ControlByte)); //Wait for write cycle to complete
}

void ByteReadI2C( unsigned char ControlByte, unsigned char HighAdd, unsigned char LowAdd, unsigned char* data, unsigned char length )
{
    IdleI2C(); // ensure module is idle
    StartI2C(); // initiate START condition
    WriteI2C( ControlByte ); // write 1 byte
    IdleI2C(); // ensure module is idle
    WriteI2C( HighAdd ); // WRITE word address to EEPROM
    IdleI2C(); // ensure module is idle
    WriteI2C( LowAdd ); // WRITE word address to EEPROM
    IdleI2C(); // ensure module is idle
    RestartI2C(); // generate I2C bus restart condition
    WriteI2C( ControlByte | 0x01 ); // WRITE 1 byte - R/W bit should be 1 for read
    IdleI2C(); // ensure module is idle
    getsI2C( data, length ); // read in multiple bytes
    NotAckI2C(); // send not ACK condition
    StopI2C(); // send STOP condition
}



void main(void){
    unsigned char c;
    unsigned char i;
    setup();

    ByteReadI2C(0xa0, 0x01, 0, counts+1, 3);
    counts[0] = counts[1];
    resetCursor();
    for(i = 0; i<4; i++){
        if(counts[i] > 99){
            counts[i] = 0;
        }
        insertChar((counts[i]/10) + 0x30);
        insertChar((counts[i] - (counts[i]/10)*10) + 0x30);
        insertChar(0x20);
        insertChar(0x20);
    }

    counter = counts[0];

    for(i = 0; i<4; i++){
        ByteWriteI2C(0xa0, 0x01, 0+i, counts[i]);
    }

    while (1){
        while(PORTAbits.RA4 ==1);
        waitms(20);
        //T2CONbits.TMR2ON = 0;
        //PORTBbits.RB3 = PORTBbits.RB3^1;
        while (PORTAbits.RA4 == 0);
        waitms(10);

    }
}