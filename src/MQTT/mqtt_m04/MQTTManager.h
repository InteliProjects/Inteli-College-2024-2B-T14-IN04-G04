#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#include <WiFi.h>
#include <PubSubClient.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <functional>

class GerenciadorMQTT {
public:
    GerenciadorMQTT(const char* servidor, int porta, const char* topicoValidacao, const char* topicoCadastro, LiquidCrystal_I2C* lcd, Adafruit_Fingerprint* fingerprint);
    void conectar();
    void loop();
    void configurarCallback(std::function<void(const char*, const char*)> callback);
    void publicarValidacao(int id);
    void publicarErro(const char* mensagem);

private:
    void processarMensagem(const char* topico, const char* mensagem);
    void iniciarCadastro(const String& id);
    void validarAcesso(const String& id);

    const char* servidor;
    int porta;
    const char* topicoValidacao;
    const char* topicoCadastro;

    WiFiClient clienteWiFi;
    PubSubClient clienteMQTT;
    LiquidCrystal_I2C* lcd;
    Adafruit_Fingerprint* fingerprint;

    bool cadastroEmAndamento = false;
};

#endif