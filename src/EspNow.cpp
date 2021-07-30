#include <Arduino.h>
#include <esp_wifi.h>
#include <WiFi.h>
#include <EspNow.h>


typedef struct struct_esp_now_message {
  char key[32];
  char value[128];
} struct_esp_now_message;

/* Get the WiFi channel for this ssid
   if not found return -1 
*/
uint8_t EspNow::GetWiFiChannel(String ssid){

  int n = WiFi.scanNetworks();
  for (int i = 0; i < n; ++i) {
    //Serial.printf("searching %s WiFi.SSID(%d)=%s\r\n",ssid.c_str(),i,WiFi.SSID(i).c_str());
    if(WiFi.SSID(i) == ssid){
      // found!
      return (uint8_t) WiFi.channel(i);
    }
    delay(10);
  }
  return (uint8_t) -1;
}

EspNow * espNow;

// Outside of class
// declare a pointer to EspNow class
EspNow * pointerToEspNowClass; 

// define global handler
// that calls class member handler
static void ExternalOnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  pointerToEspNowClass->OnDataSent(mac_addr, status); 
}

static void ExternalOnDataReceived(const uint8_t * mac, const uint8_t *incomingData, int len) {
  pointerToEspNowClass->OnDataReceived(mac, incomingData, len); 
}

EspNow::EspNow(){
    pointerToEspNowClass=this;
    this->rx_queue = xQueueCreate( 10, sizeof( esp_now_message ) );
    this->Init();
}

EspNow::~EspNow(){

}


void EspNow::Init(){
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(false);
  WiFi.persistent(false);
  
  //WiFi.mode(WIFI_AP_STA);
  //WiFi.softAP("waiting-for-ESP-NOW","testonaicco",1);
  esp_wifi_set_ps(WIFI_PS_NONE) ; // WIFI don't sleep, to be ready to receive ESP-NOW
  //Serial.printf("WiFiChannel is %d\r\n", WiFi.channel());

  settings.Load();
  Serial.printf("trying to connect to ssid %s with password %s\r\n", settings.entries.ssid,settings.entries.pwd); 
  WiFi.begin(settings.entries.ssid,settings.entries.pwd);
  WiFi.waitForConnectResult();
  if(WiFi.isConnected() == false){
    Serial.printf("error connecting to %s\r\n", settings.entries.ssid);  
    EspNow::changeChannel(1); // start from channel 1
  }else{
    Serial.printf("successfully connected to %s\r\n", settings.entries.ssid);  
  }


  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packet info
  esp_err_t err = esp_now_register_recv_cb(ExternalOnDataReceived);
  if(err == ESP_OK){
    //Serial.println("OnSent callback successfully registered\r\n");
  }else
    Serial.printf("Error %d registering OnDataReceived callback\r\n",err);

  err = esp_now_register_send_cb(ExternalOnDataSent);
  if(err == ESP_OK){
    //Serial.println("OnSent callback successfully registered\r\n");
  }else
    Serial.printf("Error %d registering OnDataSent callback\r\n",err);
}

bool EspNow::changeChannel(uint8_t new_channel){
  esp_wifi_set_promiscuous(true);
  esp_wifi_set_channel(new_channel, WIFI_SECOND_CHAN_NONE);
  esp_wifi_set_promiscuous(false);
  return (WiFi.channel() == new_channel);
  
}

/* if it doesn't exist, add this mac to the peers 
*  if it exists, change its channel  
*/
bool EspNow::AddModPeer(uint8_t mac[6], uint8_t channel){
    // create the peer struct
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, mac, 6);
    peerInfo.channel = channel;  
    peerInfo.encrypt = false;
    peerInfo.ifidx = ESP_IF_WIFI_STA;

    // check if this mac is already registered
    if(esp_now_is_peer_exist(mac)){
      //Serial.printf("The peer %02X:%02X:%02X:%02X:%02X:%02X already esists -> modify it\r\n",
      //    mac[0],mac[1],mac[2],mac[3], mac[4], mac[5]);
      esp_err_t result = esp_now_mod_peer(&peerInfo);  
      if(result != ESP_OK)
        Serial.println("error modifying peer");
    }else{
      // Add peer        
      //Serial.printf("Adding peer %02X:%02X:%02X:%02X:%02X:%02X channel %d\r\n",
      //    mac[0],mac[1],mac[2],mac[3], mac[4], mac[5], channel );
      //Serial.printf("requested channel is %d,  WiFi operating on channel %d\r\n",channel,WiFi.channel());
      
      esp_err_t result = esp_now_add_peer(&peerInfo);
      switch(result){
        case ESP_OK:
          //Serial.println("peer succesfully added");
          break;
        case ESP_ERR_ESPNOW_NOT_INIT :
          Serial.println("ESPNOW is not initialized");
          break;
        case ESP_ERR_ESPNOW_ARG :
          Serial.println("invalid argument");
          break;
        case ESP_ERR_ESPNOW_FULL : 
          Serial.println("peer list is full");
          break;
        case ESP_ERR_ESPNOW_NO_MEM : 
          Serial.println("out of memory");
          break;
        case ESP_ERR_ESPNOW_EXIST : 
          Serial.println("peer has existed");
          break;  
        default:
          Serial.println("Error adding ESP-NOW peer");
      }
    }
    return true;
}

/*
  Send a packet to mac on channel
  return true on sucessful delivery, false otherwise 
*/
bool EspNow::Send(uint8_t mac[6], uint8_t channel, esp_now_message sm){
    
  AddModPeer(mac,channel);
  OnDataSentReady = false;
    
  // send data
  esp_err_t result = esp_now_send(mac, (uint8_t *) &sm, sizeof(sm));
  switch(result){
    case ESP_OK: 
      //Serial.printf("%s %s sent with success to mac %2x:%2x:%2x:%2x:%2x:%2x\r\n",sm.key, sm.value,
      //    mac[0],mac[1],mac[2],mac[3], mac[4], mac[5]);
    break;
    default:
      Serial.printf("sending error result is %d\r\n",result);
    break;
  }
  unsigned long timeout_millis = millis() + ESP_NOW_TIMEOUT;
  while(OnDataSentReady == false){// wait until timeout_millis
    if(millis() > timeout_millis)
      break;
  }
  if(OnDataSentReady){
    return OnDataSentResult;// return delivery result 
  }
  return false; // on timeout return false
}

void EspNow::OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  EspNow::OnDataSentResult = (status == ESP_NOW_SEND_SUCCESS);
  EspNow::OnDataSentReady = true;
  Serial.printf("OnDataSentResult is %d to mac %2x:%2x:%2x:%2x:%2x:%2x\r\n", 
      EspNow::OnDataSentResult, mac_addr[0],mac_addr[1],mac_addr[2],mac_addr[3],mac_addr[4],mac_addr[5]);
}

// callback function that will be executed when ESP-NOW data is received
bool need_to_send_ack = false;
void EspNow::OnDataReceived(const uint8_t * _mac, const uint8_t *incomingData, int len) {
  esp_now_message sm;
  memcpy(&sm, incomingData, sizeof(sm));
  //Serial.printf("ESP-NOW message received from %02X:%02X:%02X:%02X:%02X:%02X\r\n",
  //                _mac[0],_mac[1],_mac[2],_mac[3],_mac[4],_mac[5] );
  //Serial.printf("ESP-NOW bytes received: %d at %lu\r\n",len,millis());
  //Serial.printf("key is %s  value is %s\r\n",sm.key, sm.value);

  // answer to the sending MAC, on the actual channel  
  memcpy(this->mac,_mac,6);
  xQueueSend(rx_queue, &sm, portMAX_DELAY);
}


// Examine and handle all the received messages   
void EspNow::Loop(){ 
  int waiting_messages = uxQueueMessagesWaiting(rx_queue); 
  if(waiting_messages>0){
    // a new message has been received -> send back an ack
    esp_now_message sm;
    esp_now_message sm_ack;
    strcpy(sm_ack.key,"");
    strcpy(sm_ack.value, "ack");
      
    xQueueReceive( rx_queue, &sm, portMAX_DELAY);
    Serial.printf("received key=%s value=%s\r\n",sm.key,sm.value);   
    if( 0 == strcmp(sm.key, "ssid")){ 
      strcpy(settings.entries.ssid, sm.value);  
      settings.Save();
      int ch_now = WiFi.channel(); // store the actual channel that will be changed by GetWiFiChannel
      // send back the channel used by this ssid
      strcpy(sm_ack.key,"ch");
      strcpy(sm_ack.value, String(GetWiFiChannel(String(settings.entries.ssid))).c_str());
      changeChannel(ch_now); // restore the previous channel that was changed by GetWiFiChannel
      Send(mac,WiFi.channel(),sm_ack);
    }else if( 0 == strcmp(sm.key, "pwd")){ 
      // send back an ack
      strcpy(sm_ack.key,"pwd");
      strcpy(sm_ack.value, "ack");
      Send(mac,WiFi.channel(),sm_ack);
      // use the password
      strcpy(settings.entries.pwd, sm.value); 
      settings.Save();
       esp_restart();
      
    }else if( 0 == strcmp(sm.key, "get_ip")){ 
        // send back a messsage with the assigned IP address
        esp_now_message sm_ack;
        strcpy(sm_ack.key, "ip");
        strcpy(sm_ack.value, WiFi.localIP().toString().c_str());
        Send(mac,WiFi.channel(),sm_ack);
    }else if( 0 == strcmp(sm.key, "mqtt_server")){ 
      strcpy(settings.entries.MQTTServer, sm.value);  
      settings.Save();
      strcpy(sm_ack.key,"mqtt_server");
      Send(mac,WiFi.channel(),sm_ack);
    }else if( 0 == strcmp(sm.key, "mqtt_port")){ 
      settings.entries.MQTTServerPort =  atoi(sm.value);  
      settings.Save();
      strcpy(sm_ack.key,"mqtt_port");
      Send(mac,WiFi.channel(),sm_ack);
    }else if( 0 == strcmp(sm.key, "mqtt_topic")){ 
      strcpy(settings.entries.MQTTMainTopic, sm.value);  
      settings.Save();
      strcpy(sm_ack.key,"mqtt_topic");
      Send(mac,WiFi.channel(),sm_ack);
    }else if( 0 == strcmp(sm.key, "mqtt_subscribed_topic")){ 
      strcpy(settings.entries.MQTTSubscribedTopic, sm.value);  
      settings.Save();
      strcpy(sm_ack.key,"mqtt_subscribed_topic");
      Send(mac,WiFi.channel(),sm_ack);
    }

    Serial.printf("WiFi Channel is %d\r\n", WiFi.channel()); 
  }
} 
