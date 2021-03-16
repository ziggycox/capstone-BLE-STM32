/**
  * Defines queue system for HitDataBLE.
  * Default value of being able to hold a max of 60 items.
  *
  * Author: Terry Cox (coxte@pm.me)
  * Date:   6/9/19
*/

#include "HitDataQueueBLE.h"

static HitDataBLE array[capacity];
static unsigned short head = 0, size = 0, tail = 0;

// insert HitData into queue
void addHitDataQueue(HitDataBLE data) {
	if(size == capacity)
		return;
	array[tail] = data;
	tail = (tail+1)%capacity;
	size++;
}

// Will return first item and REMOVES it from queue
HitDataBLE removeHitDataQueue(void) {
	HitDataBLE temp;
	temp = array[head];
	head = (head+1)%capacity; 
	size--;
	return temp;
}

// Will return first item of queue WITHOUT removing it
HitDataBLE peekHitDataQueue(void) {
	HitDataBLE temp;
	temp = array[head];
	return temp;
}

// Returns true on full
bool isFullHitDataQueue(void) {
	return (size == capacity);
}

// Returns true on empty
bool isEmptyHitDataQueue(void) {
	return (size == 0);
}

