#include "state.h"

// Define and initialize the global variables
SystemState current_state = {sine, 1000, 10, 1000, 50};   // Default state: sine wave, 1000Hz, amplitude 10, pulse freq 1000Hz, duty cycle 50%
SystemState previous_state = {sine, 0, 0, 0, 0};  // Initialize to default values
TOUCH_LEVEL_T tsiLevels = {0,0xFFA0U,0,0};
//TOUCH_LEVEL_T tsiLevels = {0xF000U,0xF000U,0,0};
