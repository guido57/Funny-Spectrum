#ifndef __MYFFT_H
#define __MYFFT_H

#include <Arduino.h>
#include "fft.h"

extern const uint8_t num_bands;
extern double bands[];

const uint16_t samples = 1024; //This value MUST ALWAYS be a power of 2

extern fft_config_t *real_fft_plan_left; 
extern fft_config_t *real_fft_plan_right; 

extern float benchmark_mags_left[];
extern float benchmark_mags_right[];


// -----------------------------------------------------------------------------------
// Calculate a FFT on N float samples and store the N/2 magnitudes on 
// magnitudes (from 0 to real_fft_plan->size/2 - 1
// -----------------------------------------------------------------------------------
void myFFT(fft_config_t * real_fft_plan, float * magnitudes);

// Compute 12 bands for 12 LEDs reading a spectrum of num_frequencies frequencies
// Arguuments:
void Compute12bands(float *frequencies, uint16_t num_frequencies, double bands[12]);

#endif