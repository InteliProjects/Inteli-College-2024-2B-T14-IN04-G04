#include "ButtonManager.h"
#include <esp_sleep.h>

ButtonManager::ButtonManager(int shutdownPin, int restartPin, LiquidCrystal_I2C& lcd)
    : shutdownPin(shutdownPin), restartPin(restartPin), lcd(lcd) {
    pinMode(shutdownPin, INPUT_PULLUP);
    pinMode(restartPin, INPUT_PULLUP);
}

void ButtonManager::handleButtons() {
    if (digitalRead(shutdownPin) == LOW && millis() - lastButtonPress > debounceInterval) {
        lastButtonPress = millis();
        shutdownDevice();
    }

    if (digitalRead(restartPin) == LOW && millis() - lastButtonPress > debounceInterval) {
        lastButtonPress = millis();
        restartDevice();
    }
}

void ButtonManager::shutdownDevice() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Desligando...");
    delay(1000);

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_25, 0); // Configura wake-up por GPIO
    esp_deep_sleep_start();
}

void ButtonManager::restartDevice() {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Reiniciando...");
    delay(1000);

    esp_restart(); // Reinicia o dispositivo
}
