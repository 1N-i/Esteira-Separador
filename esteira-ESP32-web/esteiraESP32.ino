#include <Wire.h>
#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ESP32Servo.h>
#include "Adafruit_TCS34725.h"
#include <LiquidCrystal_I2C.h>

//Credenciais do seu Wi-Fi
const char* ssid = "Nome do wifi";
const char* password = "Senha";

//Inicializa o Servidor WebSocket na porta 81
WebSocketsServer webSocket = WebSocketsServer(81);

//Configuração I2C (LCD e Sensor) - ESP32 usa SDA 21, SCL 22 por padrão
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_24MS, TCS34725_GAIN_16X);

//Servos
Servo servoRed; 
Servo servoBlue;
Servo servoGreen;

//Variáveis
int red = 0;
int blue = 0;
int green = 0;
int dela_ = 1250;
int tempo = 0;
int angulo = 60;

unsigned long ultimoTempo = 0;

void setup() {
  Serial.begin(115200); //ESP32 geralmente usa 115200

  //Conexão Wi-Fi
  Serial.print("Conectando ao Wi-Fi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado! IP do ESP32: ");
  Serial.println(WiFi.localIP()); //Anote este IP para por no site

  //Inicia WebSocket
  webSocket.begin();

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (1);
  }

  //Pinos dos Servos no ESP32 (Exemplo: 13, 12, 14)
  servoRed.attach(13);
  servoGreen.attach(12);
  servoBlue.attach(14);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("IP: ");
  lcd.print(WiFi.localIP()); //Mostra o IP no visor para facilitar

  servoRed.write(0);
  servoGreen.write(0);
  servoBlue.write(0);
}

void enviarDadosWeb() {
  //Monta a string que o JavaScript espera
  String payload = "Tempo: " + String(tempo) + "s; " +
                   "Vermelho: " + String(red) + "; " +
                   "Azul: " + String(blue) + "; " +
                   "Verde: " + String(green) + "; " +
                   "Total: " + String(red + blue + green) + ";\n";
  
  //Envia para todos os clientes conectados no site
  webSocket.broadcastTXT(payload);
}

void vermelho() {
  servoRed.write(angulo);
  delay(dela_);
  servoRed.write(0);
  red += 1;
  enviarDadosWeb();
}

void verde() {
  servoGreen.write(angulo);
  delay(dela_);
  servoGreen.write(0);
  green += 1;
  enviarDadosWeb();
}

void azul() {
  servoBlue.write(angulo);
  delay(dela_);
  servoBlue.write(0);
  blue += 1;
  enviarDadosWeb();
}

void loop() {
  webSocket.loop(); //Mantém a conexão viva

  //Lógica de tempo sem travar o código (substitui o tempo += 1 solto)
  if (millis() - ultimoTempo >= 1000) {
    tempo += 1;
    ultimoTempo = millis();
    enviarDadosWeb(); //Atualiza o relógio no site a cada segundo
  }

  uint16_t r, g, b, c;
  tcs.getRawData(&r, &g, &b, &c);

  if ((r + g + b) > 815) {
    if ((r > g) && (r > b)) {
      vermelho();
    } else if ((g > r) && (g > b)) {
      verde();
    } else if ((b > r) && (b > g)) {
      azul();
    }
  }

  //Atualização básica do LCD
  lcd.setCursor(0, 1);
  lcd.print("R:"); lcd.print(red);
  lcd.print(" G:"); lcd.print(green);
  lcd.print(" B:"); lcd.print(blue);
}