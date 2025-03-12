#include "state.h"

// Define and initialize the global variables
SystemState current_state = {sine, 0, 0, 0, 0};   // Default state: sine wave, 1000Hz, amplitude 10, pulse freq 1000Hz, duty cycle 50%
SystemState previous_state = {sine, 0, 0, 0, 0};  // Initialize to default values
