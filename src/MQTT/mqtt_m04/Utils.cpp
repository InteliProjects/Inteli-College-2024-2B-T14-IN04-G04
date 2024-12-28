#include "Utils.h"
#include <Arduino.h>

// Variáveis para gerenciar delays
static unsigned long tempoInicio = 0;
static unsigned long duracao = 0;
static bool emDelay = false;

void iniciarDelay(unsigned long tempo) {
    tempoInicio = millis();
    duracao = tempo;
    emDelay = true;
}

void gerenciarDelay() {
    if (emDelay && (millis() - tempoInicio >= duracao)) {
        emDelay = false;
    }
}

bool delayAtivo() {
    return emDelay;
}
