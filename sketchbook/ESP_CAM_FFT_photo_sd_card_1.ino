/*********
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-cam-take-photo-save-microsd-card
  
  IMPORTANT!!! 
   - Select Board "AI Thinker ESP32-CAM"
   - GPIO 0 must be connected to GND to upload a sketch
   - After connecting GPIO 0 to GND, press the ESP32-CAM on-board RESET button to put your board in flashing mode
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*********/

#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            // read and write from flash memory
#include "esp_vfs_fat.h"

// define the number of bytes you want to access
#define EEPROM_SIZE 1

// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27

#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

int pictureNumber = 0;
// Norwegian
// https://www.norwegiancreations.com/2017/08/what-is-fft-and-how-can-you-implement-it-on-an-arduino/
#include "arduinoFFT.h" // Standard Arduino FFT library https://github.com/kosme/arduinoFFT
arduinoFFT FFT = arduinoFFT();
#define SAMPLES 2048            // Must be a power of 2
#define SAMPLING_FREQUENCY 40000 // Hz, must be 40000 or less due to ADC conversion time. Determines maximum frequency that can be analysed by the FFT Fmax=sampleF/2.
#define amplitude 150            // Depending on your audio source level, you may need to increase this value
unsigned int sampling_period_us;
double vReal1[SAMPLES];
double vImag1[SAMPLES];
int bb =55; // nb de frequence qu'on garde 
double traceur [275]={0}; // pour sauver les 5 FFT de 55 nombres
double local [10]={0}; // pour sauver les 10 localisations

//    voir l'exemple FFT_01 https://github.com/kosme/arduinoFFT/blob/master/Examples/FFT_01/FFT_01.ino
unsigned long newTime, chrono1, chrono2;
int compteur_tour =0;
int Nb_chrono =0;
// compteur d'attaque
int Cpt_Attaque=0;
int file_number=0;
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
 
  Serial.begin(115200);
  Serial.println(__FILE__);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG; 
  
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  
  //Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  }
  
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  }
  }

void loop() {
    //   prise de son et création de sa FFT
    for (int i = 0; i < SAMPLES; i++) {
    newTime  = micros();         //  lecture du son via un micro mis en GPIO0
    vReal1[i] = analogRead(0);   //  pour test
    //vReal1[i] = random(1000);   //  Using pin number for ADC port ici GPIO0
    vImag1[i] = 0;
    //while ((micros() - newTime) < sampling_period_us) { /* do nothing to wait */ }
    }
    FFT.Windowing(vReal1, SAMPLES, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal1, vImag1, SAMPLES, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal1, vImag1, SAMPLES);
    // calcul du pic   
    double pic1 = FFT.MajorPeak(vReal1, SAMPLES, SAMPLING_FREQUENCY); 
    //
    // début des tests de frelons asiatiques
    //     
    // pic1 = 220;  // pour test
       // on cherche si le pic = 234 +- 10%
      if (pic1 >210  && pic1 <=260) {   
         
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

    Serial.print("Start photo");
        
      camera_fb_t * fb = NULL;
      
      // Take Picture with Camera
      fb = esp_camera_fb_get();  
      if(!fb) {
        Serial.println("Camera capture failed");
        return;
      }
      // initialize EEPROM with predefined size
      EEPROM.begin(EEPROM_SIZE);
      pictureNumber = EEPROM.read(0) + 1;
    
      // Path where new picture will be saved in SD Card
      String path = "/picture" + String(pictureNumber) +".jpg";
    
      fs::FS &fs = SD_MMC; 
      Serial.printf("Picture file name: %s\n", path.c_str());
      
      File file = fs.open(path.c_str(), FILE_WRITE);
      if(!file){
        Serial.println("Failed to open file in writing mode");
      } 
      else {
        file.write(fb->buf, fb->len); // payload (image), payload length
        Serial.printf("Saved file to path: %s\n", path.c_str());
        EEPROM.write(0, pictureNumber);
        EEPROM.commit();
      }
      file.close();
      esp_camera_fb_return(fb); 
      
      // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
      pinMode(4, OUTPUT);
      digitalWrite(4, LOW);
      rtc_gpio_hold_en(GPIO_NUM_4);

      ESP.restart();  // redémarrage 
      /*
      delay(2000);
      Serial.println("Going to sleep now");
      delay(2000);
      esp_deep_sleep_start();
      Serial.println("This will never be printed");
      */
    }              
    }   // fin des tests           
    }   // fin de LOOP
