typedef enum{
	CONNTECTED,
	DISCONECTED,
	RESET,
	ERROR,
	STALL,
	STATECHANGE,
	TRANSCOM,
	IN,
	OUT,
	SETUP
}USB_LOG_EVENT;
 
typedef struct{
   USB_LOG_EVENT type;
   int time;
   int value;
} USB_RECORD;

extern USB_RECORD USB_LOG[1000];
extern unsigned char USB_LOG_COUNT;
extern unsigned int timeSinceConnection;