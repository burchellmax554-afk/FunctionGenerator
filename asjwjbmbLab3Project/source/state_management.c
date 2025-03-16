#include "state_management.h"
#include "state.h"

/* Reset the system state to default values */
void ResetSystemState(void) {
    current_state = (SystemState) {sine, 1000, 10, 1000, 50};
    // Reset rotary encoder variables to prevent them from overriding the initial state
    SINE.qeCnt = 10;             // Default value for sine amplitude (or pulse duty cycle)
    PULSE.qeCnt = 50;
    SINE.qeXCnt = (10 * EDGE_DIV);  // Default position, scaled by EDGE_DIV (match the default values)
    PULSE.qeXCnt = (10 * EDGE_DIV);
}

/* Calculate the number of digits of a number */
INT8U GetNumberOfDigits(INT32U num) {
    INT8U num_digits = 0;
    if (num == 0) {
        return 1;
    }
    while (num > 0) {
        num /= 10;
        num_digits++;
    }
    return num_digits;
}
