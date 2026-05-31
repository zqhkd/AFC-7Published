#ifndef __LOWPASSFILTER_H
#define __LOWPASSFILTER_H

#include "stm32h7xx_hal.h"

typedef struct {
	float _alpha;
	float _base_value;
	uint8_t _base_value_set;
} LowPassFilter;

void LPF_set_cutoff_frequency(LowPassFilter *LPF, float time_step, float cutoff_freq);
float LPF_apply(LowPassFilter *LPF, float sample);

typedef struct {
	float           _cutoff_freq; 
	float           _a1;
	float           _a2;
	float           _b0;
	float           _b1;
	float           _b2;
	float           _delay_element_1;        // buffered sample -1
	float           _delay_element_2;        // buffered sample -2
} LPF_2P;

void LPF2P_set_cutoff_frequency(LPF_2P *LPF, float sample_freq, float cutoff_freq);
float LPF2P_apply(LPF_2P *LPF, float sample);


#endif


