# Funny Spectrum (audio spectrum viewed by colors)

This is an Internet Radio receiver based on ESP32 (D1 Mini),  PCM5102 I2S DAC and WS2812 to display the audio spectrum by colors.

### Show

[![](https://github.com/guido57/Funny-Spectrum/blob/master/pictures/Internet%20Radio%20ESP32%20I2S%20PCM5102%20WS2812.PNG)](https://github.com/guido57/Funny-Spectrum/blob/master/pictures/Internet%20Radio%20ESP32%20I2S%20PCM5102%20WS2812.PNG)

See the video, also.

[![Watch the video](https://youtu.be/EDWJl283ymY)](https://youtu.be/EDWJl283ymY)

[![Watch the video](https://github.com/guido57/Funny-Spectrum/blob/master/pictures/Internet%20Radio%20ESP32%20I2S%20PCM5102%20WS2812.PNG)](https://youtu.be/vt5fpE0bzSY)

### Overview
It is mostly forked from https://github.com/schreibfaul1/ESP32-audioI2S.

I added:

1) a local web server with two pages: 
- one to change the WIFI credentials 
- and the other one to set up to 5 different internet radio stations

2) a button to go to the next radio station 

3) two WS2812 12 LEDs rings to display the audio spectrum 

### Issues
- It's always on
- a) "FFT calculations"  b) "http receiving" c) "MP3/AAC decoding" run concurrently on core 1. Therefore LEDs refreshing rate must be limited at 226 msecs (10000/44100) to avoid audio clicks 

### TODO 
- Add a clock to turn off and on at specific time
- Move FFT calculations on core 0 


### Schematics

[![](https://github.com/guido57/InternetRadioI2S-PCM5102/blob/master/schematics/InternetRadioPCM5102-I2S-Schematic.png)](https://github.com/guido57/InternetRadioI2S-PCM5102/blob/master/schematics/InternetRadioPCM5102-I2S-Schematic.png)
