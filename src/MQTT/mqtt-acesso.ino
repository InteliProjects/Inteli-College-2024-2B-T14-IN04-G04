#include <WiFi.h> // Inclui a biblioteca para lidar com conexões Wi-Fi.
#include <WiFiClientSecure.h> // Inclui a biblioteca para conexões seguras via Wi-Fi (TLS/SSL).
#include <PubSubClient.h> // Inclui a biblioteca para trabalhar com o protocolo MQTT.
#include <Wire.h> // Inclui a biblioteca para comunicação I2C.
#include <LiquidCrystal_I2C.h> // Inclui a biblioteca para controlar displays LCD via I2C.
#include <Adafruit_Fingerprint.h> // Inclui a biblioteca para o sensor biométrico.
#include <ArduinoJson.h> // Inclua a biblioteca ArduinoJson para criar mensagens JSON.
#include <map> // Biblioteca para criar mapas.

std::map<int, unsigned long> registros; // Mapeia o ID da pessoa para o horário de entrada.

// Configuração dos pinos do leitor biométrico e outros periféricos
#define BIOMETRIC_RX_PIN 16 // Define o pino RX do sensor biométrico.
#define BIOMETRIC_TX_PIN 17 // Define o pino TX do sensor biométrico.
#define LED_VERDE 25 // Define o pino do LED verde.
#define LED_VERMELHO 32 // Define o pino do LED vermelho.
#define LED_AZUL 33 // Define o pino do LED azul.
#define BUZZER_PIN 26 // Define o pino do buzzer.
#define RELE_PIN 18 // Define o pino do relé.

// Configuração do Wi-Fi
const char* ssid = "Apontados"; // SSID da rede Wi-Fi.
const char* password = "Apontados123"; // Senha da rede Wi-Fi.

// Configuração do MQTT
const int mqtt_port = 8883; // Porta segura para conexão MQTT.
const char* mqtt_server = "55b8f29d49124c61b0c0c1a44faf306b.s1.eu.hivemq.cloud"; // Endereço do broker MQTT.
const char* mqtt_topic_acesso = "instituto/acesso"; // Tópico MQTT para mensagens de acesso.
const char* mqtt_user = "julialika"; // Usuário para autenticação no MQTT.
const char* mqtt_pass = "1234Inteli"; // Senha para autenticação no MQTT.

struct Pessoa {
    int id;               // ID da pessoa.
    String tipo;          // Tipo: "Aluno" ou "Funcionário".
    unsigned long entrada; // Horário de entrada (em milissegundos).
    unsigned long saida;   // Horário de saída (em milissegundos).
};

String obterTipoPessoa(int id) {
    if (id >= 1 && id <= 100) {
        return "Funcionario"; // IDs de 1 a 100 são funcionários.
    } else if (id > 100 && id <= 900) {
        return "Aluno"; // IDs de 101 a 900 são alunos.
    } else {
        return "Desconhecido"; // Caso não esteja mapeado.
    }
}

bool jaEntrou(int id) {
    return registros.find(id) != registros.end(); // Retorna true se o ID já existir no mapa.
}

// Objetos e variáveis globais
WiFiClientSecure espClient; // Cliente seguro para comunicação Wi-Fi.
PubSubClient client(espClient); // Objeto para lidar com conexões MQTT.
LiquidCrystal_I2C lcd(0x27, 16, 2); // Inicializa o display LCD com endereço I2C e dimensões.
Adafruit_Fingerprint leitorBiometrico(&Serial2); // Inicializa o leitor biométrico no Serial2.
unsigned long ultimaLeitura = 0; // Tempo da última leitura.
unsigned long lastPublish = 0; // Tempo da última publicação MQTT.
bool acessoConcedido = false; // Indica se o acesso foi concedido.
Pessoa pessoaAtual; // Armazena os dados da pessoa que está entrando.

// Função para configurar o Wi-Fi
void setupWiFi() {
  delay(10);
  lcd.setCursor(0, 0);
  lcd.print("Conectando WiFi"); // Mostra mensagem no LCD.
  WiFi.begin(ssid, password); // Inicia a conexão Wi-Fi.

  while (WiFi.status() != WL_CONNECTED) { // Aguarda conexão Wi-Fi.
    delay(1000);
    lcd.setCursor(0, 1);
    lcd.print("..."); // Indica progresso.
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi conectado!"); // Confirma conexão.
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP().toString().c_str()); // Mostra IP obtido.
  delay(2000);
}

// Configuração do sensor biométrico
void verificarLeitorBiometrico() {
  if (leitorBiometrico.verifyPassword()) { // Verifica se o leitor está funcionando.
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Biometrico OK!"); // Confirma operação.
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Erro Biom."); // Informa falha.
    while (true); // Para o sistema em caso de erro crítico.
  }
}

// Captura de ID biométrico
int capturarIdDigital() {
  int p = leitorBiometrico.getImage(); // Captura a imagem da digital.
  if (p != FINGERPRINT_OK) return -1; // Verifica erro na captura.
  p = leitorBiometrico.image2Tz(1); // Converte a imagem para template.
  if (p != FINGERPRINT_OK) return -1; // Verifica erro na conversão.
  p = leitorBiometrico.fingerSearch(); // Busca ID no banco de dados.
  if (p == FINGERPRINT_OK) {
    return leitorBiometrico.fingerID; // Retorna ID se encontrado.
  } else {
    return -2; // Retorna erro se não encontrado.
  }
}

void registrarEntrada(int id) {
    registros[id] = millis(); // Registra o horário de entrada no mapa.
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Entrada Registrada");
    lcd.setCursor(0, 1);
    lcd.print("ID: " + String(id));
}

// Acesso permitido
void acessoPermitido(int id) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Acesso Permitido"); // Exibe mensagem no LCD.
  lcd.setCursor(0, 1);
  lcd.print("ID: " + String(id)); // Mostra o ID permitido.
  digitalWrite(LED_VERDE, HIGH); // Liga o LED verde.
  digitalWrite(RELE_PIN, LOW); // Aciona o relé.
  tone(BUZZER_PIN, 1000, 200); // Toca som no buzzer.
  client.publish(mqtt_topic_acesso, ("Acesso Concedido: ID " + String(id)).c_str()); // Envia mensagem MQTT.
  delay(3000); // Aguarda 3 segundos.
}

void registrarSaida(int id) {
    unsigned long horarioEntrada = registros[id]; // Recupera o horário de entrada.
    unsigned long horarioSaida = millis(); // Captura o horário de saída.
    unsigned long tempoPermanencia = (horarioSaida - horarioEntrada) / 1000; // Calcula o tempo em segundos.

    // Prepara os dados para publicação.
    Pessoa pessoa;
    pessoa.id = id;
    pessoa.tipo = obterTipoPessoa(id);
    pessoa.entrada = horarioEntrada;
    pessoa.saida = horarioSaida;


    // Publica os dados no MQTT.
    publicarAcesso(pessoa);


    // Remove o ID do mapa.
    registros.erase(id);


    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Saida Registrada");
    lcd.setCursor(0, 1);
    lcd.print("ID: " + String(id));
}


void finalizarAcesso() {
    pessoaAtual.saida = millis(); // Registra o horário de saída.


    // Publica os dados no MQTT.
    publicarAcesso(pessoaAtual);


    // Reseta o estado do sistema.
    resetarEstado();
}


// Acesso negado
void acessoNegado() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Acesso Negado"); // Mostra mensagem no LCD.
  digitalWrite(LED_VERMELHO, HIGH); // Liga o LED vermelho.
  tone(BUZZER_PIN, 500, 500); // Toca som de erro.
  client.publish(mqtt_topic_acesso, "Acesso Negado"); // Envia mensagem MQTT.
  delay(3000); // Aguarda 3 segundos.
}


// Reset do estado inicial do sistema
void resetarEstado() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pronto"); // Indica que está pronto para uso.
  digitalWrite(LED_VERDE, LOW);
  digitalWrite(LED_VERMELHO, LOW);
  digitalWrite(RELE_PIN, HIGH); // Desativa o relé.
  acessoConcedido = false; // Reseta o acesso concedido.
}


// Reconexão ao MQTT
void reconnectMQTT() {
  while (!client.connected()) { // Tenta reconectar enquanto desconectado.
    lcd.setCursor(0, 0);
    lcd.print("Conectando MQTT");
    if (client.connect("ESP32_Client", mqtt_user, mqtt_pass)) { // Conexão com credenciais.
      lcd.setCursor(0, 1);
      lcd.print("MQTT Conectado!"); // Confirma conexão.
      client.subscribe(mqtt_topic_acesso); // Inscreve no tópico.
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Falha:");
      lcd.print(client.state()); // Mostra erro no LCD.
      delay(2000);
    }
  }
}


void publicarAcesso(Pessoa pessoa) {
    StaticJsonDocument<256> doc; // Cria um documento JSON.


    // Converte os timestamps para horas legíveis (em segundos).
    unsigned long tempoPermanencia = (pessoa.saida - pessoa.entrada) / 1000;
    doc["id"] = pessoa.id;
    doc["tipo"] = pessoa.tipo;
    doc["entrada"] = String(pessoa.entrada / 1000); // Converte entrada para segundos.
    doc["saida"] = String(pessoa.saida / 1000); // Converte saída para segundos.
    doc["tempo_permanencia"] = String(tempoPermanencia) + " segundos";


    // Serializa o JSON para um buffer de string.
    String jsonString;
    serializeJson(doc, jsonString);


    // Publica o JSON no tópico MQTT.
    client.publish(mqtt_topic_acesso, jsonString.c_str());
}




void callback(char* topic, byte* payload, unsigned int length) {
    // Converte o payload (byte array) para uma string.
    String message = "";
    for (int i = 0; i < length; i++) {
        message += (char)payload[i];
    }


    // Exibe o tópico e a mensagem recebida no Serial Monitor.
    Serial.print("Mensagem recebida no tópico: ");
    Serial.println(topic);
    Serial.print("Mensagem: ");
    Serial.println(message);


    // Processa ações com base no conteúdo da mensagem.
    if (String(topic) == mqtt_topic_acesso) {
        if (message == "RESET") {
            resetarEstado(); // Reseta o sistema.
        } else if (message.startsWith("DISPLAY:")) {
            String text = message.substring(8); // Pega o texto após "DISPLAY:"
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print(text); // Exibe no LCD.
        } else if (message == "ACENDER_LED") {
            digitalWrite(LED_VERDE, HIGH); // Acende o LED verde.
        }
    }
}




// Configuração inicial do sistema
void setup() {
  Serial.begin(115200); // Inicializa comunicação serial.
  Serial2.begin(57600, SERIAL_8N1, BIOMETRIC_RX_PIN, BIOMETRIC_TX_PIN); // Configura Serial2 para o leitor biométrico.
  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_VERMELHO, OUTPUT);
  pinMode(RELE_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(RELE_PIN, HIGH); // Inicializa o relé desligado.


  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Inicializando..."); // Mensagem de inicialização.


  setupWiFi(); // Configura Wi-Fi.
  espClient.setInsecure(); // Configura cliente TLS sem verificação de certificado.
  client.setServer(mqtt_server, mqtt_port); // Configura o broker MQTT.
  client.setCallback(callback); // Define o callback para processar mensagens recebidas.
  verificarLeitorBiometrico(); // Verifica o leitor biométrico.
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pronto"); // Indica que o sistema está pronto.
}


// Loop principal
void loop() {
    if (!client.connected()) {
        reconnectMQTT(); // Reconecta ao MQTT se desconectado.
    }
    client.loop(); // Mantém a comunicação MQTT ativa.


    // Captura o ID biométrico.
    int idDigital = capturarIdDigital();


    if (idDigital >= 0) { // Verifica se a digital foi reconhecida.
        if (jaEntrou(idDigital)) {
            registrarSaida(idDigital); // A pessoa está saindo.
        } else {
            registrarEntrada(idDigital); // A pessoa está entrando.
        }
    } else if (idDigital == -2) {
        acessoNegado(); // Digital não reconhecida.
    }


    // Tempo limite para redefinir estado.
    if (millis() - ultimaLeitura > 10000) {
        resetarEstado();
    }
}