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
static volatile unsigned char BUTTON_EVENT = 0;

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
static volatile int completedDives = 0;		
static volatile DIVE_FILE diveData[2];		// array of dive records, represents EEPROM
static volatile int currentDepth = 0;		// depth in FT

xSemaphoreHandle xMutexEEPROM;

/************************************
* Author: Patrick Barrett
* DESC: Print Out the Dive File
*************************************/
void printDiveFile(){
	//diveData[completedDives]
}

/************************************
* Author: Patrick Barrett
* DESC: Initialize New Dive
*************************************/
void newDive(){
	wentUnder = seconds;
}

/************************************
* Author: Patrick Barrett
* DESC: Finish Dive
*************************************/
void endDive(){

	printDiveFile();
	completedDives++;
}

/************************************
* Author: Patrick Barrett
* DESC: take sample
*************************************/
unsigned char takeSample(){
	unsigned char buttonEvent = 0;

	taskENTER_CRITICAL();
	buttonEvent = BUTTON_EVENT;
	BUTTON_EVENT = 0;
	taskEXIT_CRITICAL();

	if(buttonEvent){
		xTaskCreate( eventTask, (signed char*) "EXE EVENT", configMINIMAL_STACK_SIZE, NULL, 10, NULL);
	}

	return 0;
}


/************************************
* Author: Patrick Barrett
* DESC: event record
*************************************/
void eventRecord(){
	xSemaphoreTake( xMutexEEPROM, portMAX_DELAY );
	// Write Event to EEPROM

	xSemaphoreGive( xMutexEEPROM );
}

void updateLEDs(){
}

/************************************
* Author: Patrick Barrett
* DESC: initialize all dive variables
*************************************/
void initializePDC(){
	xMutexEEPROM = xSemaphoreCreateMutex();
}


/************************************
* Author: Patrick Barrett
* DESC: keeps track of the clock and depth.
*************************************/
void clockTask( void *pvParameters ){
	// inf loop
	for(EVER){
		vTaskDelay( 1000 / portTICK_RATE_MS );
		seconds++;

		if(PDC_STATE == SURFACE_MODE){
			if(DESCENT_RATE_FPS > 0 && currentDepth == 0){
				newDive();
			}
		}else{
			currentDepth += DESCENT_RATE_FPS;
	
			if(DESCENT_RATE_FPS < 0 && currentDepth <= 0){
				endDive();
			}
		}
	}
	vTaskDelete(NULL); // precaution in case
}


/************************************
* Author: Patrick Barrett
* DESC: Writes to the 'EEPROM' every 2 seconds.
*************************************/
void samplingTask( void *pvParameters ){
	// inf loop
	for(EVER){
		vTaskDelay( 2000 / portTICK_RATE_MS );
	
	if(PDC_STATE == DIVE_MODE){
		xSemaphoreTake( xMutexEEPROM, portMAX_DELAY );
		//Write Sample to EEPROM Here

		xSemaphoreGive( xMutexEEPROM );
	}

	}
	vTaskDelete(NULL); // precaution in case
}

/************************************
* Author: Patrick Barrett
* DESC: This task is enabled as a one-off every time a button is pressed.
*************************************/
void eventTask( void *pvParameters ){
	unsigned char buttonEvent =  0;
	if(buttonEvent == 1){
	}else if(buttonEvent == 2){
	}

	eventRecord();
	vTaskDelete(NULL); // One Time Task, Don't Repeat
}