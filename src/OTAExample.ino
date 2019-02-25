/*
 * Project Particle Electron OTA Update Without Cloud
 * Description: Download and flash user firmware using TCPClient to OTA update Particle Electron.
 * Author: Franc Muñoz - ZeroWorks!
 * Date: 25/02/2019
 */

#include "ota.h"

// Update or remove if needed...
STARTUP(cellular_credentials_set("orangeworld", "", "", NULL));

// We only need internet connection...
SYSTEM_MODE(SEMI_AUTOMATIC);

void setup() {
    Serial.begin();
    delay(2000);
    Serial.println("Device OTA Update v.0.1");
    Cellular.on();
    Cellular.connect();
}

int ms=0;
bool once=true;

void loop() {
    // Wait 5 seconds... don't needed at all... but gave me some time to stop particle while debugging
    if(millis()-ms>5000 && once) {
        if(Cellular.ready()) {
            once=false;
            downloadFirmware();
        } else {
            ms=millis();
        }
    }
}
