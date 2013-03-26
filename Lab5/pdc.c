#include "semphr.h"
#include "FreeRTOS.h"			// FreeRTOS.org includes.
#include "task.h"

/*-----------------------------------------------------------*/
// PDC variables
/*-----------------------------------------------------------*/
#define SURFACE_MODE 0
#define DIVE_MODE 1
static volatile unsigned char PDC_STATE;				// always has one of the MODE defined values
static volatile unsigned long seconds = 0;				// time
static volatile int DESCENT_RATE_FPS = 0;				// descent rate in ft/second
static volatile unsigned char wentUnder = 0;			// simple flag to check if they went underwater yet

// These define dive records, and the diveData array is 
// representing a fixed size storage media
// length of diveData is how many dives
typedef unsigned char byte;
typedef unsigned int word;
typedef struct {
	byte 	TYPE; 			// 0xAA=sample record, 0x55=event record
	word 	DEPTH_IN_FT;
	word 	TIME_IN_SEC;
	word 	DATA1;			// depends on record type
	word 	DATA2;			// ..
} RECORD;
typedef struct {
	word 	HEADER;				// in this implementation this is simply time in seconds, binary
	byte	numRecords;
	RECORD	diveRecords[100];	// array of RECORDS
	word 	TRAILER;			// in this implementation this is simply time in seconds, binary
} DIVE_FILE;
static volatile int diveNumber = -1; 		// acts as a pointer to the current dive records in diveData
static volatile int completedDives = 0;		
static volatile DIVE_FILE diveData[2];		// array of dive records, represents EEPROM
static volatile int currentDepth = 0;		// depth in FT

xSemaphoreHandle xMutex;

/************************************
* Author: E.Pataky
* DESC: end the dive
*************************************/
void printDiveFile()
{
}

/************************************
* Author: E.Pataky
* DESC: create new dive
*************************************/
void newDive()
{
}

/************************************
* Author: E.Pataky
* DESC: take sample
*************************************/
void takeSample()
{
}


/************************************
* Author: E.Pataky
* DESC: event record
*************************************/
void eventRecord()
{
}

/************************************
* Author: E.Pataky
* DESC: end the dive
*************************************/
void endDive()
{
}

void updateLEDs()
{
}

/************************************
* Author: E.Pataky
* DESC: initialize all dive variables
*************************************/
void initializePDC()
{
}
