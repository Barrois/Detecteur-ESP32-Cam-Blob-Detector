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
/* https://robotzero.one/time-lapse-esp32-cameras/
 * https://github.com/robotzero1/esp32cam-timelapse/blob/master/timelapse-sd.ino
 */
  //
 // remise à zero du compteur de vue 
 //

#include "esp_camera.h"
#include "esp_timer.h"
#define CAMERA_MODEL_AI_THINKER 
#include "camera_pins.h"
#include "Arduino.h"
// MicroSD
#include "driver/sdmmc_host.h"
#include "driver/sdmmc_defs.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"

#include <EEPROM.h>            // read and write from flash memory
// define the number of bytes you want to access
#define EEPROM_SIZE 1
int file_number;

int capture_interval = 5000; // microseconds between captures

static esp_err_t cam_err;
static esp_err_t card_err;

#define FRAME_SIZE FRAMESIZE_QVGA
#define WIDTH 320
#define HEIGHT 240
#define BLOCK_SIZE 4
#define W (WIDTH / BLOCK_SIZE)
#define H (HEIGHT / BLOCK_SIZE)
#define BLOCK_DIFF_THRESHOLD 0.2
#define IMAGE_DIFF_THRESHOLD 0.1
#define DEBUG 0

uint16_t current_frame[H][W] = { 0 };
uint8_t color =255;
int color_x, color_y;

bool setup_camera(framesize_t);
bool capture_still();
bool motion_detect();
void update_frame();
void print_frame(uint16_t frame[H][W]);
void print_blob (uint16_t frame[H][W]);
int  j_loca=0; // pour ecrire dans le fichier local(30)

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
int bb =55; // nb de frequence qu'on garde 
double traceur [275]={0}; // pour sauver les 5 FFT de 55 nombres
double local [10]={0}; // pour sauver les 10 localisations

//    voir l'exemple FFT_01 https://github.com/kosme/arduinoFFT/blob/master/Examples/FFT_01/FFT_01.ino
unsigned long newTime, chrono1, chrono2;
int compteur_tour =0;
int Nb_chrono =0;
// compteur d'attaque
int Cpt_Attaque=0;

// relai en GPIO 16
#define Relai_Ext 16

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  3   // durée du sommeil en secondes ici 900 = 1/4 d'heure

void setup() {
    // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  file_number = EEPROM.read(0) + 1;  
  if(file_number > 255) file_number = 0;  // remise à zéro du compteur de vue
  
  Serial.begin(115200);
  Serial.println(__FILE__);
      pinMode(0, INPUT);  // entrée son sur GPIO 0
  // camera    
     Serial.println(setup_camera(FRAME_SIZE) ? "OK" : "ERR INIT");

  // SD camera init
  card_err = init_sdcard();
  if (card_err != ESP_OK) {
    Serial.printf("SD Card init failed with error 0x%x", card_err);
    return;
  }
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
  pinMode(Relai_Ext, OUTPUT);

  // prise d'une photo au debut
    Serial.println("+++ prise une premiere photo au debut");  
        capture_still()  ;  

    // save_photo_numbered(); 
  // prise d'une localisation  au debut
      Serial.println("+++ prise une localisation au debut ");
    if (!capture_still()) {
        Serial.println("Failed capture");
        delay(3000);
        return;
    }
     save_loca_numbered();  // ecriture du fichier des localisations  
      Serial.println("fin  de la localisation ");       
}
static esp_err_t init_sdcard(){
  esp_err_t ret = ESP_FAIL;
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
  esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    .format_if_mount_failed = false,
    .max_files = 1,
  };
  sdmmc_card_t *card;

  Serial.println("Mounting SD card...");
  ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

  if (ret == ESP_OK) {
    Serial.println("SD card mount successfully!");
  }  else  {
    Serial.printf("Failed to mount SD card VFAT filesystem. Error: %s", esp_err_to_name(ret));
  }
}
static esp_err_t save_photo_numbered(){
  Serial.print("Taking picture: ");
  Serial.println(file_number);
  delay(1000);
  camera_fb_t *fb = esp_camera_fb_get();

  //char *filename = (char*)malloc(21 + sizeof(int));
  char *filename = (char*)malloc(21 + sizeof(file_number));
  sprintf(filename, "/sdcard/capture_%d.jpg", file_number);

  Serial.println(filename);
  FILE *file = fopen(filename, "w");
  if (file != NULL)  {
    size_t err = fwrite(fb->buf, 1, fb->len, file);
    Serial.printf("File saved: %s\n", filename);
  }  else  {
    Serial.println("Could not open file");
  }
  fclose(file);
  esp_camera_fb_return(fb);
  free(filename);
}
static esp_err_t save_FFT_numbered(){
  Serial.print("Taking FFT: ");
  Serial.println(file_number);
  char *filename = (char*)malloc(21 + sizeof(file_number));
  sprintf(filename, "/sdcard/FFT_%d.txt", file_number);
  Serial.println(filename);
  FILE *file = fopen(filename, "w");
  if (file != NULL)  {
      for (int i = 0; i < 5*bb; i++) { // 500 = 100 * 5 
          char x_buffer[20];
          dtostrf(traceur[i], 5,1, x_buffer);
      fprintf(file,x_buffer);
      fprintf(file,";"); 
       }
       }  
       else  {
    Serial.println("Could not open file");
  }
  fclose(file);
  free(filename);
}
static esp_err_t save_loca_numbered(){
  Serial.print("Taking LOCALISATION : ");
  Serial.println(file_number);
  char *filename = (char*)malloc(21 + sizeof(file_number));
  sprintf(filename, "/sdcard/LOCA_%d.txt", file_number);
  Serial.println(filename);
  FILE *file = fopen(filename, "w");
  if (file != NULL)  {
      for (int i = 0; i < 10; i++) { // 10
          char x_buffer[20];
          dtostrf(local[i], 5,1, x_buffer);
      fprintf(file,x_buffer);
      fprintf(file,";"); 
       }
       }  
       else  {
    Serial.println("Could not open file");
  }
  fclose(file);
  free(filename);
}

void loop(){  
  digitalWrite(Relai_Ext, LOW);   // relai externe mis bas  
      //
      //   prise de son et création de sa FFT
      //
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
//
// début des tests de frelons asiatiques
// 

pic1 = 220;  // pour test


       // on cherche si le pic = 234 +- 10%
      if (pic1 >210  && pic1 <=260) {   
  
        // soupçon de frelon => on garde la trace de la FFT
          // enregistrement de la FFT 
             Serial.print("debut du vecteur FFT : ");
             Serial.println(Nb_chrono);
          // mise en tableau des 5 parties réelles vecteurs de la FFT
           for (int i = 2; i < bb; i++) { // i = 2 pour éliminer les basses fréquences < 78 Hz
            traceur [Nb_chrono*bb+i] = vReal1 [i];  
            Serial.print(traceur [i]);
            Serial.print("  ");
                 }        
            Serial.println("fin du vecteur FFT");
        //    Serial.println("Computed magnitudes: apres _1 ");
        //    PrintVector(traceur, bb, SCL_FREQUENCY);
  Serial.print("0_Nb_chrono : ");
  Serial.print (Nb_chrono);
  Serial.print(" chrono1 : ");
  Serial.print (chrono1);  
  Serial.print(" chrono2 : ");
  Serial.println(chrono2);            
    // est-ce que il y a  5 pics en 235 en moins de 30 secondes ? 
        chrono2 = millis(); // demarrage du chrono 2  
       if (Nb_chrono == 0) {
       chrono1 = chrono2 ;  // remise à l'heure du compteur 1 
       }     
       unsigned long interval;     
       interval = chrono2 - chrono1 ;
  Serial.print("1_Nb_chrono : ");
  Serial.print (Nb_chrono);
  Serial.print(" chrono1 : ");
  Serial.print (chrono1);
  Serial.print(" interval : ");
  Serial.print (interval);  
  Serial.print(" chrono2 : ");
  Serial.println(chrono2);
     if (interval < 5000 ) {     // les deux pics sont séparés de moins de 5s
                            chrono1 = chrono2;
                            chrono2 = millis();
                            Nb_chrono++;
                            delay(100); // attente 
                            }
                     else { Nb_chrono = 0;}   // remise à zero du compteur
  Serial.print("2_Nb_chrono : ");
  Serial.print (Nb_chrono);
  Serial.print(" chrono1 : ");
  Serial.print (chrono1);
  Serial.print(" interval : ");
  Serial.print (interval);  
  Serial.print(" chrono2 : ");
  Serial.println(chrono2);                       
    if ( Nb_chrono > 4 ) {     // !!!! FRELON !!!!          
         Nb_chrono=0;       
    // affichage alerte  FRELON !     
      Serial.println("  A L E R T E ");
      Serial.println(" F R E L O N S");
      Serial.println("   /-------!  ");      
      Serial.println("   !  o-o  !  ");
      Serial.println("   !  (!)  ! ");
      Serial.println("   !___!___/  ");
   // mise du relai externe en G16 High
     digitalWrite(Relai_Ext, HIGH);     

     save_FFT_numbered();

  // prise d'une localisation  au debut
      Serial.println("+++ prise une localisation au debut ");
    if (!capture_still()) {
        Serial.println("Failed capture");
        delay(3000);
        return;
    }
     save_loca_numbered();  // ecriture du fichier des localisations  
      Serial.println("fin  de la localisation ");    
     
     delay(1000);
    file_number++;
    EEPROM.write(0, file_number);
    EEPROM.commit();
     /*
     delay(1000);
     save_photo_numbered();
    // on endort la bete !!!
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
    Serial.println("This will never be printed");
    */
    ESP.restart();  // redémarrage      
    }              
    }   // fin des tests           
    }   // fin de LOOP
bool setup_camera(framesize_t frameSize) {
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
    config.pixel_format = PIXFORMAT_GRAYSCALE;
    config.frame_size = frameSize;
    config.jpeg_quality = 12;
    config.fb_count = 1;

    bool ok = esp_camera_init(&config) == ESP_OK;

    sensor_t *sensor = esp_camera_sensor_get();
    sensor->set_framesize(sensor, frameSize);

    return ok;
}
bool capture_still() {

  Serial.print(" photo !! ");    
    camera_fb_t *frame_buffer = esp_camera_fb_get();
 //
// vient de EsayMotionDetectionGrayscale 
//
            Serial.println("The image will be saved as /capture.jpg");
            jpegWriter.writeGrayscale(imageFile, frame->buf, quality);
            imageFile.close();
            printFilesize("/capture.jpg");    

    if (!frame_buffer)
        return false;

    // set all 0s in current frame
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            current_frame[y][x] = 0;

    // down-sample image in blocks
    for (uint32_t i = 0; i < WIDTH * HEIGHT; i++) {
        const uint16_t x = i % WIDTH;
        const uint16_t y = floor(i / WIDTH);
        const uint8_t block_x = floor(x / BLOCK_SIZE);
        const uint8_t block_y = floor(y / BLOCK_SIZE);
        const uint8_t pixel = frame_buffer->buf[i];
        const uint16_t current = current_frame[block_y][block_x];

        // average pixels in block (accumulate)
        current_frame[block_y][block_x] += pixel;
    } 
    // average pixels in block (rescale)
    for (int y = 0; y < H; y++)
        for (int x = 0; x < W; x++)
            current_frame[y][x] /= BLOCK_SIZE * BLOCK_SIZE;
            
    Serial.print("xx blob sombre : ");
    print_blob(current_frame);

#if DEBUG
    Serial.println("Current frame:");
    print_frame(current_frame);
    Serial.println("---------------");
#endif

    return true;
}
void print_blob(uint16_t frame[H][W]) {
   int  xcolor = 256; // reinitialisation à blanc  
   int  xcolor_x, xcolor_y;
  Serial.print("Taking LOCALISATION : ");
  Serial.println(file_number);
  char *filename = (char*)malloc(21 + sizeof(file_number));
  sprintf(filename, "/sdcard/PHOTO_%d.txt", file_number);
  Serial.println(filename);
  FILE *file = fopen(filename, "w");
  if (file != NULL)  {

    for (int y = 0; y < H; y++) {
        for (int x = 0; x < W; x++) {
          //  Serial.print(frame[y][x]);
          //  Serial.print('\t');
          char x_buffer[20];
          dtostrf(frame[y][x], 5,1, x_buffer);
          fprintf(file,x_buffer);
          fprintf(file," ; "); 
    if (current_frame[y][x] < xcolor) {
      xcolor = current_frame[y][x];
      xcolor_x=x;
      xcolor_y=y;
    }            
        }
         fprintf(file,"\n"); 
       // Serial.println();
    } 
    Serial.print("xx blob sombre : ");
    Serial.print(xcolor);
    Serial.print(", en x :"); 
    Serial.print(xcolor_x);
    Serial.print(", en y :"); 
    Serial.print(xcolor_y);
    Serial.println("-------xxx--------");        

    local[  j_loca] = xcolor_x;
    local[1+j_loca] = xcolor_y;
    local[2+j_loca] = xcolor;
    j_loca = j_loca+3;
 }  
       else  {
    Serial.println("Could not open file");
  }
  fclose(file);
  free(filename);
}    
