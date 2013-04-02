/*============================================================
	Author:		E.Pataky
	Adapted By:	Patrick Barrett
	DESC:		Dive computer RTOS
==============================================================*/

#include <p32xxxx.h>			// MPLAB includes.
#include <plib.h>				// Adds support for PIC32 Peripheral library functions and macros

#include "FreeRTOS.h"			// FreeRTOS.org includes.
#include "task.h"

#include "pdc.c"

/*-----------------------------------------------------------*/
// PIC32 configuration
/*-----------------------------------------------------------*/
#pragma config FPLLODIV = DIV_1, FPLLMUL = MUL_20, FPLLIDIV = DIV_2
#pragma config FWDTEN = OFF, FPBDIV = DIV_2, POSCMOD = XT, FNOSC = PRIPLL, CP = OFF
//#pragma config FSRSSEL = PRIORITY_7

/* Basic hardware and debug interface configuration. */
void vSetupEnvironment( void );

/************************************
* Author: E.Pataky
* DESC: main function, setup 1 task for now
*************************************/
int main( void )
{
	initializePDC();
	/* Configure both the hardware and the debug interface. */
	vSetupEnvironment();

	// Create Tasks
	xTaskCreate( clockTask,    (signed char*) "RTC",          configMINIMAL_STACK_SIZE,NULL, 100, NULL);
	xTaskCreate( samplingTask, (signed char*) "INPUT SAMPLE", configMINIMAL_STACK_SIZE,NULL, 20, NULL);

	/* Start the scheduler so our tasks start executing. */
	vTaskStartScheduler();	
	
	/* If all is well we will never reach here as the scheduler will now be
	running.  If we do reach here then it is likely that there was insufficient
	heap available for the idle task to be created. */
	for( ;; );
	return 0;
}

/************************************
* Author: Patrick Barrett
* DESC: setup I/O and interrupts
*************************************/
void setupPorts()
{
	unsigned char temp;
    PORTSetPinsDigitalOut(IOPORT_D, BIT_0);
    PORTSetPinsDigitalOut(IOPORT_D, BIT_1);
    PORTSetPinsDigitalOut(IOPORT_D, BIT_2);
    PORTSetPinsDigitalIn(IOPORT_D, BIT_6);
    PORTSetPinsDigitalIn(IOPORT_D, BIT_7);
    //PORTSetPinsDigitalIn(IOPORT_D, BIT_13);

	mPORTDSetBits(BIT_0);
	mPORTDSetBits(BIT_1);
	mPORTDSetBits(BIT_2);

    mCNOpen((CN_ON | CN_IDLE_CON), (CN15_ENABLE | CN16_ENABLE), (CN15_PULLUP_ENABLE | CN16_PULLUP_ENABLE));
    temp = mPORTDRead();
    ConfigIntCN((CHANGE_INT_ON | CHANGE_INT_PRI_2)); //Clear Flag
    INTEnableSystemMultiVectoredInt();
}

void vSetupEnvironment( void )
{
	/* Setup the main clock for maximum performance, and the peripheral clock
	to equal the main clock divided by 2. */
	SYSTEMConfigPerformance( configCPU_CLOCK_HZ );
	mOSCSetPBDIV( OSC_PB_DIV_2 );

	// setup ports ?	
	setupPorts();

	/* Enable global interrupt handling. */
	INTEnableSystemMultiVectoredInt();

	/* Initialise the debug utils library to enable strings printed from the
	demo source code to be displayed in the MPLAB IDE. */
	DBINIT();

	/* Set the console output to line buffered mode. */
	vSetIOBuffering();
}
/*-----------------------------------------------------------*/

void __ISR(_CHANGE_NOTICE_VECTOR, ipl2) ChangeNotice_Handler(void)
{
	unsigned short portState;

    // clear the mismatch condition
    portState = mPORTDReadBits(BIT_6 | BIT_7);

    // clear the interrupt flag
    mCNClearIntFlag();

    // .. things to do .. toggle the led
    BUTTON_EVENT = (portState >> 6);



	if(!(portState & 0b01000000)){
		BUTTON_EVENT |= 1;
		DESCENT_RATE_FPS++;
	}
	if(!(portState & 0b10000000)){
		BUTTON_EVENT |= 2;
		DESCENT_RATE_FPS--;
	}
}