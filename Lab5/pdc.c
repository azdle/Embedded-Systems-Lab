#include "semphr.h"
#include "FreeRTOS.h"			// FreeRTOS.org includes.
#include "task.h"

#define EVER ;;					// Just for Fun



/*-----------------------------------------------------------*/
// The task functions.
/*-----------------------------------------------------------*/
void clockTask( void *pvParameters );
void samplingTask( void *pvParameters );
void eventTask( void *pvParameters );

/*-----------------------------------------------------------*/
// PDC variables
/*-----------------------------------------------------------*/
#define SURFACE_MODE 0
#define DIVE_MODE 1
static volatile unsigned char PDC_STATE;				// always has one of the MODE defined values
static volatile unsigned long seconds = 0;				// time
static volatile int DESCENT_RATE_FPS = 0;				// descent rate in ft/second
static volatile unsigned char wentUnder = 0;			// simple flag to check if they went underwater yet
static volatile unsigned char BUTTON_STATE = 0;

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

xSemaphoreHandle xMutexEEPROM;

/************************************
* Author: E.Pataky
* DESC: end the dive
*************************************/
void printDiveFile(){
}

/************************************
* Author: E.Pataky
* DESC: create new dive
*************************************/
void newDive(){
}

/************************************
* Author: E.Pataky
* DESC: take sample
*************************************/
unsigned char takeSample(){
	static unsigned char knownButtonState = 0;
	unsigned char buttonPressedEvent = 0;
	// This will return a 1 for any bit that has gone from 0 to 1:
	buttonPressedEvent = (BUTTON_STATE ^ knownButtonState) & BUTTON_STATE;
	knownButtonState = BUTTON_STATE;
	return buttonPressedEvent;
}


/************************************
* Author: E.Pataky
* DESC: event record
*************************************/
void eventRecord(){
	xSemaphoreTake( xMutexEEPROM, portMAX_DELAY );
	
	if(PDC_STATE == SURFACE_MODE){
		
	}

	xSemaphoreGive( xMutexEEPROM );
}

/************************************
* Author: E.Pataky
* DESC: end the dive
*************************************/
void endDive(){
}

void updateLEDs(){
}

/************************************
* Author: E.Pataky
* DESC: initialize all dive variables
*************************************/
void initializePDC(){
	xMutexEEPROM = xSemaphoreCreateMutex();
}




void clockTask( void *pvParameters ){
	// inf loop
	for(EVER){
		vTaskDelay( 1000 / portTICK_RATE_MS );
		seconds++;
	}
	vTaskDelete(NULL); // precaution in case
}


void samplingTask( void *pvParameters ){
	static unsigned char buttonEvent = 0;
	// inf loop
	for(EVER){
		vTaskDelay( 250 / portTICK_RATE_MS );
		buttonEvent = takeSample();
		if(buttonEvent){
			xTaskCreate( eventTask, (signed char*) "EXE EVENT", configMINIMAL_STACK_SIZE, &buttonEvent, 10, NULL);
		}
	}
	vTaskDelete(NULL); // precaution in case
}


void eventTask( void *pvParameters ){
	eventRecord();

	vTaskDelete(NULL); // One Time Task, Don't Repeat
}