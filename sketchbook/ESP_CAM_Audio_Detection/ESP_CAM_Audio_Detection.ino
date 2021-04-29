/*
 * Licensed to the Apache Software Foundation (ASF) under one
or more contributor license agreements.  See the NOTICE file
distributed with this work for additional information
regarding copyright ownership.  The ASF licenses this file
to you under the Apache License, Version 2.0 (the
"License"); you may not use this file except in compliance
with the License.  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing,
software distributed under the License is distributed on an
"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
KIND, either express or implied.  See the License for the
specific language governing permissions and limitations
under the License.
*/

// Norwegian
// https://www.norwegiancreations.com/2017/08/what-is-fft-and-how-can-you-implement-it-on-an-arduino/
#include "arduinoFFT.h" // Standard Arduino FFT library https://github.com/kosme/arduinoFFT

arduinoFFT FFT = arduinoFFT();
#define SAMPLES 2048             // Must be a power of 2
#define SAMPLING_FREQUENCY 40000 // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define amplitude 150            // Depending on your audio source level, you may need to increase this value
unsigned int sampling_period_us;
double vReal1[SAMPLES];
double vImag1[SAMPLES];
int bb = 55; // nb de frequence qu'on garde
double traceur [275]={0}; // pour sauver les 5 FFT de 55 nombres
double local [10]={0}; // pour sauver les 10 localisations

// voir l'exemple FFT_01 https://github.com/kosme/arduinoFFT/blob/master/Examples/FFT_01/FFT_01.ino
unsigned long newTime, chrono1, chrono2;
int compteur_tour =0;
int Nb_chrono =0;

void setup() {

    Serial.begin(115200);
    pinMode(0, INPUT);  // entrée son sur GPIO 0
    sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
}


void loop(){

    //   prise de son et création de sa FFT
    for (int i = 0; i < SAMPLES; i++) {
        newTime  = micros();         //  lecture du son via un micro mis en GPIO0
        vReal1[i] = analogRead(0);   //  Using pin number for ADC port ici GPIO0
        vImag1[i] = 0;
        while ((micros() - newTime) < sampling_period_us) { /* do nothing to wait */ }
    }

    FFT.Windowing(vReal1, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal1, vImag1, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal1, vImag1, SAMPLES);

    // calcul du pic
    double pic1 = FFT.MajorPeak(vReal1, SAMPLES, SAMPLING_FREQUENCY);

    // début des tests de frelons asiatiques
    //pic1 = 220;  // pour test

    // on cherche si le pic = 234 +- 10%
    if (pic1 >210  && pic1 <=260) {
        // soupçon de frelon => on garde la trace de la FFT
        // enregistrement de la FFT
        // mise en tableau des 5 parties réelles vecteurs de la FFT
        for (int i = 2; i < bb; i++) { // i = 2 pour éliminer les basses fréquences < 78 Hz
            traceur [Nb_chrono*bb+i] = vReal1 [i];
            Serial.print(traceur [i]);
            Serial.print("  ");
        }

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
            delay(100); // attente
        }
        else { Nb_chrono = 0;}   // remise à zero du compteur

        if ( Nb_chrono > 4 ) {     // !!!! FRELON !!!!
            Nb_chrono=0;
            // affichage alerte  FRELON !
            Serial.println("  A L E R T E ");
            Serial.println(" F R E L O N S");
            Serial.println("   /-------!  ");
            Serial.println("   !  o-o  !  ");
            Serial.println("   !  (!)  ! ");
            Serial.println("   !___!___/  ");
        }
    }   // fin des tests
}   // fin de LOOP




