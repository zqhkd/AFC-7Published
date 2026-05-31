#include "LowPassFilter.h"
#include "fc_math.h"

void LPF_set_cutoff_frequency(LowPassFilter *LPF, float time_step, float cutoff_freq)
{
	float rc;
		
	if (cutoff_freq <= 0.0f) {
		LPF->_alpha = 1.0f;
		return;
	}

	rc = 1/(2*PI*cutoff_freq);
	LPF->_alpha = time_step / (time_step + rc);
}

float LPF_apply(LowPassFilter *LPF, float sample)
{
    if( !LPF->_base_value_set ) {
        LPF->_base_value = sample;
        LPF->_base_value_set = 1;
    }

    LPF->_base_value = LPF->_base_value + LPF->_alpha * ((float)sample - LPF->_base_value);
    return LPF->_base_value;
}

void LPF2P_set_cutoff_frequency(LPF_2P *LPF, float sample_freq, float cutoff_freq)
{
    LPF->_cutoff_freq = cutoff_freq;
    float fr = sample_freq/LPF->_cutoff_freq;
    float ohm = tanf(PI/fr);
    float c = 1.0f+2.0f*cosf(PI/4.0f)*ohm + ohm*ohm;
    LPF->_b0 = ohm*ohm/c;
    LPF->_b1 = 2.0f*LPF->_b0;
    LPF->_b2 = LPF->_b0;
    LPF->_a1 = 2.0f*(ohm*ohm-1.0f)/c;
    LPF->_a2 = (1.0f-2.0f*cosf(PI/4.0f)*ohm+ohm*ohm)/c;
}

float LPF2P_apply(LPF_2P *LPF, float sample)
{
    float delay_element_0 = sample - LPF->_delay_element_1 * LPF->_a1 - LPF->_delay_element_2 * LPF->_a2;
    if (isnan(delay_element_0) || isinf(delay_element_0)) {
        delay_element_0 = sample;
    }
    float output = delay_element_0 * LPF->_b0 + LPF->_delay_element_1 * LPF->_b1 + LPF->_delay_element_2 * LPF->_b2;
    
    LPF->_delay_element_2 = LPF->_delay_element_1;
    LPF->_delay_element_1 = delay_element_0;
		
    return output;
}

