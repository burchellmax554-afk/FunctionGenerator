#include "MCUType.h"

#ifndef STATE_H
#define STATE_H

// Rotary Definitions
#define SLICE_PER 50
#define EDGE_DIV 2
#define CNT_MIN 0
#define CNT_MAX 20

typedef enum {
    sine,
    pulse
} WAVE_FORM;

typedef struct {
    WAVE_FORM wave_form;           // Waveform type (sine or pulse)
    INT32U sine_frequency;         // Frequency for sinewave (Hz)
    INT32S sine_amplitude;         // Amplitude for sinewave
    INT32U pulse_frequency;        // Frequency for pulse train (Hz)
    INT32S pulse_duty_cycle;       // Duty cycle for pulse train (%)
} SystemState;

extern SystemState current_state;  // Declare current_state as an external variable
extern  SystemState previous_state;

extern INT16S qeCnt;        // Declare as external variable
extern INT16S qeXCnt;       // Declare as external variable
typedef struct{
    INT16U baseline;
    INT16U offset;
    INT16U threshold;
    INT8U tsiFlag;
}TOUCH_LEVEL_T;

extern TOUCH_LEVEL_T tsiLevels;

#endif /* STATE_H */
