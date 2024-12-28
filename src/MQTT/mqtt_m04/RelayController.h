#ifndef RELAY_CONTROLLER_H
#define RELAY_CONTROLLER_H

#include <Arduino.h>

class RelayController {
public:
    RelayController(int relayPin);
    void activate(unsigned long duration = 0); // Ativa o relé com uma duração opcional
    void update();                             // Gerencia o estado do relé
    bool isActive();                           // Retorna se o relé está ativo

private:
    int relayPin;
    bool relayActive;
    unsigned long activationTime;
    unsigned long duration;
};

#endif