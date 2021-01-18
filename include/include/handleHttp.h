#include <arduino.h>


void handleRoot(); 


/** Redirect to captive portal if we got a request for another domain. Return true in that case so the page handler do not try to handle the request again. */
boolean captivePortal();

/** Wifi config page handler */
void handleWifi();


/** Handle the WLAN save form and redirect to WLAN config page again */
void handleWifiSave();

boolean captivePortal();

void handleHygroTempPower();
void handleHygrostat();

void handleNotFound();

extern void saveCredentials();
extern void loadCredentials();

