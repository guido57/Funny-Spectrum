#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include "captive_portal.h"
#include "mytask.h"
#include "WS2812Ring.h"
#include "myfft.h"

// Digital I/O used for MAX98357
//#define I2S_DOUT      27  // DIN connection
//#define I2S_BCLK      14  // Bit clock
//#define I2S_LRC       12  // Left Right Clock

// Digital I/O used for PCM5102
#define I2S_DOUT      0  // DIN connection
#define I2S_BCLK      4  // Bit clock
#define I2S_LRC       2  // Left Right Clock

// W22812 12 LEDs RING
//  GPIO33 for ESP32 - SERIAL DATA PIN
int WS2812LeftRing_pin = 33;
int WS2812RightRing_pin = 14;
Adafruit_NeoPixel * strips[2];

// ----------------------------------------------------------------------------
// read 12 double values from bands and light 12 LEDs of strip
// ----------------------------------------------------------------------------
void showbands(Adafruit_NeoPixel * strip, double bands[12]){

  //Serial.println("Computed bands:");
  double max = 0;
  //double avg = 0;
  
  // evaluate bands  
  for(int j=0; j<num_bands;j++){
      if(bands[j] > max) max = bands[j];
  }
  // set LEDs according to bands
  for(int j=0; j<num_bands;j++){
    bands[j] = bands[j] / 4000;
    if(bands[j]>255) bands[j] = 255;      
    bands[j] = (bands[j]*bands[j])/(255.0);
  }
  strip->setPixelColor(0, strip->Color(bands[0],0,0));                                     // RED
  strip->setPixelColor(1, strip->Color(bands[1]*3/4,bands[1]*1/4,                0)); //ORANGE        
  strip->setPixelColor(2, strip->Color(bands[2]*2/4,bands[2]*2/4,                0));         
  strip->setPixelColor(3, strip->Color(bands[3]*1/4,bands[3]*3/4,                0));         
  strip->setPixelColor(4, strip->Color(                0,bands[4]    ,                0)); //GREEN        
  strip->setPixelColor(5, strip->Color(                0,bands[5]*3/4,bands[5]*1/4));         
  strip->setPixelColor(6, strip->Color(                0,bands[6]*2/4,bands[5]*2/4));         
  strip->setPixelColor(7, strip->Color(                0,bands[7]*1/4,bands[7]*3/4));         
  strip->setPixelColor(8, strip->Color(                0,                0,bands[8]    )); // BLUE         
  strip->setPixelColor(9, strip->Color(bands[9]*1/4,                0,bands[9]*3/4));         
  strip->setPixelColor(10, strip->Color(bands[10]*2/4,               0,bands[10]*2/4));         
  strip->setPixelColor(11, strip->Color(bands[11]*3/4,                0,bands[11]*1/4));         
  strip->show();
}

// -------------------------------------------------------------------------------------------------
// receive a stereo sample and use it to light the LEDs
// store 1024 stereo samples, calculate FFT, compute 12 audio bands and show on WS2812 12 LEDs rings  
// --------------------------------------------------------------------------------------------------
unsigned long count_samples = 0;
void playSampleCallback(int16_t sample[2]){
  
  uint16_t ndx = count_samples % 2048;
  // read 1024 samples, decimating by 2 (i.e. sampling at 22050 Hz)
  if(ndx%2 == 0){
    real_fft_plan_left->input[ndx>>1] = sample[0];  // left channel
    real_fft_plan_right->input[ndx>>1] = sample[1]; // right channel

    if(ndx == 2046){
      // now vReal contains 1024 samples -> it's time to compute the FFT
      //unsigned long start_millis = millis();
      myFFT(real_fft_plan_left, benchmark_mags_left);
      myFFT(real_fft_plan_right, benchmark_mags_right);
      Compute12bands(benchmark_mags_left, samples/2,bands);
      showbands(strips[0],bands);
      Compute12bands(benchmark_mags_right, samples/2,bands);
      showbands(strips[1],bands);
      //Serial.printf("FFT on 1024 samples and light LEDs took %lu msecs\r\n",millis()-start_millis);
    }
  }
  //Serial.printf("sample #%lu at millis=%lu \r\n",count_samples, millis());
  count_samples++;
}

// =====================================================
//  AUDIO SECTION
Audio audio;

// ---------------------------------------------
// tosk run by Taskscheduler to handle Audio
// ---------------------------------------------
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
//============================================
//push button
int gpio32 = 1;
int last_gpio32 = 1;

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
      audio.connecttohost(stations[station].c_str());
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

  CaptivePortalSetup(); 
  _PL("TaskScheduler Audio Task");
  myTaskAudio = new TaskAudio(10,&myScheduler);

  pinMode(32, INPUT_PULLUP); // the button will short to ground
  myTaskButton = new MyTaskButton(20,&myScheduler,button_loop); 

  real_fft_plan_left = fft_init((int)samples, FFT_REAL,(fft_direction_t) FFT_FORWARD, NULL, NULL);
  real_fft_plan_right = fft_init((int)samples, FFT_REAL,(fft_direction_t) FFT_FORWARD, NULL, NULL);
}

void loop()
{
    myScheduler.execute();
}

