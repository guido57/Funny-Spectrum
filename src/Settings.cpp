#include <Arduino.h>

#include <Settings.h>
#include <EEPROM.h>


Settings::Settings(){
}

Settings::~Settings(){

}


//===================================================
void Settings::Load() {
  EEPROM.begin(2048);
  entries_struct ss;
  EEPROM.get(0,ss);
  int len = sizeof(entries);
  String ok = EEPROM.readString(len); // load ok. ok means that stored data are valid
  EEPROM.end();

  if (ok == String("OK")) {
    Serial.printf("successfully loaded settinge from EEPROM\r\n");
    entries = ss;
  }else{  
    Serial.printf("ok=%s != OK -> set default settings\r\n", ok.c_str());
  }
}

/** Store WLAN credentials to EEPROM */
void Settings::Save() {
  EEPROM.begin(2048);
  EEPROM.put(0,entries);
  int len = sizeof(entries);
  EEPROM.writeString(len,"OK"); // load ok. ok means that stored data are valid
  EEPROM.commit();
  EEPROM.end();
}

