/**
  * STM32 BLE integration communticates with BLE module through UART to transfer data from
  * to transfer data from wall to phone and vice versa in-order to emulate button presses.
  * 
  * Contains all the logic for encode / decode, handling acks (everything with BLE)
  * Raytac BLE board using pins RX: PA9 -- TX: PA10 (Pins correspond to RX / TX labels [J10] on Raytac board)
  * VCC using 3.3V from STM
  * Data transfer rate of 57600 BAUD
  * UART PD Pin tied to GND
  * 
  * Author: Terry Cox (coxte@pm.me)
  * Date:   6/9/19
*/

#include "BLE.h"
#include "HitDataQueueBLE.h"
#include "LogDataQueueBLE.h"

static unsigned short seqHitData = 0;
static unsigned short seqLogData = 0;
static unsigned short lastHitDataAck = 0;
static unsigned short lastLogDataAck = 0;
static unsigned short dataInAck = 0;


/*
[DataType] is defined as such
HitData    --- 1
LogData    --- 2
DataIn     --- 3
TestIn	   --- 4	// Received from phone
AckHitData --- 11 
AckLogData --- 12
AckDataIn  --- 13
AckTestIn  --- 14    // Send to phone
*/

// encode hitData into byte array to send over BLE
// Returns temp[] which is char array to send over UART to BLE board
// type 1 == new data (update seq), type 2 == old data (dont update seq)
void encodeHitData(HitDataBLE data, unsigned char temp[], unsigned char type) {
	if(type == 1) {
		// update seq numbering
		seqHitData++;
		data.seq = seqHitData;
	} else {
		data.seq = seqHitData;
	}
	// knowing structure add bytes into array
	temp[0] = data.dataType;
	// Extract unsigned short (seq) into 2 bytes
	temp[1] = (data.seq & 0xFF);	
	temp[2] = ((data.seq >>8) & 0xFF);
	// Extract unsigned int (timeMillis) into 4 bytes
	temp[3] = (data.timeMillis & 0xFF);			// extract first byte
	temp[4] = ((data.timeMillis >>8) & 0xFF);  	// extract second byte
	temp[5] = ((data.timeMillis >>16) & 0xFF);  	// extract third byte
	temp[6] = ((data.timeMillis >>24) & 0xFF);  	// extract fourth byte
	// Extract unsigned int (timeStamp) into 4 bytes
	temp[7] = (data.timeStamp & 0xFF);			// extract first byte
	temp[8] = ((data.timeStamp >>8) & 0xFF);  	// extract second byte
	temp[9] = ((data.timeStamp >>16) & 0xFF);  	// extract third byte
	temp[10] = ((data.timeStamp >>24) & 0xFF);  	// extract fourth byte
	temp[11] = data.gameMode;
	temp[12] = data.targetPanel;
	temp[13] = data.hitPanel;
	// Extract unsigned int (gameTimer) into 4 bytes
	temp[14] = (data.gameTimer & 0xFF);	
	temp[15] = ((data.gameTimer >>8) & 0xFF);  
	temp[16] = ((data.gameTimer >>16) & 0xFF);  
	temp[17] = ((data.gameTimer >>24) & 0xFF); 
	// Extract unsigned short (score) into 2 bytes
	temp[18] = (data.score & 0xFF);	
	temp[19] = ((data.score >>8) & 0xFF);  
	// Extract unsigned int (handicapTime) into 4 bytes
	temp[20] = (data.handicapTime & 0xFF);	
	temp[21] = ((data.handicapTime >>8) & 0xFF);  
	temp[22] = ((data.handicapTime >>16) & 0xFF); 
	temp[23] = ((data.handicapTime >>24) & 0xFF);
}

// encode logData into byte array to send over BLE
// Returns temp[] which is char array to send over UART to BLE board
// type 1 == new data (update seq), type 2 == old data (dont update seq)
void encodeLogData(LogDataBLE data, unsigned char temp[], unsigned char type) {
	if(type == 1) {
		// update seq numbering
		seqLogData++;
		data.seq = seqLogData;
	} else {
		data.seq = seqLogData;
	}
	temp[0] = data.dataType;
	temp[1] = (data.seq & 0xFF);	
	temp[2] = ((data.seq >>8) & 0xFF);
	temp[3] = (data.timeStamp & 0xFF);			
	temp[4] = ((data.timeStamp >>8) & 0xFF);  
	temp[5] = ((data.timeStamp >>16) & 0xFF);  
	temp[6] = ((data.timeStamp >>24) & 0xFF);
	temp[7] = data.gameMode;
	temp[8] = data.targetPanel;
	temp[9] = data.hitPanel;
	temp[10] = (data.gameTimer & 0xFF);	
	temp[11] = ((data.gameTimer >>8) & 0xFF);  
	temp[12] = ((data.gameTimer >>16) & 0xFF);  
	temp[13] = ((data.gameTimer >>24) & 0xFF);
	temp[14] = (data.score & 0xFF);	
	temp[15] = ((data.score >>8) & 0xFF);
	temp[16] = (data.handicapTime & 0xFF);	
	temp[17] = ((data.handicapTime >>8) & 0xFF);  
	temp[18] = ((data.handicapTime >>16) & 0xFF); 
	temp[19] = ((data.handicapTime >>24) & 0xFF);
	temp[20] = (data.wallTemp & 0xFF);	
	temp[21] = ((data.wallTemp >>8) & 0xFF);
	temp[22] = (data.batteryCharge & 0xFF);
	temp[23] = ((data.batteryCharge >>8) & 0xFF);
	temp[24] = (data.ledBrightness & 0xFF);
	temp[25] = ((data.ledBrightness >>8) & 0xFF);
}

// decode data from phone buffer and put into dataIn
DataInBLE decode(unsigned char data[]) {
	DataInBLE temp;

	// turn byte array into 
	temp.dataType = data[0];
	temp.seq = (unsigned short)(data[1] | data[2] << 8);
	temp.timeStamp = (unsigned int)(data[3] | (data[4] << 8) | (data[5] << 16) | (data[6] << 24));
	temp.gameMode = data[7];
	temp.handicapTime = (unsigned int)(data[8] | (data[9] << 8) | (data[10] << 16) | (data[11] << 24));
	temp.playPause = data[12];
	temp.startEnd = data[13];
	temp.wallPowerOff = data[14];

	// Get seq # for ack
	dataInAck = temp.seq;

	return temp;
}

// Function that is called on receive to first determine if message is ack
// Returns true on ack
// Returns false on regular message
bool decodeAck(unsigned char data[]) {
	if(data[0] == 11) {
		// Message recieived is HitData ack
		// extract seq that is being ack'd
		unsigned short newAckSeq = (unsigned short)(data[1] | data[2] << 8);
		if(newAckSeq == seqHitData) {
			// Received ack is correct one for message just sent
			// Dequeue sent item that was ack'd
			if(!isEmptyHitDataQueue()){
				removeHitDataQueue();
			}
			lastHitDataAck = newAckSeq;
		} else {
			// Received ack is incorrect one
			
		}
		return true;
	} else if(data[0] == 12) {
		// Message recieived is LogData ack
		// extract seq that is being ack'd
		unsigned short newAckSeq = (unsigned short)(data[1] | data[2] << 8);
		if(newAckSeq == seqLogData) {
			// Received ack is correct one for message just sent
			if(!isEmptyLogDataQueue()) {
				removeLogDataQueue();
			}
			lastLogDataAck = newAckSeq;
		} else {
			// Received ack is incorrect one
			
		}
		return true;
	} else {
		// message received is not ack
		return false;
	}
}

// Function to send ack to phone
// inserts last received seq
void sendAck(unsigned char temp[]) {
	// send ack to phone (DataIn ack)
	temp[0] = 13;					// dataType
	temp[1] = (dataInAck & 0xFF);	
	temp[2] = ((dataInAck >> 8) & 0xFF);
}

// Function to send ack to phone
// inserts last received seq
void sendAckTest(unsigned char temp[]) {
	// send ack to phone (DataIn ack)
	temp[0] = 14;					// dataType
	temp[1] = (dataInAck & 0xFF);	
	temp[2] = ((dataInAck >> 8) & 0xFF);
}

// Returns ability to transmit new Data
// As in current message ack'd
bool canTransmitHitData(void) {
	return (seqHitData == lastHitDataAck);
}

// Returns ability to transmit new Data
// As in current message ack'd
bool canTransmitLogData(void) {
	return (seqLogData == lastLogDataAck);
}

void resetHitDataSeq(void) {
	seqHitData = 0;
	lastHitDataAck = 0;
}
