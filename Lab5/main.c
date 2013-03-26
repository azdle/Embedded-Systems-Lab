/*
	E.Pataky
	DESC: Dive computer RTOS example
*/

#include <p32xxxx.h>			// MPLAB includes.
#include <plib.h>				// Adds support for PIC32 Peripheral library functions and macros

#include "FreeRTOS.h"			// FreeRTOS.org includes.
#include "task.h"

/*-----------------------------------------------------------*/
// PIC32 configuration
/*-----------------------------------------------------------*/
#pragma config FPLLODIV = DIV_1, FPLLMUL = MUL_20, FPLLIDIV = DIV_2
#pragma config FWDTEN = OFF, FPBDIV = DIV_2, POSCMOD = XT, FNOSC = PRIPLL, CP = OFF
//#pragma config FSRSSEL = PRIORITY_7

/* Basic hardware and debug interface configuration. */
void vSetupEnvironment( void );

/*-----------------------------------------------------------*/
// The task functions.
/*-----------------------------------------------------------*/
void clockTask( void *pvParameters );

/************************************
* Author: E.Pataky
* DESC: main function, setup 1 task for now
*************************************/
int main( void )
{
	/* Configure both the hardware and the debug interface. */
	vSetupEnvironment();

	// create tasks

	/* Start the scheduler so our tasks start executing. */
	//vTaskStartScheduler();	
	
	/* If all is well we will never reach here as the scheduler will now be
	running.  If we do reach here then it is likely that there was insufficient
	heap available for the idle task to be created. */
	for( ;; );
	return 0;
}

// use supplied ports example to setup ports and 
// change notification feature here
void setupPorts()
{

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

