#include "RelayController.h"

RelayController::RelayController(int relayPin) 
    : relayPin(relayPin), relayActive(false), activationTime(0), duration(0) {
    pinMode(relayPin, OUTPUT);
    digitalWrite(relayPin, HIGH); // Relé desativado no estado inicial
}

void RelayController::activate(unsigned long duration) {
    digitalWrite(relayPin, LOW); // Ativa o relé
    relayActive = true;
    this->duration = duration;
    activationTime = millis();
}

void RelayController::update() {
    if (relayActive && duration > 0 && millis() - activationTime >= duration) {
        digitalWrite(relayPin, HIGH); // Desativa o relé
        relayActive = false;
        duration = 0;
    }
}

bool RelayController::isActive() {
    return relayActive;
}