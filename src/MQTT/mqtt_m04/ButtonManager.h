#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

class ButtonManager {
public:
    ButtonManager(int shutdownPin, int restartPin, LiquidCrystal_I2C& lcd);
    void handleButtons();

private:
    int shutdownPin;
    int restartPin;
    LiquidCrystal_I2C& lcd;

    unsigned long lastButtonPress = 0;
    const unsigned long debounceInterval = 50;

    void shutdownDevice();
    void restartDevice();
};

#endif