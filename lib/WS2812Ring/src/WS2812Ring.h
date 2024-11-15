#include "Adafruit_NeoPixel.h"
/*
#define _TASK_SLEEP_ON_IDLE_RUN // Enable 1 ms SLEEP_IDLE powerdowns between tasks if no callback methods were invoked during the pass 
#define _TASK_STATUS_REQUEST    // Compile with support for StatusRequest functionality - triggering tasks on status change events in addition to time only
#define _TASK_WDT_IDS           // Compile with support for wdt control points and task ids
#define _TASK_PRIORITY          // Support for layered scheduling priority
#define _TASK_TIMEOUT           // Support for overall task timeout 
#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>
*/

/*
class TaskWS2812Ring : public Task {
  public:
    void (*_myCallback)();
    ~TaskWS2812Ring();
    TaskWS2812Ring(unsigned long interval, Scheduler* aS, void (* myCallback)() );// :  Task(interval, TASK_FOREVER, aS, true) {
    //  _myCallback = myCallback;
    //};
    bool Callback();//
    //  _myCallback();
    //  return true;     
    //};
};
*/

void showbands(Adafruit_NeoPixel * strip, double bands[], int num_bands, double brightness);

void WS2812Ring_loop();

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait);

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos);

void rainbow(uint8_t wait);

// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait);

//Theatre-style crawling lights.
void theaterChase(uint32_t c, uint8_t wait);

//Theatre-style crawling lights with rainbow effect
void theaterChaseRainbow(uint8_t wait);
