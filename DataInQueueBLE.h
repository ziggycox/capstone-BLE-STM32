#ifndef DataInQueueBLE_H
#define DataInQueueBLE_H
/**
  * Defines queue system for DataInBLE.
  * Default value of being able to hold a max of 60 items.
  *
  * Author: Terry Cox (coxte@pm.me)
  * Date:   6/9/19
*/

#include "DataStructsBLE.h"
#include <stdbool.h>

#define capacity 60

// insert DataIn into queue
void addDataInQueue(DataInBLE data);

// Will return first item and remove it from queue
DataInBLE removeDataInQueue(void);

// Returns true on full
bool isFullDataInQueue(void);

// Returns true on empty
bool isEmptyDataInQueue(void);

#endif
