#include "captive_portal.h"
#include <Audio.h>

#include <TaskScheduler.h>
Scheduler myScheduler;
//=== CaptivePortal stuff ================
String softAP_ssid;
String softAP_password  = APPSK;

/* hostname for mDNS. Should work at least on windows. Try http://esp32.local */
const char *myHostname = "esp32";

// NTP settings
const char* ntpServer = "europe.pool.ntp.org";
const long  gmtOffset_sec = 3600; // GMT + 1
//Change the Daylight offset in milliseconds. If your country observes Daylight saving time set it to 3600. Otherwise, set it to 0.
const int   daylightOffset_sec = 3600;

/* Don't set these wifi credentials. They are configurated at runtime and stored on EEPROM */
String ssid;
String password;

String stations[5];
int volume = 10;
int station = 0;
uint8_t brightness = 20; // 0 ... 255

// On and Off Hours and Minutes
String hh_on = "00", mm_on="00", hh_off="00", mm_off="00";
bool audio_light_on = false;

// DNS server
const byte DNS_PORT = 53;
DNSServer dnsServer;

// Web server
WebServer web_server(80);

/* Setup the Access Point */
void AccessPointSetup();

/* Soft AP network parameters */
IPAddress apIP(192, 168, 4, 1);
IPAddress netMsk(255, 255, 255, 0);
/** Should I connect to WLAN asap? */
boolean connect;

/** Last time I tried to connect to WLAN */
unsigned long lastConnectTry = 0;

/** Last time I tried to ping an external IP address (usually the gateway) */
unsigned long lastPing = 0;

/** Current WLAN status */
unsigned int status = WL_IDLE_STATUS;

extern Audio audio;

// =====================================================
void connectWifi() {
  if(ssid == ""){
   Serial.println("SSID is empty!");
   ssid = "TIM-18373419";
   password = "K9giYCTW4ryRS1MT26oIs7BG";
  }
  
  {
    Serial.println("Connecting as wifi client...");
    //WiFi.forceSleepWake();
    WiFi.disconnect();
    Serial.print("ssid=");Serial.println(ssid);
    Serial.print("password=");Serial.println(password);
    WiFi.begin(ssid.c_str(), password.c_str());
    int connRes = WiFi.waitForConnectResult();
    Serial.print("connRes: ");
    Serial.println(connRes);
  }
}


void printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

// =====================================================
// Callback for the TaskWIFI  
/**
   check WIFI conditions and try to connect to WIFI.
 * @return void
 */
void WiFi_loop(void){

  // handle a connect request (request is true)
  if (connect) {
    Serial.println("Connect requested");
    connect = false;
    connectWifi();
    lastConnectTry = millis();
  }
  // if not connected try to connect after 60 seconds from last attempt  
  unsigned int s = WiFi.status();
  if (s != WL_CONNECTED && millis() > (lastConnectTry + 60000))
  {
    /* If WLAN disconnected and idle try to connect */
    /* Don't set retry time too low as retry interfere the softAP operation */
    connect = true;
  }

  // if connected, try to ping an external IP (usually the gsteway)
  /*
  if (s==WL_CONNECTED && millis() > lastPing + 60000){
    _PL("ping 192.168.1.1 for 5 times");
    bool pingok = true;
    for (int i = 0; i < 5; i++)
    {
      if (!Ping.ping("192.168.1.1")){
        pingok = false;
        break;
      }else
      {
        Serial.printf("ping time=%.0f\r\n",Ping.averageTime());
      }
        
    }
    if (pingok){
      _PL("Ping success!");
    }
    else{
      _PL("Ping error :( -> turn on the Access Point");
      WiFi.mode(WIFI_MODE_APSTA);
    }
    lastPing = millis();
  }
  */
  // if WLAN status changed
  if (status != s)
  {
    Serial.printf("Status changed from %d to %d:\r\n",status,s);
    status = s;
    if (s == WL_CONNECTED){
      /* Just connected to WLAN */
      Serial.printf("\r\nConnected to %s\r\n",ssid.c_str());
      _PP("IP address");
      Serial.println(WiFi.localIP());


      //const char * defaultTimezone = "CET-1CEST,M3.5.0/2,M10.5.0/3";

      //init and get the time
      //configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
      configTzTime( "GMT-1", ntpServer); //sets TZ and starts NTP sync
      printLocalTime();

      // Setup MDNS responder
      /*
          if (!MDNS.begin(myHostname)) {
            Serial.println("Error setting up MDNS responder!");
          } else {
            Serial.println("mDNS responder started");
            // Add service to MDNS-SD
            MDNS.addService("http", "tcp", 80);
          }
          */
      _PL("just connected -> Turn off the Access Point")
      WiFi.mode(WIFI_MODE_STA);

      // start the radio
      audio.connecttohost(stations[station].c_str());
      audio.setVolume(volume);
    }
    else if (s == WL_NO_SSID_AVAIL){
      _PL("no SSID available -> turn on the Access Point");
      WiFi.disconnect();
      WiFi.mode(WIFI_MODE_APSTA);
    }
    else{
      _PL("not connected -> turn on the Access Point");
      WiFi.mode(WIFI_MODE_APSTA);

    }
  }

  if (s == WL_CONNECTED){
    //MDNS.update();

  }
  // Do work:
  //DNS
  dnsServer.processNextRequest();
  //HTTP
  //web_server.Loop();
  web_server.handleClient();

  tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
  }else{
    // check if we ar ON or OFF
    uint16_t mmm_on = hh_on.toInt() * 60 + mm_on.toInt();
    uint16_t mmm_off = hh_off.toInt() * 60 + mm_off.toInt();
    int16_t now_t = timeinfo.tm_hour * 60 + timeinfo.tm_min;
    //Serial.printf("now_t=%d mmm_on=%d mmm_off=%d\r\n",now_t,mmm_on,mmm_off);
    if(     (mmm_on < mmm_off &&  now_t >= mmm_on && now_t < mmm_off) 
         || (mmm_on > mmm_off && (now_t >= mmm_on || now_t < mmm_off)) 
      ){
      if(audio_light_on == false){
        Serial.println("Turn on radio and lights");
        if(!audio.isRunning())
          audio.pauseResume();
        audio.connecttohost(stations[station].c_str());  
        audio_light_on = true;  
      }      
    }else{
      if(audio_light_on)  
        Serial.println("Turn off radio and lights");
      if(audio.isRunning()){
        audio.pauseResume();
        
      }
      audio_light_on = false;  
    }
  }
}
// =====================================================
// tosk run by Taskscheduler to handle WIFI  
class TaskWiFi : public Task {
  public:
    void (*_myCallback)();
    ~TaskWiFi() {};
    TaskWiFi(unsigned long interval, Scheduler* aS, void (* myCallback)() ) :  Task(interval, TASK_FOREVER, aS, true) {
      _myCallback = myCallback;
    };
    bool Callback(){
      _myCallback();
      return true;     
    };
};
Task * myTaskWiFi;

//===================================================
/** Load WLAN credentials from EEPROM */
void loadCredentials() {
  EEPROM.begin(2048);
  size_t len = 0;
  ssid = EEPROM.readString(len); // load ssid
  len += ssid.length() + 1;
  password = EEPROM.readString(len); // load password
  len += password.length() + 1;
  for(int i=0;i<5;i++){
    stations[i] = EEPROM.readString(len); // load station. 
    //printf("got station[%d]=%s\r\n",i,stations[i].c_str());
    len += stations[i].length() + 1;
  }
  volume = EEPROM.readInt(len); // load volume 
  len += sizeof(volume);
  station = EEPROM.readInt(len); // load selected station index 
  len += sizeof(station);

  hh_on = EEPROM.readString(len); // load hh_on
  len += hh_on.length() + 1;
  mm_on = EEPROM.readString(len); // load mm_on
  len += mm_on.length() + 1;
  hh_off = EEPROM.readString(len); // load hh_off
  len += hh_off.length() + 1;
  mm_off = EEPROM.readString(len); // load mm_off
  len += mm_off.length() + 1;

  String ok = EEPROM.readString(len); // load ok. ok means that stored data are valid
  len += ok.length() + 1;

  brightness = EEPROM.readShort(len);

  //printf("got ok=%s\r\n",ok.c_str());
  
  EEPROM.end();

  if (ok != String("OK")) {
    Serial.printf("ok=%s != OK -> set default settings\r\n",ok.c_str());
    ssid = "";
    password = "";
    volume = 10;
    station = 0;
    stations[0] = "http://uk5.internet-radio.com:8347/";
    for(int i = 1;i<5;i++)
      stations[i] = "";
    hh_on = "00";
    hh_off = "00";
    mm_on = "00";
    mm_off = "00";
  }
  
  Serial.println("Recovered settings:");
  Serial.println(ssid.length() > 0 ? ssid : "<no ssid>");
  Serial.println(password.length() > 0 ? password : "<no password>");
  for(int i=0;i<5;i++)
    Serial.printf("stations[%d]=%s\r\n",i,stations[i].c_str()) ;
  Serial.printf("Selected station = %d\r\n",station);   
  Serial.printf("Volume = %d\r\n", volume);   
  Serial.printf("hh_on = %s\r\n", hh_on.c_str());   
  Serial.printf("mm_on = %s\r\n", mm_on.c_str());   
  Serial.printf("hh_off = %s\r\n", hh_off.c_str());   
  Serial.printf("mm_off = %s\r\n", mm_off.c_str());   
}

/** Store WLAN credentials to EEPROM */
void saveCredentials() {

  Serial.println("Saving settings ...");
  
  Serial.printf("ssid=%s\r\n",ssid.c_str());
  Serial.printf("password=%s\r\n",password.c_str());
  for(int i=0;i<5;i++)
    Serial.printf("stations[%d]=%s\r\n",i,stations[i].c_str()) ;
  Serial.printf("Selected station = %d\r\n",station);   
  Serial.printf("Volume = %d\r\n", volume); 
  Serial.printf("ON %s:%s\r\n", hh_on.c_str(),mm_on.c_str()); 
  Serial.printf("OFF %s:%s\r\n", hh_off.c_str(),mm_off.c_str()); 
  
  EEPROM.begin(2048);
  size_t len = 0;
  len += EEPROM.writeString(len, ssid) + 1;
  len += EEPROM.writeString(len, password) + 1;
  for(int i=0; i<5;i++){
    len += EEPROM.writeString(len,stations[i]) + 1;
  }
  len += EEPROM.writeInt(len, volume);
  len += EEPROM.writeInt(len, station);
  len += EEPROM.writeString(len, hh_on) + 1;
  len += EEPROM.writeString(len, mm_on) + 1;
  len += EEPROM.writeString(len, hh_off) + 1;
  len += EEPROM.writeString(len, mm_off) + 1;
  len += EEPROM.writeString(len,"OK") + 1; 
  EEPROM.writeByte(len, brightness);
  EEPROM.commit();
  EEPROM.end();
}

void AccessPointSetup(){

  softAP_ssid = "ESP32_" + WiFi.macAddress();

  // Access Point Setup
  if(WiFi.getMode() != WIFI_MODE_AP && WiFi.getMode() != WIFI_MODE_APSTA){
      
    WiFi.mode(WIFI_MODE_AP);
    
    /* You can remove the password parameter if you want the AP to be open. */
    WiFi.softAP(softAP_ssid.c_str(), softAP_password.c_str());
    delay(2000); 
    WiFi.softAPConfig(apIP, apIP, netMsk);
    delay(100); // Without delay I've seen the IP address blank
    Serial.println("Access Point set:");
  }else
  {
    Serial.println("Access Point already set:");
  }
  Serial.printf("    SSID: %s\r\n", softAP_ssid.c_str());
  Serial.print("    IP address: ");
  Serial.println(WiFi.softAPIP());
}

void CaptivePortalSetup(){

  AccessPointSetup();
  /* Setup the DNS web_server redirecting all the domains to the apIP */
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(DNS_PORT, "*", apIP);

  /* Setup web pages: root, wifi config pages, SO captive portal detectors and not found. */
  web_server.on("/", handleRoot);
  web_server.on("/settingssave", handleSettingsSave);
  web_server.on("/wifi", handleWifi);
  web_server.on("/wifisave", handleWifiSave);
  web_server.on("/generate_204", handleRoot);  //Android captive portal. Maybe not needed. Might be handled by notFound handler.
  web_server.on("/fwlink", handleRoot);  //Microsoft captive portal. Maybe not needed. Might be handled by notFound handler.
  web_server.onNotFound(handleNotFound);
  web_server.begin(); // Web server start
  Serial.println("HTTP server started");
  loadCredentials(); // Load WLAN credentials from network
  connect = ssid.length() > 0; // Request WLAN connect if there is a SSID

  _PL("TaskScheduler WIFI Task");
  myTaskWiFi = new TaskWiFi(30,&myScheduler,WiFi_loop);
}