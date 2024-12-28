#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <LiquidCrystal_PCF8574.h>

// Configuração do LCD I2C
#define LCD_COLUMNS 16
#define LCD_ROWS 2
#define I2C_ADDR 0x27  // Endereço do LCD I2C

LiquidCrystal_PCF8574 lcd(I2C_ADDR);  // Inicializa o LCD com o endereço I2C

// Definições dos pinos para LEDs, buzzer e relé
#define LED_VERDE 25
#define LED_AZUL 33
#define LED_VERMELHO 32
#define BUZZER_PIN 26
#define RELE_PIN 18

// Configuração do leitor biométrico usando Serial2
#define BIOMETRIC_RX_PIN 16
#define BIOMETRIC_TX_PIN 17
Adafruit_Fingerprint leitorBiometrico = Adafruit_Fingerprint(&Serial2); // Inicializa o leitor de digitais

// Variáveis para controle de temporização
unsigned long tempoLedVerde = 0;
unsigned long tempoLedVermelho = 0;
unsigned long duracaoLedVerde = 2000;
unsigned long duracaoLedVermelho = 2000;
unsigned long tempoAtivacaoRele = 0;
bool ledVerdeAceso = false;
bool ledVermelhoAceso = false;
bool releAtivo = false;

void setup() {
  inicializarLCD();
  inicializarSerial();
  configurarPinos();
  verificarLeitorBiometrico();
}

void loop() {
  int resultadoLeitura = capturarIdDigital();
  
  if (resultadoLeitura >= 0) { // Digital válida encontrada
    exibirAcessoPermitido();
    ativarRele();
    acenderLedVerde();
  } else if (resultadoLeitura == -2) { // Digital não encontrada
    exibirAcessoNegado();
    acenderLedVermelho();
  }

  // Gerenciamento de temporização dos LEDs e do relé
  gerenciarLedVerde();
  gerenciarLedVermelho();
  desativarRele();
}

// Função para configurar e inicializar o LCD
void inicializarLCD() {
  lcd.begin(LCD_COLUMNS, LCD_ROWS);
  lcd.setBacklight(255);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sistema Ativo");
  lcd.setCursor(0, 1);
  lcd.print("Inicializando...");
  delay(3000);
  lcd.clear();
}

// Função para inicializar a comunicação Serial com ESP32 e leitor biométrico
void inicializarSerial() {
  Serial.begin(115200);
  Serial2.begin(57600, SERIAL_8N1, BIOMETRIC_RX_PIN, BIOMETRIC_TX_PIN);
  leitorBiometrico.begin(57600);
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
    delay(2000);
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
  delay(1000);
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
