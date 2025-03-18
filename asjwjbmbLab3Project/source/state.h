/****************************************************************************************
* Credit: Everyone
****************************************************************************************/
#include "MCUType.h"
#include "os.h"

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
    INT16U pulse_duty_cycle;       // Duty cycle for pulse train (%)
} SystemState;

extern SystemState current_state;  // Declare current_state as an external variable
extern  SystemState previous_state; //Declaring previous staet as an external variable
extern SystemState defaultedstate; //defaulted state variable (for help with the reset.

typedef struct{
    INT16S qeCnt; //qeCnt val
    INT16S qeXCnt; //qeXCnt val
} rotary;

extern rotary SINE; //For splitting up the rotary movements for sine and pulse
extern rotary PULSE;

typedef struct{
    INT16U baseline;
    INT16U offset;
    INT16U threshold;
    INT8U tsiFlag;
}TOUCH_LEVEL_T; //TSI struct

extern TOUCH_LEVEL_T tsiLevels; //extern tsi struct (instead of mutex)


#endif /* STATE_H */
