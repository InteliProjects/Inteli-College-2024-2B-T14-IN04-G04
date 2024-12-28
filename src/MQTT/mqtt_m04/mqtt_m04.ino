#include "BiometricManager.h"
#include "BuzzerController.h"
#include "LCDManager.h"
#include "LEDController.h"
#include "RelayController.h"
#include "WiFiManager.h"
#include "MQTTManager.h"
#include <LiquidCrystal_I2C.h>

// Instâncias dos componentes
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiManager wifi("julia lmao", "lika1234", lcd);
GerenciadorMQTT gerenciadorMQTT("broker.hivemq.com", 1883, "instituto/config/validacao", "instituto/config/cadastro", &lcd, &fingerprint);
BiometricManager biometricManager(16, 17);
BuzzerController buzzer(26);
LEDController leds(14, 33, 32);
RelayController relay(18);
Adafruit_Fingerprint fingerprint(&Serial2);

void setup() {
    Serial.begin(115200);
    lcd.init();
    lcd.backlight();

    fingerprint.begin(57600);
    gerenciadorMQTT.conectar();

    // Configurar callback
    gerenciadorMQTT.configurarCallback([&gerenciadorMQTT](const char* topic, const char* message) {
    gerenciadorMQTT.processarMensagem(topic, message);
    });

}

void loop() {
    // Atualiza o estado do relé
    relay.update();

    // Captura o ID da digital
    int fingerprintID = biometricManager.captureFingerprintID();

    if (fingerprintID >= 0) {
        // Digital válida
        String message = "{\"id\":" + String(fingerprintID) + "}";
        Serial.println("Publicando mensagem: " + message);

        // Exibe acesso permitido
        lcd.print("Acesso Permitido");
        buzzer.beep(1000, 200); // Bipe curto
        leds.setGreen();
        relay.activate(5000);   // Ativa o relé por 5 segundos

    } else if (fingerprintID == -1) {
        // Nenhuma digital detectada
        // Não faz nada, aguarda próximo ciclo
    } else if (fingerprintID == -2) {
        // Digital inválida
        lcd.print("Acesso Negado");
        buzzer.beep(500, 1000); // Bipe longo
        leds.setRed();
    }

    // Mensagem padrão ao final do ciclo (se necessário)
    if (!relay.isActive()) {
        leds.reset();                        // Reseta LEDs para estado padrão
        lcd.print("Instituto", "Apontar"); // Mensagem padrão
    }

    // Pequeno atraso para evitar processamento contínuo
    delay(100);
}