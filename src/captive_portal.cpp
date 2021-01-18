#include "captive_portal.h"
#include <Audio.h>

#include <TaskScheduler.h>
Scheduler myScheduler;
//=== CaptivePortal stuff ================
String softAP_ssid;
String softAP_password  = APPSK;

/* hostname for mDNS. Should work at least on windows. Try http://esp32.local */
const char *myHostname = "esp32";

/* Don't set these wifi credentials. They are configurated at runtime and stored on EEPROM */
String ssid;
String password;

String stations[5];
int volume = 10;
int station = 0;

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
  }else{
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
      audio.connecttohost(stations[station]);
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
  web_server.handleClient();
  
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
  String ok = EEPROM.readString(len); // load ok. ok means that stored data are valid
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
  }
  
  Serial.println("Recovered settings:");
  Serial.println(ssid.length() > 0 ? ssid : "<no ssid>");
  Serial.println(password.length() > 0 ? password : "<no password>");
  for(int i=0;i<5;i++)
    Serial.printf("stations[%d]=%s\r\n",i,stations[i].c_str()) ;
  Serial.printf("Selected station = %d\r\n",station);   
  Serial.printf("Volume = %d\r\n", volume);   
}

/** Store WLAN credentials to EEPROM */
void saveCredentials() {

  Serial.println("Saving settings ...");
  /*
  Serial.printf("ssid=%s\r\n",ssid.c_str());
  Serial.printf("password=%s\r\n",password.c_str());
  for(int i=0;i<5;i++)
    Serial.printf("stations[%d]=%s\r\n",i,stations[i].c_str()) ;
  Serial.printf("Selected station = %d\r\n",station);   
  Serial.printf("Volume = %d\r\n", volume); 
  */
  EEPROM.begin(2048);
  size_t len = 0;
  len += EEPROM.writeString(len, ssid) + 1;
  len += EEPROM.writeString(len, password) + 1;
  for(int i=0; i<5;i++){
    len += EEPROM.writeString(len,stations[i]) + 1;
  }
  len += EEPROM.writeInt(len, volume);
  len += EEPROM.writeInt(len, station);
  EEPROM.writeString(len,"OK"); 
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