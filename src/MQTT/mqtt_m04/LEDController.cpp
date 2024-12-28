#include "LEDController.h"
#include <Arduino.h>

LEDController::LEDController(int greenPin, int bluePin, int redPin)
    : greenPin(greenPin), bluePin(bluePin), redPin(redPin) {
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    pinMode(redPin, OUTPUT);
}

void LEDController::setGreen() {
    digitalWrite(greenPin, HIGH);
    digitalWrite(bluePin, LOW);
    digitalWrite(redPin, LOW);
}

void LEDController::setRed() {
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, LOW);
    digitalWrite(redPin, HIGH);
}

void LEDController::reset() {
    digitalWrite(greenPin, LOW);
    digitalWrite(bluePin, HIGH);
    digitalWrite(redPin, LOW);
}