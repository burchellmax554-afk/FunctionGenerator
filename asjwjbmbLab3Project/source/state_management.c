#include "state_management.h"
#include "state.h"

/* Reset the system state to default values */
/****************************************************************************************
* Credit: Max Burchell + Jake Sheckler
****************************************************************************************/
void ResetSystemState(void) {
    SINE.qeCnt = 10;             // Default value for sine amplitude (or pulse duty cycle)
    PULSE.qeCnt = 50;
    SINE.qeXCnt = (10 * EDGE_DIV);  // Default position, scaled by EDGE_DIV (match the default values)
    PULSE.qeXCnt = (10 * EDGE_DIV); //Defaut pos for pulse
    current_state.sine_frequency = 1000; //Defauts for the state
    current_state.pulse_frequency = 1000;
    current_state.pulse_duty_cycle = 50;
    current_state.wave_form = sine;
    // Reset rotary encoder variables to prevent them from overriding the initial state
}

/* Calculate the number of digits of a number */
/****************************************************************************************
* Credit: Max Burchell
****************************************************************************************/
INT8U GetNumberOfDigits(INT32U num) {
    INT8U num_digits = 0;  // Start with 0 digits
    if (num == 0) {  // If the number is 0
        return 1;  // 0 has 1 digit
    }
    while (num > 0) {  // While the number is greater than 0
        num /= 10;  // Remove the last digit
        num_digits++;  // Count the digit
    }
    return num_digits;  // Return the number of digits
}
