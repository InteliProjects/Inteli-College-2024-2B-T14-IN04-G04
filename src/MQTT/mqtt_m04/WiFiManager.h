#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "LCDManager.h"

class WiFiManager {
public:
    WiFiManager(const char* ssid, const char* password, LCDManager& lcdManager);
    void connect();

private:
    const char* ssid;
    const char* password;
    LCDManager& lcd;
};

#endif