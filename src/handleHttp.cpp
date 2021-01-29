/** Handle root or redirect to captive portal */
#include <arduino.h>
#include <handleHttp.h>
#include <main.h>
#include <tools.h>
#include <Audio.h>

extern Audio audio;
extern int station;
#define S String
// ==================================================================================================
void handleRoot() {

  if (captivePortal()) { // If captive portal redirect instead of displaying the page.
    return;
  }
  web_server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  web_server.sendHeader("Pragma", "no-cache");
  web_server.sendHeader("Expires", "-1");

  String Page;
  Page += F(
            "<!DOCTYPE html><html lang='en'><head>"
            "<meta name='viewport' content='width=device-width'>"
            "<title>Choose your Internet Radio</title></head><body>"
            "<h1>Choose your Internet Radio!</h1>");
  if (web_server.client().localIP() == apIP) {
    Page += S("<p>You are connected through the soft AP: ") + softAP_ssid + S("</p>");
  } else {
    Page += S("<p>You are connected through the wifi network: ") + ssid + S("</p>");
  }
  Page += S(
            "<form method='POST' action='settingssave'>"
          );
  for(int i = 0; i <5; i++){
    Page += S("<input type='radio' ") + (station == i ? S(" checked = true ") : S("")) + S(" name='station' value='") + S(i) + S("'><input name='station") + S(i) +  S("' size=100 value='") + stations[i] + S("'><br>");
  }

  Page += S("Volume: <input type='range' min='0' max='21'  style='width:400px' name='volume' value='") + S(volume) + S("' ><br>");
  
  Page += F("ON HH:MM <select name='hh_on' id='hh_on'>");
  for(int hh = 0; hh<24;hh++){
    String hhs = (hh < 10 ? "0" : "") + String(hh);
    Page += S("<option value='" + hhs + "' " + (hhs == hh_on ? "selected" : "") +  + ">" + hhs + "</option>");
  }
  Page += F("</select>"
            " : <select name='mm_on' id='mm_on'>"
          );
  for(int mm = 0; mm<60;mm+=15){
    String mms = (mm < 15 ? "0" : "") + String(mm);
    Page += S("<option value='" + mms + "' " + (mms == mm_on ? "selected" : "") +  + ">" + mms + "</option>");
  }
  Page += F("</select><br>");
          
  Page += F("OFF HH:MM <select name='hh_off' id='hh_off'>");
  for(int hh = 0; hh<24;hh++){
    String hhs = (hh < 10 ? "0" : "") + String(hh);
    Page += S("<option value='" + hhs + "' " + (hhs == hh_off ? "selected" : "") +  + ">" + hhs + "</option>");
  }
  Page += F("</select>"
            " : <select name='mm_off' id='mm_off'>"
          );
  for(int mm = 0; mm<60;mm+=15){
    String mms = (mm < 15 ? "0" : "") + String(mm);
    Page += S("<option value='" + mms + "' " + (mms == mm_off ? "selected" : "") +  + ">" + mms + "</option>");
  }
  Page += F("</select>"
            " "
          );

  Page += F(
            "<br /><input type='submit' value='Save'/>"
            "</form>"
            "<p>You may want to <a href='/wifi'>config the wifi connection</a>.</p>"
            "</body></html>");

  web_server.send(200, "text/html", Page);
}
// ==================================================================================================
/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean captivePortal() {
  Serial.println("==== Captive portal verification ...");
  Serial.print("hostHeader: "); Serial.println(web_server.hostHeader());
  if (!isIp(web_server.hostHeader()) && web_server.hostHeader() != (String(myHostname) + ".local")) {
    //Serial.print("myHostname: "); Serial.println(myHostname);
    //Serial.println("Request redirected to captive portal");
    Serial.print("Captive portal location: "); Serial.println(String("http://") + toStringIp(web_server.client().localIP()));
    web_server.sendHeader("Location", String("http://") + toStringIp(web_server.client().localIP()), true);
    web_server.send(302, "text/plain", "");   // Empty content inhibits Content-length header so we have to close the socket ourselves.
    web_server.client().stop(); // Stop is needed because we sent no content length
    Serial.println("==== Captive portal verification returned True");
    return true;
  }
  Serial.println("==== Captive portal verification returned False");
  return false;
}
// ==================================================================================================
/** Wifi config page handler */
void handleWifi() {
  web_server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  web_server.sendHeader("Pragma", "no-cache");
  web_server.sendHeader("Expires", "-1");

  String Page;
  Page += F(
            "<!DOCTYPE html><html lang='en'><head>"
            "<meta name='viewport' content='width=device-width'>"
            "<title>CaptivePortal</title></head><body>"
            "<h1>Wifi config</h1>");
  if (web_server.client().localIP() == apIP) {
    Page += String(F("<p>You are connected through the soft AP: ")) + softAP_ssid + F("</p>");
  } else {
    Page += String(F("<p>You are connected through the wifi network: ")) + ssid + F("</p>");
  }
  
  Page +=
    String(F(
             "\r\n<br />"
             "<table><tr><th align='left'>SoftAP config</th></tr>"
             "<tr><td>SSID ")) +
    softAP_ssid +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.softAPIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"
      "<table><tr><th align='left'>WLAN config</th></tr>"
      "<tr><td>SSID ") +
    ssid +
    F("</td></tr>"
      "<tr><td>IP ") +
    toStringIp(WiFi.localIP()) +
    F("</td></tr>"
      "</table>"
      "\r\n<br />"
      "<table><tr><th align='left'>WLAN list (refresh if any missing)</th></tr>");
  
  Serial.println("scan start");
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n > 0) {
    for (int i = 0; i < n; i++) {
      Page += String(F("\r\n<tr><td>SSID ")) + WiFi.SSID(i) + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? F(" ") : F(" *")) + F(" (") + WiFi.RSSI(i) + F(")</td></tr>");
    }
  } else {
    Page += F("<tr><td>No WLAN found</td></tr>");
  }
  
  Page += F(
            "</table>"
            "\r\n<br /><form method='POST' action='wifisave'><h4>Connect to network:</h4>"
            "<input type='text' placeholder='network' name='n'/>"
            "<br /><input type='password' placeholder='password' name='p'/>"
            "<br /><input type='submit' value='Connect/Disconnect'/></form>"
            "<p>You may want to <a href='/'>return to the home page</a>.</p>"
            "</body></html>");
  
  web_server.send(200, "text/html", Page);
  web_server.client().stop(); // Stop is needed because we sent no content length
}
// ==================================================================================================
/** Handle the WLAN save form and redirect to WLAN config page again */
void handleWifiSave() {
  //Serial.println("wifi save");
  ssid = web_server.arg("n");
  password = web_server.arg("p");
  web_server.sendHeader("Location", "wifi", true);
  web_server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  web_server.sendHeader("Pragma", "no-cache");
  web_server.sendHeader("Expires", "-1");
  web_server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  web_server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  connect = ssid.length() > 0; // Request WLAN connect with new credentials if there is a SSID
}
// ==================================================================================================
/** Handle the WLAN save form and redirect to WLAN config page again */
void handleSettingsSave() {
  
  //for(int a=0; a<web_server.args();a++){
  //  printf("arg=%s value=%s\r\n",web_server.argName(a).c_str(),web_server.arg(a).c_str());
  //}

  for(int i=0; i<5;i++){
    stations[i] = web_server.arg("station" + S(i));
  }
  int last_station =  station;
  station = web_server.arg("station").toInt();
  volume = web_server.arg("volume").toInt();
  
  hh_on = web_server.arg("hh_on");
  mm_on = web_server.arg("mm_on");
  hh_off = web_server.arg("hh_off");
  mm_off = web_server.arg("mm_off");

  for(int i=0;i<5;i++){
    Serial.printf("Station[%d]=%s\r\n",i,stations[i].c_str());
  }
  Serial.printf("station=%d\r\n",station);  
  Serial.printf("volume=%d\r\n",volume);  
  Serial.printf("ON=%s:%s\r\n",hh_on.c_str(),mm_on.c_str());  
  Serial.printf("OFF=%s:%s\r\n",hh_off.c_str(),mm_off.c_str());  

  web_server.sendHeader("Location", "/", true);
  web_server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  web_server.sendHeader("Pragma", "no-cache");
  web_server.sendHeader("Expires", "-1");
  web_server.send(302, "text/plain", "");    // Empty content inhibits Content-length header so we have to close the socket ourselves.
  web_server.client().stop(); // Stop is needed because we sent no content length
  saveCredentials();
  if(last_station != station)
    audio.connecttohost(stations[station].c_str());
  audio.setVolume(volume);
}
// ==================================================================================================
void handleNotFound() {
  if (captivePortal()) { // If captive portal redirect instead of displaying the error page.
    return;
  }
  String message = F("File Not Found\n\n");
  message += F("URI: ");
  message += web_server.uri();
  message += F("\nMethod: ");
  message += (web_server.method() == HTTP_GET) ? "GET" : "POST";
  message += F("\nArguments: ");
  message += web_server.args();
  message += F("\n");

  for (uint8_t i = 0; i < web_server.args(); i++) {
    message += String(F(" ")) + web_server.argName(i) + F(": ") + web_server.arg(i) + F("\n");
  }
  web_server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  web_server.sendHeader("Pragma", "no-cache");
  web_server.sendHeader("Expires", "-1");
  web_server.send(404, "text/plain", message);
}
// ==================================================================================================
