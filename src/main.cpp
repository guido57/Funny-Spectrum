#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include "captive_portal.h"
#include "mytask.h"
#include "WS2812Ring.h"
#include "arduinoFFT.h"

// Digital I/O used for MAX98357
//#define I2S_DOUT      27  // DIN connection
//#define I2S_BCLK      14  // Bit clock
//#define I2S_LRC       12  // Left Right Clock

// Digital I/O used for PCM5102
#define I2S_DOUT      0  // DIN connection
#define I2S_BCLK      4  // Bit clock
#define I2S_LRC       2  // Left Right Clock



/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
const uint16_t samples = 1024; //This value MUST ALWAYS be a power of 2

double vRealLeft[samples];
double vImagLeft[samples];
double vRealRight[samples];
double vImagRight[samples];
double magnitudes[12];
const double samplingFrequency = 22050; // yes, we are decimating 44100 by 2
const uint8_t amplitude = 100;
const uint8_t num_magnitudes = 12;

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

void myFFTLeft(){

  //PrintVector(vReal, samples, SCL_TIME);
  FFT.Windowing(vRealLeft,samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	// Weigh data 
  //Serial.println("Weighed data:");
  //PrintVector(vReal, samples, SCL_TIME);
  
  //for(int i=0; i<1024;i++){  
  //  Serial.printf("After FFT.Windowing 1024 samples vReal[%d] = %f vImag[%d] = %f \r\n",i,vReal[i],i,vImag[i]);
  //}
  
  
  FFT.Compute(vRealLeft, vImagLeft, samples, FFT_FORWARD); // Compute FFT 
  //Serial.println("Computed Real values:");
  //PrintVector(vReal, samples, SCL_INDEX);
  //Serial.println("Computed Imaginary values:");
  //PrintVector(vImag, samples, SCL_INDEX);
  
  //for(int i=0; i<20;i++){  
  //  Serial.printf("After FFT-Compute 20 samples vReal[%d] = %f vImag[%d] = %f \r\n",i,vReal[i],i,vImag[i]);
  //}
  
  
  FFT.ComplexToMagnitude(vRealLeft, vImagLeft, samples); // Compute magnitudes 
  
  
  //for(int i=0; i<20;i++){  
  //  Serial.printf("After Complex to magnitude 20 samples vReal[%d] = %f vImag[%d] = %f \r\n",i,vReal[i],i,vImag[i]);
  //}

}


void myFFTRight(){

  FFT.Windowing(vRealRight,samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	// Weigh data 
  FFT.Compute(vRealRight, vImagRight, samples, FFT_FORWARD); // Compute FFT 
  FFT.ComplexToMagnitude(vRealRight, vImagRight, samples); // Compute magnitudes 
}

// Compute 12 magnitudes for 12 LEDs reading a spectrum of num_frequencies frequencies
// Arguuments:
void Compute12magnitudes(double *frequencies, uint16_t num_frequencies, double magnitudes[12])
{
  // set the magnitudes to 0
  int num_magnitudes = 12;
  for (int j = 0; j < num_magnitudes; j++)
    magnitudes[j] = 0;
  //for(int j=0; j<num_magnitudes;j++){
  //  Serial.printf("should be zero: mag[%d]=%f\r\n",j,magnitudes[j]);

  //}
  for (int i = 1; i < num_frequencies; i++) // start from 1 to get rid of DC at i=0
  {
    double freq = double(i) * samplingFrequency / double(samples);
    if (freq < 100)
    {
      if (frequencies[i] > magnitudes[0])
        magnitudes[0] = frequencies[i];
    }
    else if (freq < 250)
    {
      if (frequencies[i] > magnitudes[1])
        magnitudes[1] = frequencies[i];
    }
    else if (freq < 500)
    {
      if (frequencies[i] > magnitudes[2])
        magnitudes[2] = frequencies[i];
    }
    else if (freq < 1000)
    {
      if (frequencies[i] > magnitudes[3])
        magnitudes[3] = frequencies[i];
    }
    else if (freq < 2000)
    {
      if (frequencies[i] > magnitudes[4])
        magnitudes[4] = frequencies[i];
    }
    else if (freq < 3000)
    {
      if (frequencies[i] > magnitudes[5])
        magnitudes[5] = frequencies[i];
    }
    else if (freq < 4000)
    {
      if (frequencies[i] > magnitudes[6])
        magnitudes[6] = frequencies[i];
    }
    else if (freq < 5000)
    {
      if (frequencies[i] > magnitudes[7])
        magnitudes[7] = frequencies[i];
    }
    else if (freq < 6000)
    {
      if (frequencies[i] > magnitudes[8])
        magnitudes[8] = frequencies[i];
    }
    else if (freq < 7000)
    {
      if (frequencies[i] > magnitudes[9])
        magnitudes[9] = frequencies[i];
    }
    else if (freq < 8000)
    {
      if (frequencies[i] > magnitudes[10])
        magnitudes[10] = frequencies[i];
    }
    else
    {
      if (frequencies[i] > magnitudes[11])
        magnitudes[11] = frequencies[i];
    }
  }
}

// W22812 12 LEDs RING
//  GPIO33 for ESP32 - SERIAL DATA PIN
int WS2812LeftRing_pin = 33;
int WS2812RightRing_pin = 14;
Adafruit_NeoPixel * strips[2];

void showMagnitudes(Adafruit_NeoPixel * strip, double magnitudes[12]){

  //Serial.println("Computed magnitudes:");
  double max = 0;
  double avg = 0;
  
  // evaluate magnitudes  
  for(int j=0; j<num_magnitudes;j++){
      if(magnitudes[j] > max) max = magnitudes[j];
      avg += magnitudes[j];
      //Serial.printf("mag[%d]=%f\r\n",j,magnitudes[j]);
  }
  avg = avg / num_magnitudes;
  // set LEDs according to magnitudes
  for(int j=0; j<num_magnitudes;j++){
    magnitudes[j] = magnitudes[j] * 255 / max;      
    //magnitudes[j] = (magnitudes[j]*magnitudes[j]*magnitudes[j])/(255.0*255.0);
    magnitudes[j] = (magnitudes[j]*magnitudes[j])/(255.0);
  }
  strip->setPixelColor(0, strip->Color(magnitudes[0],0,0));         
  strip->setPixelColor(1, strip->Color(magnitudes[1]*5/6,magnitudes[1]*1/6,                0));         
  strip->setPixelColor(2, strip->Color(magnitudes[2]*4/6,magnitudes[2]*2/6,                0));         
  strip->setPixelColor(3, strip->Color(magnitudes[3]*3/6,magnitudes[3]*3/6,                0));         
  strip->setPixelColor(4, strip->Color(magnitudes[4]*2/6,magnitudes[4]*4/6,                0));         
  strip->setPixelColor(5, strip->Color(magnitudes[5]*1/6,magnitudes[5]*5/6,                0));         
  strip->setPixelColor(6, strip->Color(                0,magnitudes[6]    ,                0));         
  strip->setPixelColor(7, strip->Color(                0,magnitudes[7]*4/5,magnitudes[7]*1/5));         
  strip->setPixelColor(8, strip->Color(                0,magnitudes[8]*3/5,magnitudes[8]*2/5));         
  strip->setPixelColor(9, strip->Color(                0,magnitudes[9]*2/5,magnitudes[9]*3/5));         
  strip->setPixelColor(10, strip->Color(                0,magnitudes[10]*1/5,magnitudes[10]*4/5));         
  strip->setPixelColor(11, strip->Color(                0,                0,magnitudes[11]));         
  strip->show();

}

unsigned long count_samples = 0;
void playSampleCallback(int16_t sample[2]){
  uint16_t ndx = count_samples % 10000;
  if( ndx < 2048){
    // read the first 1024 samples, decimating by 2 (i.e. sampling at 22050 Hz)
    if(ndx%2 == 0){
      vRealLeft[ndx>>1] = sample[0]; // left channel only
      vImagLeft[ndx>>1] = 0;
      //Serial.printf("count_samples=%lu ndx=%u sample[0]=%d ndx>>2=%d vReal[ndx>>2]=%f\r\n",count_samples,ndx,sample[0],ndx>>2,vReal[ndx>>2]  );
    }else{
      vRealRight[ndx>>1] = sample[1]; // left channel only
      vImagRight[ndx>>1] = 0;
    }
  }
  if(ndx == 2048){
    // now vReal contains 1024 samples -> it's time to compute the FFT
    unsigned long start_millis = millis();
    //Serial.printf("Compute FFT on 1024 samples at %lu millis\r\n",start_millis);
    //for(int i=0; i<20;i++){  
    //  Serial.printf("first 20 samples vReal[%d] = %f \r\n",i,vReal[i]);
    //}
    myFFTLeft();
    myFFTRight();
    Compute12magnitudes(vRealLeft, samples/2,magnitudes);
    showMagnitudes(strips[0],magnitudes);
    Compute12magnitudes(vRealRight, samples/2,magnitudes);
    showMagnitudes(strips[1],magnitudes);

    Serial.printf("FFT on 1024 samples and light LEDs took %lu msecs\r\n",millis()-start_millis);
  }
  //Serial.printf("sample #%lu at millis=%lu \r\n",count_samples, millis());
  count_samples++;
}

Audio audio;

//push button
int gpio32 = 1;
int last_gpio32 = 1;

// =====================================================
// tosk run by Taskscheduler to handle Audio
class TaskAudio : public Task {
  public:
    void (*_myCallback)();
    ~TaskAudio() {};
    TaskAudio(unsigned long interval, Scheduler* aS ) :  Task(interval, TASK_FOREVER, aS, true) {
      
    };
    bool Callback(){
      //_myCallback();
      audio.loop();
      return true;     
    };
};
TaskAudio * myTaskAudio;
//============================================
void button_loop(){
  //Serial.printf("millis=%lu gpio12=%d\r",millis(),gpio12);
  if(digitalRead(32)==1){
    //Serial.printf("GPIIO32=1\r");  
    gpio32 = gpio32 < 10 ? gpio32 + 1 : 10; 
  }else{
    //Serial.printf("GPIIO32=0\r");  
    gpio32 = gpio32 > 0 ? gpio32 - 1 : 0; 
  }
  if( (gpio32 == 10 || gpio32 == 0) && gpio32 != last_gpio32){
    //Serial.printf("gpio32 turned from %d to %d\r\n",last_gpio32,gpio32);

    if(last_gpio32 == 10 and gpio32 == 0){
      // someone pushed the button
      Serial.println("last_gpio32 == 10 and gpio32 == 0 -> cambia stazione");
      // find the next not empty url
      for(int i=0; i<5;i++){
         station = (station+1)%5;
         if(stations[station] != "")
             break;
      }   
      saveCredentials();
      audio.connecttohost(stations[station]);
    }
    last_gpio32 = gpio32;
  }
}

class  MyTaskButton:MyTask{
  public:
    void (*_myCallback)();
    ~MyTaskButton(){};
    MyTaskButton(unsigned long interval, Scheduler* aS, void (* myCallback)() )
                  :  MyTask(interval, aS, myCallback){};
    
} * myTaskButton;
//==============================================
void setup() {
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();

    audio.setPlaySampleCallback(playSampleCallback);
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(15); // 0...21

    strips[0] = new Adafruit_NeoPixel(12, WS2812LeftRing_pin, NEO_GRB + NEO_KHZ800);
    strips[1] = new Adafruit_NeoPixel(12, WS2812RightRing_pin, NEO_GRB + NEO_KHZ800);

    strips[0]->begin();
    strips[1]->begin();
    strips[0]->setBrightness(255);
    strips[1]->setBrightness(255);
    strips[0]->show(); // Initialize all pixels to 'off'
    strips[1]->show(); // Initialize all pixels to 'off'

// MEDITATION
//    audio.connecttohost("http://hirschmilch.de:7000/chillout.aac"); //  128k aac
//    audio.connecttohost("http://uk2.internet-radio.com:31491"); //  128k mp3
//    audio.connecttohost("http://uk5.internet-radio.com:8347/");
     
//    audio.connecttospeech("Wenn die Hunde schlafen, kann der Wolf gut Schafe stehlen.", "de");
//    audio.connecttohost("http://media.ndr.de/download/podcasts/podcast4161/AU-20190404-0844-1700.mp3"); // podcast

  CaptivePortalSetup(); 
  _PL("TaskScheduler Audio Task");
  myTaskAudio = new TaskAudio(10,&myScheduler);

  pinMode(32, INPUT_PULLUP); // the button will short to ground
  myTaskButton = new MyTaskButton(20,&myScheduler,button_loop); 

}

void loop()
{
    myScheduler.execute();
    
}

// optional
void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}
void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}
void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}
void audio_showstreaminfo(const char *info){
    Serial.print("streaminfo  ");Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}
void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}
void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}
void audio_eof_speech(const char *info){
    Serial.print("eof_speech  ");Serial.println(info);
}