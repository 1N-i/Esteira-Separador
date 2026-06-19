#include <Wire.h>
#include "Adafruit_TCS34725.h"
#include <Servo.h>
#include <LiquidCrystal_I2C.h>

/* Servo Mega 2560
Connect SCL    to 21
Connect SDA    to 20
Connect VDD    to 3.3V DC
Connect GROUND to common ground */

/* Lcd Mega
Arduino Mega SCL SDA */

//Configuração: (Endereço I2C, Colunas, Linhas)
LiquidCrystal_I2C lcd(0x27, 16, 2); 

//Adafruit
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

void setup(void) {


  Serial.begin(9600);
  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
    while (20);
  }
  //Servos
  servoRed.attach(5);
  servoGreen.attach(6);
  servoBlue.attach(7);

  lcd.init(); //Inicializa o LCD
  lcd.backlight(); //Liga a luz de fundo
  
  //Mensagem de inicialização
  lcd.setCursor(0, 0);
  lcd.print("Ligado!");
  delay(500);
  lcd.clear();

  servoRed.write(0);
  servoGreen.write(0);
  servoBlue.write(0);
}

void vermelho() {
  servoRed.write(angulo);
  delay(dela_);
  servoRed.write(0);
  red += 1;
  tempo += (dela_ / 1000);
  contagem();
}

void verde() {
  delay(1000);
  servoGreen.write(angulo);
  delay(dela_);
  servoGreen.write(0);
  green += 1;
  tempo += (dela_ / 1000);
  contagem();
}

void azul() {
  delay(1250);
  servoBlue.write(angulo);
  delay(dela_);
  servoBlue.write(0);
  blue += 1;
  tempo += (dela_ / 1000);
  contagem();
}

void contagem() {
  Serial.print("Tempo: "); Serial.print(tempo); Serial.print("s; ");
  Serial.print("Vermelho: "); Serial.print(red); Serial.print("; ");
  Serial.print("Verde: "); Serial.print(green); Serial.print("; ");
  Serial.print("Azul: "); Serial.print(blue); Serial.print("; ");
  Serial.print("Total: "); Serial.print(red + blue + green); Serial.println("; ");
  Serial.println("--------------------------------------------------------");
}

void loop(void) {
  uint16_t r, g, b, c, colorTemp, lux;

  tcs.getRawData(&r, &g, &b, &c);
  colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);

/*
  //Visualização de valores da cor
  Serial.print("Color Temp: "); Serial.print(colorTemp, DEC);
  Serial.print(" R: "); Serial.print(r, DEC); Serial.print(" ");
  Serial.print("G: "); Serial.print(g, DEC); Serial.print(" ");
  Serial.print("B: "); Serial.print(b, DEC); Serial.print(" ");
  Serial.println(" ");
*/

  if ((r + g + b) > 765) {
    if ((r > g) && (r > b)) {
      vermelho();
    } else if ((g > r) && (g > b)) {
      verde();
    } else if ((b > r) && (b > g)) {
      azul();
    }
  }

  tempo += 1;
  
  //1ª Linha de 'RGB'
  lcd.setCursor(0, 0); //(Coluna 0, Linha 0)
  lcd.print("R:");
  lcd.setCursor(2, 0); //(Coluna 2, Linha 0)
  lcd.print(red);

  lcd.setCursor(5, 0);
  lcd.print("G:");
  lcd.setCursor(7, 0);
  lcd.print(green);

  lcd.setCursor(10, 0);
  lcd.print("B:");
  lcd.setCursor(12, 0);
  lcd.print(blue);
  
  //2ª Linha de 'Tempo e T'
  lcd.setCursor(0, 1); //(Coluna 0, Linha 1)
  lcd.print("Tempo:");
  lcd.setCursor(6, 1); //(Coluna 6, Linha 1)
  lcd.print(tempo);

  lcd.setCursor(11, 1);
  lcd.print("T:");
  lcd.setCursor(13, 1);
  lcd.print(red + blue + green);
}