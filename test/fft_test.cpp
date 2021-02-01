#include <Arduino.h>
#include <fft.h>
#include <unity.h>

#define twoPi 6.28318531

fft_config_t *real_fft_plan; 

/*
These values can be changed in order to evaluate the functions
*/
const double samplingFrequency = 32768; // 2^15 Hz
const uint16_t num_samples = 1024; //This value MUST ALWAYS be a power of 2
// set 4 frequencies to build a signal with 4 sinusoids 
const uint8_t num_cycles = 4;
//const double signalFrequency[num_cycles] = {107.666016,409.130859,990.527344,8010.351562};
//const double signalFrequency[num_cycles] = {441.0, 882.0, 1323.0,7938.0};
const double signalFrequency[num_cycles] = {256.0,512.0,2048.0, 8192.0}; // Hz
double cycles[num_cycles];
const uint8_t amplitude = 1;


void build_fft_input(){
  
  for(int i=0; i<num_cycles; i++){
    cycles[i] = (((num_samples-1) * signalFrequency[i]) / samplingFrequency); //Number of signal cycles that the sampling will read
  }  
  /* Build raw data */
  for (uint16_t i = 0; i < num_samples; i++)
  {
    real_fft_plan->input[i] = 0;
    for(int cycle_ndx=0; cycle_ndx<num_cycles; cycle_ndx++){
      real_fft_plan->input[i] += sin((i * (twoPi * cycles[cycle_ndx] )) / num_samples);
    }
    real_fft_plan->input[i] = amplitude * real_fft_plan->input[i];  

    //Serial.printf("FFT input real_fft_plan->input[%i]=%f\r\n",
    //                i,real_fft_plan->input[i]);
  }
  
}

void myFloatWindowingForwardHamming(float *vData, uint16_t samples)
{// Weighing factors are computed once before multiple use of FFT
// The weighing function is symetric; half the weighs are recorded
	//#warning("This method is deprecated and may be removed on future revisions.")
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


void test_fft(){
    // Create the FFT config structure
     real_fft_plan = fft_init((int)num_samples, FFT_REAL,(fft_direction_t) FFT_FORWARD, NULL, NULL);
     // build the input signal for the FFT
     build_fft_input();

    //myFloatWindowingForwardHamming(real_fft_plan->input, num_samples);	// Weigh data 
 
    // execute the fft
    fft_execute(real_fft_plan);
    // evaluate the magnitudes
    float mags[num_samples];
    for (int k = 1 ; k < real_fft_plan->size / 2 ; k++)
        mags[k] = sqrt(real_fft_plan->output[2*k] * real_fft_plan->output[2*k] 
                              + real_fft_plan->output[2*k+1] * real_fft_plan->output[2*k+1]) ;
    mags[0] = real_fft_plan->output[0] + real_fft_plan->output[1];
  

    //for(int i = 0; i< num_samples/2;i++){
    //    Serial.printf("freq=%f mags[%i]=%f\r\n",
    //                samplingFrequency*i/num_samples, i,mags[i]);
    //}
    Serial.println("Assert freq=256Hz at mags[8]");
    TEST_ASSERT_EQUAL(511.345306, mags[8]);
    Serial.println("Assert freq=512Hz at mags[16]");
    TEST_ASSERT_EQUAL(512.325012, mags[16]);
    Serial.println("Assert freq=2048Hz at mags[64]");
    TEST_ASSERT_EQUAL(508.859222, mags[64]);
}


