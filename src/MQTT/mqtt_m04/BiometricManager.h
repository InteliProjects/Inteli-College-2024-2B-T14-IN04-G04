#ifndef BIOMETRIC_MANAGER_H
#define BIOMETRIC_MANAGER_H

#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>

class BiometricManager {
public:
    BiometricManager(int rxPin, int txPin);
    void initialize();
    bool verifySensor();
    int captureFingerprintID();
    bool enrollFingerprint(String identificador, LiquidCrystal_I2C& lcd);
    bool deleteFingerprint(int id);

private:
    Adafruit_Fingerprint fingerprint;
    int findAvailableID();
    uint8_t performEnrollment(int id);
};

#endif