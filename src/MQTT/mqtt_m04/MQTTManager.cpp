#include "MQTTManager.h"

GerenciadorMQTT::GerenciadorMQTT(const char* servidor, int porta, const char* topicoValidacao, const char* topicoCadastro, LiquidCrystal_I2C* lcd, Adafruit_Fingerprint* fingerprint)
    : servidor(servidor), porta(porta), topicoValidacao(topicoValidacao), topicoCadastro(topicoCadastro), lcd(lcd), fingerprint(fingerprint), clienteMQTT(clienteWiFi) {
    clienteMQTT.setServer(servidor, porta);
}

void GerenciadorMQTT::conectar() {
    while (!clienteMQTT.connected()) {
        if (lcd) {
            lcd->clear();
            lcd->setCursor(0, 0);
            lcd->print("Conectando MQTT...");
        }
        if (clienteMQTT.connect("ESP32Client")) {
            clienteMQTT.subscribe(topicoValidacao);
            clienteMQTT.subscribe(topicoCadastro);
            if (lcd) {
                lcd->clear();
                lcd->setCursor(0, 0);
                lcd->print("MQTT Conectado!");
            }
        } else {
            if (lcd) {
                lcd->clear();
                lcd->setCursor(0, 0);
                lcd->print("Erro MQTT:");
                lcd->setCursor(0, 1);
                lcd->print(String(clienteMQTT.state()));
            }
            delay(5000);
        }
    }
}

void GerenciadorMQTT::loop() {
    if (!clienteMQTT.connected()) {
        conectar();
    }
    clienteMQTT.loop();
}

void GerenciadorMQTT::configurarCallback(std::function<void(const char*, const char*)> callback) {
    clienteMQTT.setCallback([this, callback](char* topic, byte* payload, unsigned int length) {
        String mensagem;
        for (unsigned int i = 0; i < length; i++) {
            mensagem += static_cast<char>(payload[i]);
        }
        if (lcd) {
            lcd->clear();
            lcd->setCursor(0, 0);
            lcd->print("Msg recebida:");
            lcd->setCursor(0, 1);
            lcd->print(mensagem.substring(0, 16)); // Limita a exibição
        }
        if (callback) {
            callback(topic, mensagem.c_str());
        }
    });
}

void GerenciadorMQTT::publicarValidacao(int id) {
    if (!clienteMQTT.connected()) {
        conectar();
    }
    String mensagem = "{\"id\":" + String(id) + "}";
    clienteMQTT.publish(topicoValidacao, mensagem.c_str());
}

void GerenciadorMQTT::publicarErro(const char* mensagem) {
    if (!clienteMQTT.connected()) {
        conectar();
    }
    clienteMQTT.publish("erro/sistema", mensagem);
}

void GerenciadorMQTT::processarMensagem(const char* topico, const char* mensagem) {
    if (String(topico) == topicoCadastro) {
        iniciarCadastro(mensagem);
    } else if (String(topico) == topicoValidacao) {
        validarAcesso(mensagem);
    }
}

void GerenciadorMQTT::iniciarCadastro(const String& id) {
    if (cadastroEmAndamento) {
        publicarErro("Cadastro já em andamento.");
        return;
    }
    cadastroEmAndamento = true;
    if (lcd) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Cadastrando...");
        lcd.setCursor(0, 1);
        lcd.print(id);
    }

    // Implementação personalizada para cadastro
    int newId = id.toInt(); // Converta o ID para inteiro
    int result = fingerprint->getImage(); // Captura a digital
    if (result == FINGERPRINT_OK) {
        // Continue com o cadastro
        clienteMQTT.publish("cadastro/usuario/resposta", "Cadastro realizado com sucesso!");
    } else {
        clienteMQTT.publish("cadastro/usuario/resposta", "Erro ao cadastrar.");
    }
}

void GerenciadorMQTT::validarAcesso(const String& id) {
    if (lcd) {
        lcd->clear();
        lcd->setCursor(0, 0);
        lcd->print("Validando acesso...");
    }

    if (fingerprint && fingerprint->fingerFastSearch() >= 0) {
        clienteMQTT.publish("acesso/resposta", "Acesso permitido.");
    } else {
        clienteMQTT.publish("acesso/resposta", "Acesso negado.");
    }
}