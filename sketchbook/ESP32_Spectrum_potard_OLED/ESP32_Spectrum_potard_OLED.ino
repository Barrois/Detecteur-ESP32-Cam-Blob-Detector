/* ESP8266/32 Audio Spectrum Analyser on an SSD1306/SH1106 Display
 * The MIT License (MIT) Copyright (c) 2017 by David Bird.
 * The formulation and display of an AUdio Spectrum using an ESp8266 or ESP32 and SSD1306 or SH1106 OLED Display using a Fast Fourier Transform
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, but not to use it commercially for profit making or to sub-license and/or to sell copies of the Software or to
 * permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * See more at http://dsbird.org.uk
*/

/*
// I2C
*/
#include <Wire.h>
const int PIN_SDA = 26;
const int PIN_SCL = 27; 

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Entrée potentiomètre
#define potardPin 32 //ADC1_CH4;

// Entrée micro
#define microPIN 33 // ADC1_CH5

#include "arduinoFFT.h" // Standard Arduino FFT library
// https://github.com/kosme/arduinoFFT, in IDE, Sketch, Include Library, Manage Library, then search for FFT
arduinoFFT FFT = arduinoFFT();
#define SAMPLES 1024             // Must be a power of 2
#define SAMPLING_FREQUENCY 10000 // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
uint8_t amplitude = 200;            // Depending on your audio source level, you may need to increase this value
unsigned int sampling_period_us;
double vReal1[SAMPLES];
double vImag1[SAMPLES];

double pic = 230;// ou  harmoniques 460 ou 920
double picSeuilBas = pic - (10 * pic /100);
double picSeuilHaut = pic + (10 * pic /100);

// voir l'exemple FFT_01 https://github.com/kosme/arduinoFFT/blob/master/Examples/FFT_01/FFT_01.ino
unsigned long newTime, chrono1, chrono2;
int compteur_tour =0;
int Nb_chrono =0;
// compteur d'attaque
int Cpt_Attaque=0;

// relai en GPIO D1
//#define Relai_Ext D1

void displayParameters(int range,int pic, bool detected) {
  display.clearDisplay();
  display.setTextSize(1.5);      // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE); // Draw white text
  display.setCursor(0, 0);     // Start at top-left corner
  //display.cp437(true);         // Use full 256 char 'Code Page 437' font
  display.print(F("seuil: "));display.println(range);
  display.print(F("pic: "));display.println(pic);
  if (detected) {
      display.println("!  o-o  !");
      display.println("!  (!)  !");
  }
  display.display(); 
}

void setup() {
  Serial.begin(115200);
  Serial.println(__FILE__);
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));

  Wire.begin(PIN_SDA, PIN_SCL);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);


  picSeuilBas = pic - (10 * pic /100);
  picSeuilHaut = pic + (10 * pic /100);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  delay(1000);
}

void loop(){
  Serial.println(LED_BUILTIN);
  digitalWrite(LED_BUILTIN, HIGH);
  // lecture potentiomètre
  int16_t adc0;
  adc0 = analogRead(potardPin);
  // interval de valeurs 100 à 1000
  int range = map(adc0, 0, 4095, 100, 1000);
  Serial.println("-----------------------------------------------------------");
  Serial.print("AIN0: "); Serial.println(adc0);
  Serial.print("pic map: "); Serial.println(range);
  Serial.println("-----------------------------------------------------------");
  picSeuilBas = range - (10 * range /100);
  picSeuilHaut = range + (10 * range /100);

  
  //
  //   prise de son et création de sa FFT
  //
  for (int i = 0; i < SAMPLES; i++) {
    newTime  = micros();         //  lecture du son via un micro mis en ADC0
    vReal1[i] = analogRead(microPIN);   //  Using pin number for ADC port ici ADC0
    vImag1[i] = 0;
    while ((micros() - newTime) < sampling_period_us) { /* do nothing to wait */ }
  }
  FFT.Windowing(vReal1, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal1, vImag1, SAMPLES, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal1, vImag1, SAMPLES);

  // calcul du pic
  double pic1 = FFT.MajorPeak(vReal1, SAMPLES, SAMPLING_FREQUENCY);
  Serial.println(pic1);
  displayParameters(range, (int) pic1, false);
  //picSeuilHaut = pic + (10 * seuil /100);
  // on cherche si le pic = 234 +- 10%
  if (pic1 >picSeuilBas  && pic1 <=picSeuilHaut) {         // soupçon de frelon
    // est-ce que il y a  5 pics en 235 en moins de 30 secondes ?
    chrono2 = millis(); // demarrage du chrono 2
    if (Nb_chrono == 0) {
      chrono1 = chrono2 ;  // remise à l'heure du compteur 1
    }
    unsigned long interval;
    interval = chrono2 - chrono1 ;
    if (interval < 5000 ) {     // les deux pics sont séparés de moins de 5s
      chrono1 = chrono2;
      chrono2 = millis();
      Nb_chrono++;
      delay(10); // attente
    }
    else {
      // remise à zero du compteur
      Nb_chrono = 0;
    }
    if ( Nb_chrono > 4 ) {     // !!!! FRELON !!!!
      Nb_chrono=0;
      // affichage alerte  FRELON !
      displayParameters(range, (int) pic1, true);
      Serial.println("  A L E R T E ");
      Serial.println(" F R E L O N S");
      Serial.println("   /-------!  ");
      Serial.println("   !  o-o  !  ");
      Serial.println("   !  (!)  ! ");
      Serial.println("   !___!___/  ");
      // mise du relai externe en D5 High pour tir
      digitalWrite(LED_BUILTIN, LOW);
      delay(1000);
    }
  }
}
