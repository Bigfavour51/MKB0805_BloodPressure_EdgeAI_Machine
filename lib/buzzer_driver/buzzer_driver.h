#ifndef __BUZZER_DRIVER_H__
#define __BUZZER_DRIVER_H__


#include <Arduino.h>
#include "datatypes.h"

/******************************************************
 * Initialization
 ******************************************************/
bool Buzzer_begin(void);

/******************************************************
 * Update (call every loop)
 ******************************************************/
void Buzzer_update(void);

/******************************************************
 * Controls
 ******************************************************/
void Buzzer_play(BuzzerPattern pattern);

void Buzzer_stop(void);

bool Buzzer_isPlaying(void);





#endif // __BUZZER_DRIVER_H__