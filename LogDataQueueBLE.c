/**
  * Defines queue system for LogDataBLE.
  * Default value of being able to hold a max of 60 items.
  *
  * Author: Terry Cox (coxte@pm.me)
  * Date:   6/9/19
*/

#include "LogDataQueueBLE.h"

static LogDataBLE array[capacity];
static unsigned short head = 0, size = 0, tail = 0;

// insert LogData into queue
void addLogDataQueue(LogDataBLE data) {
	if(size == capacity)
		return;
	array[tail] = data;
	tail = (tail+1)%capacity;
	size++;
}

// Will return first item and REMOVES it from queue
LogDataBLE removeLogDataQueue(void) {
	LogDataBLE temp;
	temp = array[head];
	head = (head+1)%capacity; 
	size--;
	return temp;
}

// Will return first item of queue WITHOUT removing it
LogDataBLE peekLogDataQueue(void) {
	LogDataBLE temp;
	temp = array[head];
	return temp;
}

// Returns true on full
bool isFullLogDataQueue(void) {
	return (size == capacity);
}

// Returns true on empty
bool isEmptyLogDataQueue(void) {
	return (size == 0);
}
