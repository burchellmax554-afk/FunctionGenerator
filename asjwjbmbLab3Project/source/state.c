/****************************************************************************************
* Credit: Everyone
****************************************************************************************/
#include "state.h"

// Define and initialize the global variables
SystemState current_state = {sine, 1000, 10, 1000, 50};   // Default state: sine wave, 1000Hz, amplitude 10, pulse freq 1000Hz, duty cycle 50%
SystemState previous_state = {sine, 0, 0, 0, 0};  // Initialize to default values
TOUCH_LEVEL_T tsiLevels = {0,0xFDA0U,0,0};
SystemState defaultedstate = {sine, 1000, 10, 1000, 50}; //Only default state
rotary SINE = {10,10};
rotary PULSE = {10,50};
