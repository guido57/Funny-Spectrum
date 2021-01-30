#include <WS2812Ring.h>
#include <unity.h>

// W22812 12 LEDs RING
//  GPIO33 for ESP32 - SERIAL DATA PIN
int WS2812LeftRing_pin = 33;
int WS2812RightRing_pin = 14;
Adafruit_NeoPixel * strips[2];
const int num_bands = 12;
double bands[num_bands];

void test_all_bands_on_off(void) {
    for(int i = 0; i< num_bands;i++)
        bands[i] = 500000;
    showbands(strips[0],bands,num_bands);
    for(int i = 0; i< num_bands;i++)
        bands[i] = 500000;
    showbands(strips[1],bands,num_bands);
    delay(100000);
    for(int i = 0; i< num_bands;i++)
        bands[i] = 0;
    showbands(strips[0],bands,num_bands);
    for(int i = 0; i< num_bands;i++)
        bands[i] = 0;
    showbands(strips[1],bands,num_bands);
    TEST_ASSERT_EQUAL(32, 32);
}


int main() {
    UNITY_BEGIN();

    strips[0] = new Adafruit_NeoPixel(12, WS2812LeftRing_pin, NEO_GRB + NEO_KHZ800);
    strips[1] = new Adafruit_NeoPixel(12, WS2812RightRing_pin, NEO_GRB + NEO_KHZ800);

    strips[0]->begin();
    strips[1]->begin();
    strips[0]->setBrightness(255);
    strips[1]->setBrightness(255);
    strips[0]->show(); // Initialize all pixels to 'off'
    strips[1]->show(); // Initialize all pixels to 'off'


    RUN_TEST(test_all_bands_on_off);
    //RUN_TEST(test_function_calculator_subtraction);
    //RUN_TEST(test_function_calculator_multiplication);
    //RUN_TEST(test_function_calculator_division);

    UNITY_END();
}
void setup(){

    
    main();

}

void loop(){

}