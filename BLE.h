#ifndef BLE_H
#define BLE_H
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

#include "DataStructsBLE.h"
#include <stdbool.h>

// encode hitData into byte array to send over BLE
void encodeHitData(HitDataBLE data, unsigned char temp[], unsigned char type);

// encode hitData into byte array to send over BLE
void encodeLogData(LogDataBLE data, unsigned char temp[], unsigned char type);

// Test function to make sure that encode / decode work
HitDataBLE decodeHitDataTest(unsigned char data[]);

// decode data from phone buffer and put into dataIn
DataInBLE decode(unsigned char data[]);

// Function that is called on receive to first determine if message is ack
bool decodeAck(unsigned char data[]);

// Function to send ack to phone
void sendAck(unsigned char temp[]);

// Function to send ack to phone for test
void sendAckTest(unsigned char temp[]);

// Returns ability to transmit new Data
bool canTransmitHitData(void);

// Returns ability to transmit new Data
bool canTransmitLogData(void);

// Reset Hit Data seq for testing
void resetHitDataSeq(void);

#endif
