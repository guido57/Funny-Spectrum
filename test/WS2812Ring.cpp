#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif

#include "WS2812Ring.h"

extern Adafruit_NeoPixel *strip;



// ----------------------------------------------------------------------------
// read 12 double values from bands and light 12 LEDs of strip
// ----------------------------------------------------------------------------
void showbands(Adafruit_NeoPixel * strip, double bands[], int num_bands){

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

