# Funny Spectrum (audio spectrum viewed by colors)

This is an Internet Radio receiver based on ESP32 (D1 Mini),  PCM5102 I2S DAC and WS2812 to display the audio spectrum by colors.

### Show

[![Watch the video](https://github.com/guido57/Funny-Spectrum/blob/master/pictures/Internet%20Radio%20ESP32%20I2S%20PCM5102%20WS2812.PNG)](https://youtu.be/EDWJl283ymY)

### Overview
It is mostly forked from https://github.com/schreibfaul1/ESP32-audioI2S but I added something very distinctive:

1) a local web server with two pages: 
- one to change the WIFI credentials 
- and the other one to set up to 5 different internet radio stations

2) a button to go to the next radio station 

3) two WS2812 12 LEDs rings to display the audio spectrum 

### Schematics

[![Watch](https://github.com/guido57/Funny-Spectrum/blob/master/pictures/fritzing%20schematics.PNG)](https://github.com/guido57/Funny-Spectrum/blob/master/pictures/fritzing%20schematics.PNG)

### Very basic web pages

## Web page at the root of the web server (i.e. the ip address got by ESP32), to set / change up to five internet radios and set the volume
[![Watch](https://github.com/guido57/Funny-Spectrum/blob/master/pictures/Choose%20your%20Internet%20Radio.PNG)](https://github.com/guido57/Funny-Spectrum/blob/master/pictures/Choose%20your%20Internet%20Radio.PNG)

## Web page at the /wifi url to set / change the WIFI hotspot
If no Internet is available, the device becomes an access point, which you can connect to by smartphone or PC and access this page at http://192.168.4.1

[![Watch](https://github.com/guido57/Funny-Spectrum/blob/master/pictures/Internet%20settings.PNG)](https://github.com/guido57/Funny-Spectrum/blob/master/pictures/Internet%20settings.PNG)


### Issues
- There's no button or anything else to turn it on/off
- a) "FFT calculations"  b) "http receiving" c) "MP3/AAC decoding" run concurrently on core 1. Therefore LEDs refreshing rate must be limited at 226 msecs (10000/44100) to avoid audio clicks 

### TODO 
- Add a clock to turn off and on at specific time
- Move FFT calculations on core 0 


