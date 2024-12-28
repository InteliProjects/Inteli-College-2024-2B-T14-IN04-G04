#include "WiFiManager.h"

WiFiManager::WiFiManager(const char* ssid, const char* password, LCDManager& lcdManager)
    : ssid(ssid), password(password), lcd(lcdManager) {}

void WiFiManager::connect() {
    lcd.displayMessage("Connecting to", ssid);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        lcd.displayMessage("Connecting...");
    }

    lcd.displayMessage("WiFi connected", WiFi.localIP().toString());
}