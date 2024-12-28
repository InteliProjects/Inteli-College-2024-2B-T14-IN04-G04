#include "LCDManager.h"

LCDManager::LCDManager(uint8_t address, int cols, int rows) 
    : lcd(address, cols, rows) {}

void LCDManager::initialize() {
    lcd.init();
    lcd.backlight();
}

void LCDManager::displayMessage(const String& line1, const String& line2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    if (!line2.isEmpty()) {
        lcd.setCursor(0, 1);
        lcd.print(line2);
    }
}

void LCDManager::clear() {
    lcd.clear();
}