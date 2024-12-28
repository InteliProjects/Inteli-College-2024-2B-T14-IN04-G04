#include <LiquidCrystal_I2C.h> // Inclui a biblioteca do display

// Classe que controla o sistema de acesso
class SistemaAcesso {
  private:
    LiquidCrystal_I2C lcd;   // Instância do display LCD
    int ledVerde;            // Pino do LED verde
    int ledVermelho;         // Pino do LED vermelho
    int ledPorta;            // Pino do LED que simula a porta
    int botaoAutorizado;     // Pino do botão de acesso autorizado
    int botaoNaoAutorizado;  // Pino do botão de acesso não autorizado

  public:
    // Construtor para inicializar pinos e o display
    SistemaAcesso(int lcdAddr, int lcdCols, int lcdRows, int pinoLedVerde, int pinoLedVermelho, int pinoLedPorta, int pinoBotaoAutorizado, int pinoBotaoNaoAutorizado)
      : lcd(lcdAddr, lcdCols, lcdRows), ledVerde(pinoLedVerde), ledVermelho(pinoLedVermelho), ledPorta(pinoLedPorta), botaoAutorizado(pinoBotaoAutorizado), botaoNaoAutorizado(pinoBotaoNaoAutorizado) {}

    // Configuração inicial do sistema
    void inicializar() {
      pinMode(ledVerde, OUTPUT);
      pinMode(ledVermelho, OUTPUT);
      pinMode(ledPorta, OUTPUT);
      pinMode(botaoAutorizado, INPUT_PULLUP);
      pinMode(botaoNaoAutorizado, INPUT_PULLUP);

      lcd.init();
      lcd.backlight();
    }

    // Verifica o estado dos botões e toma as ações apropriadas
    void verificarBotoes() {
      int estadoBotaoAutorizado = digitalRead(botaoAutorizado);
      int estadoBotaoNaoAutorizado = digitalRead(botaoNaoAutorizado);

      if (estadoBotaoAutorizado == HIGH && estadoBotaoNaoAutorizado == HIGH) {
        // Nenhum botão pressionado: Apaga todos os LEDs e limpa o display
        resetarSistema();
      } else if (estadoBotaoAutorizado == LOW) {
        // Botão autorizado pressionado: Mostra mensagem de boas-vindas e acende o LED verde
        acessoAutorizado();
      } else if (estadoBotaoNaoAutorizado == LOW) {
        // Botão não autorizado pressionado: Mostra mensagem de acesso negado e acende o LED vermelho
        acessoNegado();
      }
    }

  private:
    // Função para resetar o sistema (apagando LEDs e limpando o display)
    void resetarSistema() {
      digitalWrite(ledVerde, LOW);
      digitalWrite(ledVermelho, LOW);
      digitalWrite(ledPorta, LOW);
      lcd.clear();
    }

    // Função que simula o acesso autorizado
    void acessoAutorizado() {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Seja bem-vindo!");
      digitalWrite(ledVerde, HIGH);
      digitalWrite(ledPorta, HIGH);
      digitalWrite(ledVermelho, LOW);
      delay(2000); // Simula o tempo de abertura da porta
      lcd.clear();
    }

    // Função que simula o acesso negado
    void acessoNegado() {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Acesso negado");
      digitalWrite(ledVermelho, HIGH);
      digitalWrite(ledVerde, LOW);
      digitalWrite(ledPorta, LOW);
      delay(2000); // Simula o tempo de resposta ao negar acesso
      lcd.clear();
    }
};

// Definição dos pinos
const int ledVerde = 12;
const int ledVermelho = 27;
const int ledPorta = 25;
const int botaoAutorizado = 13;
const int botaoNaoAutorizado = 14;

// Instância da classe SistemaAcesso
SistemaAcesso sistema(0x27, 16, 2, ledVerde, ledVermelho, ledPorta, botaoAutorizado, botaoNaoAutorizado);

void setup() {
  // Inicializa o sistema de acesso
  sistema.inicializar();
}

void loop() {
  // Verifica o estado dos botões continuamente
  sistema.verificarBotoes();
}
