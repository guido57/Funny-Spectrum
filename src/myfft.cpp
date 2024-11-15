#include <Arduino.h>
#include <myfft.h>

#define twoPi 6.28318531

const double samplingFrequency = 22050; // yes, we are decimating 44100 by 2
const uint8_t amplitude = 1;

const uint8_t num_bands = 12;
double bands[num_bands];

fft_config_t *real_fft_plan_left; 
fft_config_t *real_fft_plan_right; 

float benchmark_mags_left[samples/2];
float benchmark_mags_right[samples/2];


void myFloatWindowingForwardHamming(float *vData, uint16_t samples)
{// Weighing factors are computed once before multiple use of FFT
// The weighing function is symetric; half the weighs are recorded
	float samplesMinusOne = (float(samples) - 1.0);
  float samplesdiv2 = samples >> 1;
	for (uint16_t i = 0; i < samplesdiv2; i++) {
		//float indexMinusOne = float(i);
		//float ratio = (indexMinusOne / samplesMinusOne);
		// Compute and record weighting factor
	 	float weighingFactor = 0.54 - (0.46 * cosf(twoPi * float(i) / samplesMinusOne));
		
  	vData[i] *= weighingFactor;
		vData[samples - (i + 1)] *= weighingFactor;
		
  }
}
// -----------------------------------------------------------------------------------
// Calculate a FFT on N float samples and store the N/2 magnitudes on 
// magnitudes (from 0 to real_fft_plan->size/2 - 1
// -----------------------------------------------------------------------------------
void myFFT(fft_config_t * real_fft_plan, float * magnitudes){

  // wheigh the samples to improve S/N ratio
  myFloatWindowingForwardHamming(real_fft_plan->input, samples);	// Weigh data 
  // Execute benchmark FFT transformation
  fft_execute(real_fft_plan);
  // evaluate magnitudes
  for (int k = 1 ; k < real_fft_plan->size / 2 ; k++)
    magnitudes[k] = sqrt(real_fft_plan->output[2*k] * real_fft_plan->output[2*k] 
                              + real_fft_plan->output[2*k+1] * real_fft_plan->output[2*k+1]) ;
  magnitudes[0] = real_fft_plan->output[0] + real_fft_plan->output[1];
}

// Compute 12 bands for 12 LEDs reading a spectrum of num_magnitudes magnitudes
// Arguuments:
void Compute12bands(float *magnitudes, uint16_t num_magnitudes, double bands[12])
{
  // set the bands to 0
  for (int j = 0; j < num_bands; j++)
    bands[j] = 0;
  //for(int j=0; j<num_bands;j++){
  //  Serial.printf("should be zero: mag[%d]=%f\r\n",j,num_bands[j]);

  //}
  // get the maximum value in each range of frequencies
  for (int i = 1; i < num_magnitudes; i++) // start from 1 to get rid of DC at i=0
  {
    double freq = double(i) * samplingFrequency / double(samples);
    if (freq < 50)
    {
      if (magnitudes[i] > bands[0])
        bands[0] = magnitudes[i];
    }
    else if (freq < 100)
    {
      if (magnitudes[i] > bands[1])
        bands[1] = magnitudes[i];
    }
    else if (freq < 200)
    {
      if (magnitudes[i] > bands[2])
        bands[2] = magnitudes[i];
    }
    else if (freq < 300)
    {
      if (magnitudes[i] > bands[3])
        bands[3] = magnitudes[i];
    }
    else if (freq < 500)
    {
      if (magnitudes[i] > bands[4])
        bands[4] = magnitudes[i];
    }
    else if (freq < 700)
    {
      if (magnitudes[i] > bands[5])
        bands[5] = magnitudes[i];
    }
    else if (freq < 1000)
    {
      if (magnitudes[i] > bands[6])
        bands[6] = magnitudes[i];
    }
    else if (freq < 2000)
    {
      if (magnitudes[i] > bands[7])
        bands[7] = magnitudes[i];
    }
    else if (freq < 3000)
    {
      if (magnitudes[i] > bands[8])
        bands[8] = magnitudes[i];
    }
    else if (freq < 5000)
    {
      if (magnitudes[i] > bands[9])
        bands[9] = magnitudes[i];
    }
    else if (freq < 8000)
    {
      if (magnitudes[i] > bands[10])
        bands[10] = magnitudes[i];
    }
    else
    {
      if (magnitudes[i] > bands[11])
        bands[11] = magnitudes[i];
    }
  }
}
