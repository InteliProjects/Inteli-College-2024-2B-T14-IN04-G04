#ifndef LCD_MANAGER_H
#define LCD_MANAGER_H

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

class LCDManager {
public:
    LCDManager(uint8_t address, int cols, int rows);
    void initialize();
    void displayMessage(const String& line1, const String& line2 = "");
    void clear();

private:
    LiquidCrystal_I2C lcd;
};

#endif