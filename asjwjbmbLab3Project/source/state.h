#include "MCUType.h"

#ifndef STATE_H
#define STATE_H

typedef enum {
    sine,
    pulse
} WAVE_FORM;

typedef struct {
    WAVE_FORM wave_form;           // Waveform type (sine or pulse)
    INT32U sine_frequency;         // Frequency for sinewave (Hz)
    INT32U sine_amplitude;         // Amplitude for sinewave
    INT32U pulse_frequency;        // Frequency for pulse train (Hz)
    INT32U pulse_duty_cycle;       // Duty cycle for pulse train (%)
} SystemState;

extern SystemState current_state;  // Declare current_state as an external variable
extern  SystemState previous_state;

#endif /* STATE_H */
