#ifndef LogDataQueueBLE_H
#define LogDataQueueBLE_H
/**
  * Defines queue system for LogDataBLE.
  * Default value of being able to hold a max of 60 items.
  *
  * Author: Terry Cox (coxte@pm.me)
  * Date:   6/9/19
*/

#include "DataStructsBLE.h"
#include <stdbool.h>

#define capacity 60

// insert LogData into queue
void addLogDataQueue(LogDataBLE data);

// Will return first item and REMOVES it from queue
LogDataBLE removeLogDataQueue(void);

// Will return first item of queue WITHOUT removing it
LogDataBLE peekLogDataQueue(void);

// Returns true on full
bool isFullLogDataQueue(void);

// Returns true on empty
bool isEmptyLogDataQueue(void);

#endif
