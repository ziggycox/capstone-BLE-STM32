#ifndef HitDataQueueBLE_H
#define HitDataQueueBLE_H
/**
  * Defines queue system for HitDataBLE.
  * Default value of being able to hold a max of 60 items.
  *
  * Author: Terry Cox (coxte@pm.me)
  * Date:   6/9/19
*/

#include "DataStructsBLE.h"
#include <stdbool.h>

#define capacity 60

// insert HitData into queue
void addHitDataQueue(HitDataBLE data);

// Will return first item and REMOVES it from queue
HitDataBLE removeHitDataQueue(void);

// Will return first item of queue WITHOUT removing it
HitDataBLE peekHitDataQueue(void);

// Returns true on full
bool isFullHitDataQueue(void);

// Returns true on empty
bool isEmptyHitDataQueue(void);

#endif
