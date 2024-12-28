#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h> // Biblioteca para o leitor biométrico

// Definindo os pinos do leitor biométrico
#define BIOMETRIC_RX_PIN 16
#define BIOMETRIC_TX_PIN 17

// Definições dos pinos para LEDs, buzzer e relé
#define LED_VERDE 14
#define LED_AZUL 33
#define LED_VERMELHO 32
#define BUZZER_PIN 26
#define RELE_PIN 18

// Variáveis para controle de temporização
unsigned long tempoLedVerde = 0;
unsigned long tempoLedVermelho = 0;
unsigned long duracaoLedVerde = 2000;
unsigned long duracaoLedVermelho = 2000;
unsigned long tempoAtivacaoRele = 2;
unsigned long tempoWiFi = 0;
unsigned long tempoReconnect = 0;
unsigned long tempoLCD = 0;
unsigned long tempoCadastro = 0;
unsigned long tempoDigital = 0;
unsigned long duracaoDelay = 0;
bool ledVerdeAceso = false;
bool ledVermelhoAceso = false;
bool releAtivo = false;
bool emDelay = false;

// Configurações Wi-Fi
const char* ssid = "julia lmao";
const char* password = "lika1234";

// Configurações MQTT
const int mqtt_port = 1883;
const char* mqtt_server = "broker.hivemq.com";
// const char* mqtt_user = "Apontados";
// const char* mqtt_pass = "Apontados123";
const char* mqtt_topic_validation = "instituto/biometria/acesso";


static unsigned long lastPublish = 0;
String message = "";

// Configuração do display LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Configuração SSL para HiveMQ Cloud
WiFiClient espClient; 
PubSubClient client(espClient);

// Inicializando o leitor biométrico no Serial2
Adafruit_Fingerprint leitorBiometrico = Adafruit_Fingerprint(&Serial2);

void setupWiFi() {
  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi");
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - startAttemptTime > 15000) { // Timeout de 15 segundos
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Falha WiFi");
      return; // Sai da função se não conectar
    }
    if (millis() - tempoWiFi >= 1000) {
      tempoWiFi = millis();
      lcd.setCursor(0, 1);
      lcd.print(".");
    }
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi conectado");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP().toString().c_str());
  iniciarDelay(2000);
}

void callback(char* topic, byte* payload, unsigned int length) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Msg recebida:");
  lcd.setCursor(0, 1);

  // Exibe a mensagem recebida no LCD (até 16 caracteres)
  for (int i = 0; i < length && i < 16; i++) {
    lcd.print((char)payload[i]);
  }

  // Passa os dados para a função tratarMensagemConfig
  tratarMensagemConfig(topic, payload, length);
}

void reconnect() {
  while (!client.connected()) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Conectando MQTT");
    if (client.connect("clientId-NiczoLeBwO")) {
      lcd.setCursor(0, 1);
      lcd.print("MQTT Conectado!");
      client.subscribe("instituto/config/cadastro");
      client.subscribe("instituto/config/remocao");
    } else {
      lcd.setCursor(0, 1);
        lcd.print("Falha:");
        lcd.print(client.state());
    }
  }
}

void setup() {
    Serial.begin(115200);
    lcd.init();
    lcd.backlight();
    lcd.print("Iniciando...");
    iniciarDelay(2000); // Delay inicial
    setupWiFi(); // Conecta ao Wi-Fi
    client.setServer(mqtt_server, mqtt_port); // Configura o servidor MQTT
    client.setCallback(callback); // Callback para mensagens MQTT recebidas
    inicializarSerial(); // Inicializa Serial e leitor biométrico
    configurarPinos(); // Configura pinos como saída
    verificarLeitorBiometrico(); // Testa a comunicação com o leitor
}

void loop() {
  gerenciarDelay(); // Chama para verificar atrasos pendentes
  if (!client.connected()) {
    reconnect(); // Reestabelece conexão MQTT, se necessário
  }
  client.loop(); // Mantém a comunicação MQTT ativa

  int resultadoLeitura = capturarIdDigital();  // Tenta identificar digital

  // Controle de ativação do relé
  static bool releAtivado = false;  // Armazena se o relé está ativo
  static unsigned long tempoReleAtivado = 0; // Tempo de ativação do relé

  if (resultadoLeitura >= 0) { // Digital válida encontrada
      Serial.println(resultadoLeitura);
      String mensagem = "{\"id\":" + String(resultadoLeitura) + "}";
      client.publish(mqtt_topic_validation, mensagem.c_str()); // Publicação no tópico MQTT
      exibirAcessoPermitido();
      acenderLedVerde();
      
      // Ativa o relé e armazena o tempo de ativação
      digitalWrite(RELE_PIN, LOW);
      releAtivado = true;
      tempoReleAtivado = millis();
  } else if (resultadoLeitura == -2) { // Digital não encontrada
      exibirAcessoNegado();
      acenderLedVermelho();
  }

  // Verifica se o relé está ativo e controla a duração de 5 segundos
  if (releAtivado && millis() - tempoReleAtivado >= 5000) {
      digitalWrite(RELE_PIN, HIGH); // Desativa o relé
      releAtivado = false;          // Atualiza o estado do relé
      Serial.println("Relé desativado após 5 segundos.");
  }

  // Gerenciamento de temporização dos LEDs
  gerenciarLedVerde();
  gerenciarLedVermelho();
}

// Função para exibir mensagem de acesso permitido e acionar feedback visual e sonoro
void exibirAcessoPermitido() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Acesso Permitido");
  tone(BUZZER_PIN, 1000, 200); // Emite tom curto no buzzer
}

// Função para exibir mensagem de acesso negado e acionar feedback visual e sonoro
void exibirAcessoNegado() {
  lcd.clear();
  lcd.setCursor(0, 0);
  tone(BUZZER_PIN, 500, 1000); // Emite tom longo no buzzer
  lcd.print("Acesso negado");
  iniciarDelay(1000); // Substituição do delay por lógica millis
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Tente Novamente!");
}

// Função para ativar o LED verde
void acenderLedVerde() {
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_AZUL, LOW);
  ledVerdeAceso = true;
  tempoLedVerde = millis();
}

// Função para ativar o LED vermelho
void acenderLedVermelho() {
  digitalWrite(LED_VERMELHO, HIGH);
  digitalWrite(LED_AZUL, LOW);
  ledVermelhoAceso = true;
  tempoLedVermelho = millis();
}

// Função para ativar o relé
void ativarRele() {
  digitalWrite(RELE_PIN, LOW);
  releAtivo = true;
  tempoAtivacaoRele = millis();
}

// Função para desativar o relé após o tempo especificado
void desativarRele() {
  if (releAtivo && millis() - tempoAtivacaoRele >= 5000) {
    digitalWrite(RELE_PIN, HIGH);
    releAtivo = false;
  }
}

// Função para gerenciar temporização do LED verde
void gerenciarLedVerde() {
  if (ledVerdeAceso && millis() - tempoLedVerde >= duracaoLedVerde) {
    digitalWrite(LED_VERDE, LOW);
    ledVerdeAceso = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Instituto");
    lcd.setCursor(0, 1);
    lcd.print("Apontar");
    digitalWrite(LED_AZUL, HIGH); // Volta ao estado de espera
  }
}

// Função para gerenciar temporização do LED vermelho
void gerenciarLedVermelho() {
  if (ledVermelhoAceso && millis() - tempoLedVermelho >= duracaoLedVermelho) {
    digitalWrite(LED_VERMELHO, LOW);
    ledVermelhoAceso = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Instituto");
    lcd.setCursor(0, 1);
    lcd.print("Apontar");
    digitalWrite(LED_AZUL, HIGH); // Volta ao estado de espera
  }
}

void tratarMensagemConfig(char* topic, byte* payload, unsigned int length) {
  String mensagemRecebida = "";
  for (unsigned int i = 0; i < length; i++) {
    mensagemRecebida += (char)payload[i];
  }
  
  if (String(topic) == "instituto/config/horarios") {
    Serial.println("Configuração de horários recebida: " + mensagemRecebida);
  } 
  else if (String(topic) == "instituto/config/cadastro") {
    Serial.println("Cadastro recebido: " + mensagemRecebida);
    cadastrarDigital(mensagemRecebida);
    //capturarIdDigital();
  } 
  else if (String(topic) == "instituto/config/remocao") {
    Serial.println("Remoção recebida: " + mensagemRecebida);
    removerDigital(mensagemRecebida);
  }
}

// Cadastra uma nova digital associada a um ID disponível.
void cadastrarDigital(String identificador) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cadastrar:");
  lcd.setCursor(0, 1);
  lcd.print(identificador);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Cadastrar:");

  uint8_t p = -1;

  // Captura a digital atual
  while (p != FINGERPRINT_OK) {
    p = leitorBiometrico.getImage();
    if (p == FINGERPRINT_NOFINGER) {
      iniciarDelay(100); // Substituição do delay por lógica millis // Aguarda o dedo ser colocado
      continue;
    } else if (p == FINGERPRINT_OK) {
      p = leitorBiometrico.image2Tz();
      if (p != FINGERPRINT_OK) {
        lcd.setCursor(0, 1);
        lcd.print("Erro na leitura");
        iniciarDelay(2000); // Substituição do delay por lógica millis
        return;
      }
      break;
    }
  }

  // Verifica se a digital já está cadastrada
  p = leitorBiometrico.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Digital já cadastrada");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dedo ja reg.");
    iniciarDelay(2000); // Substituição do delay por lógica millis
    client.publish("instituto/resposta/cadastro", "Erro: Digital já cadastrada.");
    return;
  }

  // Encontrar o primeiro ID disponível dinamicamente
  int id = findAvailableID();
  if (id == -1) {
    client.publish("instituto/resposta/cadastro", "Erro: Memória cheia.");
    return;
  }

  uint8_t resultado = getFingerprintEnroll(id); // Passando o ID encontrado para a função

  if (resultado == FINGERPRINT_OK) {
    client.publish("instituto/resposta/cadastro", "Cadastro realizado com sucesso!");

    // Exibe o nome e ID da pessoa no LCD por 2 segundos
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Nome: " + identificador);
    lcd.setCursor(0, 1);
    lcd.print("ID: " + String(id));
    iniciarDelay(2000); // Substituição do delay por lógica millis

    // Exibe "Cadastrado com sucesso" por 2 segundos
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Cadastrado com");
    lcd.setCursor(0, 1);
    lcd.print("sucesso!");
    iniciarDelay(2000); // Substituição do delay por lógica millis

  } else {
    client.publish("instituto/resposta/cadastro", "Erro: Falha ao salvar.");
  }
}

int findAvailableID() {
  for (int i = 1; i < 200; i++) { // Ajuste o limite máximo de IDs conforme o sensor
    if (leitorBiometrico.loadModel(i) != FINGERPRINT_OK) {
      return i; // Retorna o primeiro ID livre
    }
  }
  return -1; // Retorna -1 se não houver IDs disponíveis
}

uint8_t getFingerprintEnroll(int id) {
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #");
  Serial.println(id);

  lcd.setCursor(0, 1);
  lcd.println("Coloque o Dedo");

  while (p != FINGERPRINT_OK) {
    p = leitorBiometrico.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        Serial.println("Image taken");
        break;
      case FINGERPRINT_NOFINGER:
        Serial.print(".");
        break;
      case FINGERPRINT_PACKETRECIEVEERR:
        Serial.println("Communication error");
        break;
      case FINGERPRINT_IMAGEFAIL:
        Serial.println("Imaging error");
        break;
      default:
        Serial.println("Unknown error");
        break;
    }
  }

  // OK success!
  p = leitorBiometrico.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }


  lcd.setCursor(0, 1);
  lcd.println("Remova o dedo");

  Serial.println("Remove finger");
  iniciarDelay(2000); // Substituição do delay por lógica millis
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = leitorBiometrico.getImage();
  }
  Serial.print("ID ");
  Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");

  lcd.setCursor(0, 1);
  lcd.println("Coloque o Dedo");

  while (p != FINGERPRINT_OK) {
    p = leitorBiometrico.getImage();
  }

  leitorBiometrico.image2Tz(2);

  // OK converted!
  Serial.print("Creating model for #");
  Serial.println(id);

  lcd.setCursor(0, 1);
  lcd.println("Dedo Cadastrado");

  leitorBiometrico.createModel();

  Serial.print("ID ");
  Serial.println(id);
  p = leitorBiometrico.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
  } else {
    lcd.setCursor(0, 1);
    lcd.println("Erro ao salvar");
  }

  return FINGERPRINT_OK;
}


void removerDigital(String identificador) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Removendo ID:");
  lcd.setCursor(0, 1);
  lcd.print(identificador);

  int resultado = leitorBiometrico.deleteModel(atoi(identificador.c_str()));
  if (resultado == FINGERPRINT_OK) {      
    client.publish("instituto/resposta/remocao", "Remoção realizada com sucesso!");
  } else {
    client.publish("instituto/resposta/remocao", "Erro: Falha ao remover.");
  }
}

// Função para verificar se o leitor biométrico está conectado e funcional
void verificarLeitorBiometrico() {
  if (!leitorBiometrico.verifyPassword()) {
    Serial.println("Erro: Não foi possível se comunicar com o leitor biométrico.");
    lcd.setCursor(0, 0);
    lcd.print("Erro no sensor!");
    while (true) { // Pisca o LED azul indefinidamente para indicar erro
      digitalWrite(LED_AZUL, bitRead(millis(), 0));
    }
  }

  leitorBiometrico.getTemplateCount();
  if (leitorBiometrico.templateCount > 0) {
    Serial.print("Digitais registradas: ");
    Serial.println(leitorBiometrico.templateCount);
    lcd.setCursor(0, 0);
    lcd.print("Digitais:");
    lcd.setCursor(0, 1);
    lcd.print(leitorBiometrico.templateCount);
    iniciarDelay(2000); // Substituição do delay por lógica millis
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Instituto");
    lcd.setCursor(0, 1);
    lcd.print("Apontar");
  } else {
    Serial.println("Erro: Nenhuma digital registrada.");
    lcd.setCursor(0, 0);
    lcd.print("Sem digitais!");
    while (true) {
      digitalWrite(LED_AZUL, bitRead(millis(), 5));
    }
  }
}

// Função para configurar pinos de saída e estado inicial
void configurarPinos() {
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AZUL, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELE_PIN, OUTPUT);
  
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(LED_AZUL, HIGH); // LED azul indica sistema ativo
  digitalWrite(RELE_PIN, HIGH); // Relé desativado no estado inicial
}

// Função para capturar a digital e identificar o ID
int capturarIdDigital() {
  uint8_t p = leitorBiometrico.getImage();
  if (p == FINGERPRINT_NOFINGER) return -1;
  else if (p != FINGERPRINT_OK) {
    Serial.println("Erro ao capturar imagem");
    return -1;
  }

  p = leitorBiometrico.image2Tz();
  if (p != FINGERPRINT_OK) {
    Serial.println("Erro ao converter imagem");
    return -1;
  }

  p = leitorBiometrico.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    Serial.print("ID encontrado: ");
    Serial.println(leitorBiometrico.fingerID);
    return leitorBiometrico.fingerID;
  } else {
    Serial.println("Digital nao encontrada");
    return -2;
  }
}

// Função para inicializar a comunicação Serial com ESP32 e leitor biométrico
void inicializarSerial() {
  Serial.begin(115200);
  Serial2.begin(57600, SERIAL_8N1, BIOMETRIC_RX_PIN, BIOMETRIC_TX_PIN);
  leitorBiometrico.begin(57600);
}

// Função para substituir delays com lógica baseada em millis
void iniciarDelay(unsigned long duracao) {
  duracaoDelay = duracao;
  emDelay = true;
  tempoCadastro = millis();
}

void gerenciarDelay() {
  if (emDelay && millis() - tempoCadastro >= duracaoDelay) {
    emDelay = false;
  }
}