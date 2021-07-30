#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <ArduinoJson.h>
#include <Settings.h>


// Structure to send data
// Must match the receiver structure
typedef struct esp_now_message {
  char key[32];
  char value[128];
} esp_now_message;

#define ESP_NOW_TIMEOUT 1000 // milliseconds

class EspNow {
  public:

    Settings  settings;   
    uint8_t mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00} ;
    bool esp_now_ready = false;
    QueueHandle_t rx_queue;
    
    EspNow();
    ~EspNow();
    void Init();

    void LoadCredentials();
    void SaveCredentials();

    // get the WiFi channel (1 ... 14) of the specified ssid
    uint8_t GetWiFiChannel(String ssid);

    // callback when data is sent
    bool OnDataSentResult = false;
    bool OnDataSentReady = false;
  
    void OnDataReceived(const uint8_t * _mac, const uint8_t *incomingData, int len);
    void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);

    bool Send(uint8_t mac[6], uint8_t channel, esp_now_message sm);
    
    /* Change WiFi channel 
    *  return true if success, false otherwise
    */
    bool changeChannel(uint8_t new_channel);
    bool AddModPeer(uint8_t mac[6], uint8_t channel);

    void Loop();

};

extern EspNow * espNow;


/* 
  Get the WiFi channel for this ssid
  if not found return -1 
*/
uint8_t GetWiFiChannel(String ssid);
