#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include "WS2812Ring.h"

extern Adafruit_NeoPixel *strip;


// ===================================================
// tosk run by Taskscheduler to handle the WS2812 LED ring
/*
TaskWS2812Ring::~TaskWS2812Ring(){};
TaskWS2812Ring::TaskWS2812Ring(unsigned long interval, Scheduler *aS, void (*myCallback)()) : Task(interval, TASK_FOREVER, aS, true)
{
  _myCallback = myCallback;
};
bool TaskWS2812Ring::Callback()
{
  _myCallback();
  return true;
};
*/

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip->numPixels(); i++) {
    strip->setPixelColor(i, c);
    strip->show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return strip->Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return strip->Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return strip->Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip->numPixels(); i++) {
      strip->setPixelColor(i, Wheel((i+j) & 255));
    }
    strip->show();
    delay(wait );
    
  }
}

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;
  //for(int br=63;br<256;br+=64){
  //  Serial.printf("brightness=%d\n",br);
    for(j=0; j<256*5; j++) { // 5 cycles of all colors on wheel
      for(i=0; i< strip->numPixels(); i++) {
        strip->setPixelColor(i, Wheel(((i * 256 / strip->numPixels()) + j) & 255));
      }
      //WS2812Ring::strip->setBrightness(br);
      strip->show();
      delay(wait);
    }
  //}
}

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip->numPixels(); i=i+3) {
        strip->setPixelColor(i+q, c);    //turn every third pixel on
      }
      strip->show();

      delay(wait);

      for (uint16_t i=0; i < strip->numPixels(); i=i+3) {
        strip->setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}


//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait) {
  for (int j=0; j < 256; j++) {     // cycle all 256 colors in the wheel
    for (int q=0; q < 3; q++) {
      for (uint16_t i=0; i < strip->numPixels(); i=i+3) {
        strip->setPixelColor(i+q, Wheel( (i+j) % 255));    //turn every third pixel on
      }
      strip->show();

      delay(wait);
      
      for (uint16_t i=0; i < strip->numPixels(); i=i+3) {
        strip->setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}

//void WS2812Ring::WS2812Ring_loop(){
void WS2812Ring_loop(){
  // Some example procedures showing how to display to the pixels:
  //Serial.println("ColorWipe Red");
  //colorWipe(strip.Color(255, 0, 0), 50); // Red
  //Serial.println("ColorWipe Green");
  //colorWipe(strip.Color(0, 255, 0), 50); // Green
  //Serial.println("ColorWipe Blue");
  //colorWipe(strip.Color(0, 0, 255), 50); // Blue
  //colorWipe(strip.Color(0, 0, 0, 255), 50); // White RGBW
  // Send a theater pixel chase in...
  //Serial.println("theaterChase White");
  //theaterChase(strip.Color(127, 127, 127), 50); // White
  //Serial.println("theaterChase Red");
  //theaterChase(strip.Color(127, 0, 0), 50); // Red
  //Serial.println("theaterChase Blue");
  //theaterChase(strip.Color(0, 0, 127), 50); // Blue
  Serial.println("rainbow");
  rainbow(50);
  Serial.println("rainbowCycle");
  rainbowCycle(10);
  //Serial.println("theaterChaseRainbow");
  //theaterChaseRainbow(50);
};

