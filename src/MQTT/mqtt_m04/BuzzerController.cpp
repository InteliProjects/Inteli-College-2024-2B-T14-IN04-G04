#include "BuzzerController.h"
#include <Arduino.h>

BuzzerController::BuzzerController(int buzzerPin) : buzzerPin(buzzerPin) {
    pinMode(buzzerPin, OUTPUT);
}

void BuzzerController::beep(int frequency, int duration) {
    tone(buzzerPin, frequency, duration);
}