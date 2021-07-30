#ifndef MY_SETINGS_H
#define MY_SETTINGS_H

#include <Arduino.h>


class Settings {
  public:

    /* GLOBAL SETTINGS */
    struct entries_struct {
      char ssid[32] = "my_ssid";     // don't modify this. It will be set by a ESP-NOW message
      char pwd[32] = "my_ssid_pwd";  // don't modify this. It will be set by a ESP-NOW message
      char MQTTServer[32] = "the_MQTT_server";
      int MQTTServerPort = 8000;
      char MQTTMainTopic[64] = "the_MQTT_main_topic";
      char MQTTSubscribedTopic[64] = "the_MQTT_subscribed_topic";
      // From here on add or delete entries as you like. Don't use String but use char xyz[N] instead.
      // if you want to update them by esp-now messages, add a specific section in EspNow::Loop as well
      char stations[5][128];
      int volume = 10;
      int station = 0;
      uint8_t brightness = 20; // 0 ... 255

      // On and Off Hours and Minutes
      char hh_on[3] = "00", mm_on[3]="00", hh_off[3]="00", mm_off[3]="00";

    } entries;
    
    Settings();
    ~Settings();
    void Init();

    void Load();
    void Save();

};


#endif