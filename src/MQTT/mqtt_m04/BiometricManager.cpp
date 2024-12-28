#include "BiometricManager.h"

BiometricManager::BiometricManager(int rxPin, int txPin) 
    : fingerprint(&Serial2) {
    Serial2.begin(57600, SERIAL_8N1, rxPin, txPin);
}

void BiometricManager::initialize() {
    fingerprint.begin(57600);
}

bool BiometricManager::verifySensor() {
    return fingerprint.verifyPassword();
}

int BiometricManager::captureFingerprintID() {
    if (fingerprint.getImage() != FINGERPRINT_OK) return -1;
    if (fingerprint.image2Tz() != FINGERPRINT_OK) return -1;
    if (fingerprint.fingerFastSearch() == FINGERPRINT_OK) {
        return fingerprint.fingerID;
    }
    return -1;
}

bool BiometricManager::enrollFingerprint(String identificador, LiquidCrystal_I2C& lcd) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cadastrando:");
    lcd.setCursor(0, 1);
    lcd.print(identificador);

    int id = findAvailableID();
    if (id == -1) {
        lcd.clear();
        lcd.print("Memória cheia");
        return false;
    }

    uint8_t result = performEnrollment(id);
    if (result == FINGERPRINT_OK) {
        lcd.clear();
        lcd.print("Cadastro sucesso!");
        lcd.setCursor(0, 1);
        lcd.print("ID: " + String(id));
        return true;
    } else {
        lcd.clear();
        lcd.print("Erro no cadastro");
        return false;
    }
}

bool BiometricManager::deleteFingerprint(int id) {
    return fingerprint.deleteModel(id) == FINGERPRINT_OK;
}

int BiometricManager::findAvailableID() {
    for (int i = 1; i < 200; i++) {
        if (fingerprint.loadModel(i) != FINGERPRINT_OK) {
            return i;
        }
    }
    return -1;
}

uint8_t BiometricManager::performEnrollment(int id) {
    int p = -1;
    Serial.println("Iniciando processo de cadastro");

    while (p != FINGERPRINT_OK) {
        p = fingerprint.getImage();
        if (p == FINGERPRINT_NOFINGER) {
            delay(100);
            continue;
        }
        if (p != FINGERPRINT_OK) return p;
    }

    p = fingerprint.image2Tz(1);
    if (p != FINGERPRINT_OK) return p;

    Serial.println("Remova o dedo");
    delay(2000);

    while ((p = fingerprint.getImage()) != FINGERPRINT_NOFINGER);

    Serial.println("Coloque o mesmo dedo novamente");
    while (p != FINGERPRINT_OK) {
        p = fingerprint.getImage();
        if (p != FINGERPRINT_OK) continue;
    }

    p = fingerprint.image2Tz(2);
    if (p != FINGERPRINT_OK) return p;

    p = fingerprint.createModel();
    if (p != FINGERPRINT_OK) return p;

    p = fingerprint.storeModel(id);
    return p;
}