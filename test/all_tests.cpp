#include <unity.h>


void test_fft(void);
void test_increasing_intensity(void);


int main() {
}
void setup(){
    UNITY_BEGIN();

    RUN_TEST(test_fft);
    RUN_TEST(test_increasing_intensity);
    
    UNITY_END();
}

void loop(){

}