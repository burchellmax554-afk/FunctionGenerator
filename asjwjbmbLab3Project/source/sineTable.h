#ifndef SINETABLE_H_
#define SINETABLE_H_

#include "MCUType.h"

// Sine wave lookup table (for 91 angles from 0 to 90 degrees)
extern INT16U sin_table[91];
extern INT16U pulse_table[91];
extern INT16U function_table[91];
// Declaration of the function to convert sine to pulse wave
void convert_sine_to_pulse_wave(void);
void table_select(void);

#endif /* SINETABLE_H_ */
