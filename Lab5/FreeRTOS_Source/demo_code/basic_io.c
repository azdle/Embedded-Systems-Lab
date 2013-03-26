/*
    FreeRTOS V7.1.0 - Copyright (C) 2011 Real Time Engineers Ltd.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.
    ***NOTE*** The exception to the GPL is included to allow you to distribute
    a combined work that includes FreeRTOS without being obliged to provide the
    source code for proprietary components outside of the FreeRTOS kernel.
    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details. You should have received a copy of the GNU General Public 
    License and the FreeRTOS license exception along with FreeRTOS; if not it 
    can be viewed here: http://www.freertos.org/a00114.html and also obtained 
    by writing to Richard Barry, contact details for whom are available on the
    FreeRTOS WEB site.

    1 tab == 4 spaces!

    http://www.FreeRTOS.org - Documentation, latest information, license and
    contact details.

    http://www.SafeRTOS.com - A version that is certified for use in safety
    critical systems.

    http://www.OpenRTOS.com - Commercial support, development, porting,
    licensing and training services.
*/
#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#define xBUFSIZ ( 100 )

/* The buffer to be used by printf(). */
static char cPrintfBuffer[ xBUFSIZ ];

void vSetIOBuffering( void )
{
	/* Set the terminal output to line buffered mode. */
	setvbuf( stdout, cPrintfBuffer, _IOLBF, xBUFSIZ );
}
/*-----------------------------------------------------------*/

void myPrintString( const char *pcStringFmt, const char *pcString, int value)
{
	/* Print the string, suspending the scheduler as method of mutual
	exclusion. */
	vTaskSuspendAll();
	{
		if(value != 0xFFFF)
		{
			printf( pcStringFmt, pcString, value );
		}
		else
		{
			printf( pcStringFmt, pcString );
		}
		fflush( stdout );
	}
	xTaskResumeAll();
}

void vPrintString( const char *pcString )
{
	/* Print the string, suspending the scheduler as method of mutual
	exclusion. */
	vTaskSuspendAll();
	{
		printf( "%s\n", pcString );
		fflush( stdout );
	}
	xTaskResumeAll();
}
/*-----------------------------------------------------------*/

void vPrintStringAndNumber( const char *pcString, unsigned long ulValue )
{
	/* Print the string, suspending the scheduler as method of mutual
	exclusion. */
	vTaskSuspendAll();
	{
		printf( "%s %lu\n", pcString, ulValue );
		fflush( stdout );
	}
	xTaskResumeAll();
}
/*-----------------------------------------------------------*/

void vPrintStringAndSignedNumber( const char *pcString, int siValue )
{
	/* Print the string, suspending the scheduler as method of mutual
	exclusion. */
	vTaskSuspendAll();
	{
		printf( "%s %i\n", pcString, siValue );
		fflush( stdout );
	}
	xTaskResumeAll();
}
/*-----------------------------------------------------------*/


void vPrintMultipleLines( const signed char *pcString )
{
char *pc;
char *pcDelimiter = { "\r\n" };

	/* If pcString contains multiple lines, then tokenised the string into single
	limes, and print each out individually.  This ensures the line buffering on
	the terminal output works as expected. */
	pc = strtok( ( char * ) pcString, pcDelimiter );
	while( pc != NULL ) 
	{
		printf( "%s\n", pc );
		fflush( stdout );
		pc = strtok( NULL, pcDelimiter );
	}
}




