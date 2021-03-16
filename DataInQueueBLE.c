/**
  * Defines queue system for DataInBLE.
  * Default value of being able to hold a max of 60 items.
  *
  * Author: Terry Cox (coxte@pm.me)
  * Date:   6/9/19
*/

#include "DataInQueueBLE.h"

static DataInBLE array[capacity];
static unsigned short head = 0, size = 0, tail = 0;

// insert DataIn into queue
void addDataInQueue(DataInBLE data) {
	if(size == capacity)
		return;
	array[tail] = data;
	tail = (tail+1)%capacity;
	size++;
}

// Will return first item and remove it from queue
DataInBLE removeDataInQueue(void) {
	DataInBLE temp;
	temp = array[head];
	head = (head+1)%capacity; 
	size--;
	return temp;
}

// Returns true on full
bool isFullDataInQueue(void) {
	return (size == capacity);
}

// Returns true on empty
bool isEmptyDataInQueue(void) {
	return (size == 0);
}
