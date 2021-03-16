#ifndef DataStructsBLE_H
#define DataStructsBLE_H
/**
  * Defines the structs for holding sets of data (used in queues)
  * HitDataBLE - for sending to phone (normal operations)
  * LogDataBLE - for sending to phone (logging operations)
  * DataInBLE - data sent from phone to wall
  * 
  * Author: Terry Cox (coxte@pm.me)
  * Date:   6/9/19
  */
  
// Struct for data that gets sent over BLE (24 bytes)
typedef struct hitDataBLE {
	unsigned char dataType;
	unsigned short seq;					// Seq inserted when encode is called
	unsigned int timeMillis;  			// milis from system clock
	unsigned int timeStamp;   			// HH:MM:SS (in form of seconds) from RTC
	unsigned char gameMode;
	unsigned char targetPanel;
	unsigned char hitPanel;
	unsigned int gameTimer;
	unsigned short score;
	unsigned int handicapTime;
} HitDataBLE;

// Struct for data that gets sent over BLE [logging] (26 bytes)
// telemetry
typedef struct logDataBLE {
	unsigned char dataType;
	unsigned short seq;
	unsigned int timeStamp;
	unsigned char gameMode;
	unsigned char targetPanel;
	unsigned char hitPanel;
	unsigned int gameTimer;
	unsigned short score;
	unsigned int handicapTime;
	unsigned short wallTemp;   				// wall internal temp
	unsigned short batteryCharge;
	unsigned short ledBrightness;			// brighness of led rings
} LogDataBLE;

// Struct for data that gets sent over BLE (15 bytes)
typedef struct dataInBLE {
	unsigned char dataType;
	unsigned short seq;
	unsigned int timeStamp;
	unsigned char gameMode;
	unsigned int handicapTime;
	unsigned char playPause;
	unsigned char startEnd;
	unsigned char wallPowerOff;
} DataInBLE;

#endif
