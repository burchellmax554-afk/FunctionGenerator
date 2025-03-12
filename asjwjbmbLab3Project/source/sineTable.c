#include "sineTable.h"
#include "MCUType.h"
#include <Timer.h>
#include "os.h"
#include "FRDM_MCXN947ClkCfg.h"
#include "FRDM_MCXN947_GPIO.h"
#include "CsOS_SW.h"
#include <string.h>
#include "state.h"


// Sine wave lookup table (for 91 angles from 0 to 90 degrees)

// Pulse wave lookup table
INT16U pulse_table[91];

// Function to convert sine wave to pulse wave
void convert_sine_to_pulse_wave() {
    for (int i = 0; i < 91; i++) {
        if (sin_table[i] >= 0x363A) {
            pulse_table[i] = 0x3FFF;  // High value for positive sine
        } else {
            pulse_table[i] = 0x2000;  // Low value for negative sine
        }
    }
}

// Function to select the waveform (sine or pulse)
void table_select() {
    switch (current_state.wave_form) {
        case sine:
        	memcpy(sin_table, sin_table, sizeof(sin_table));
            break;
        case pulse:
            convert_sine_to_pulse_wave();  // Convert sine to pulse wave
            memcpy(sin_table, pulse_table, sizeof(pulse_table));
            break;
        default:
        	memcpy(sin_table, sin_table, sizeof(sin_table));
    }
}
//Bar
