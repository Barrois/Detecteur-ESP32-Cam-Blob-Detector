/* https://robotzero.one/time-lapse-esp32-cameras/
 * https://github.com/robotzero1/esp32cam-timelapse/blob/master/timelapse-sd.ino
 */
#include "esp_camera.h"
#include "esp_timer.h"
//#include <WiFi.h>
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

// CAMERA_MODEL_AI_THINKER
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
/*
//   tracer de la FFT  voir l'exemple FFT_01
#define SCL_FREQUENCY 0x02 
#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03
*/

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
  
  Serial.begin(115200);
  Serial.println(__FILE__);
      pinMode(0, INPUT);  // entrée son sur GPIO 0
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
  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }
  // camera init
  cam_err = esp_camera_init(&config);
  if (cam_err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", cam_err);
    return;
  }
  // SD camera init
  card_err = init_sdcard();
  if (card_err != ESP_OK) {
    Serial.printf("SD Card init failed with error 0x%x", card_err);
    return;
  }
  sampling_period_us = round(1000000 * (1.0 / SAMPLING_FREQUENCY));
  pinMode(Relai_Ext, OUTPUT);

  // prise d'une photo 
   save_photo_numbered(); 
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
  Serial.print("MajorPeak : ");
  Serial.print(pic1);
  Serial.print(" 18 ");
  Serial.print(vReal1[17]);
  Serial.print(" : ");
  Serial.print(vReal1[18]);
  Serial.print(" : ");
  Serial.print(vReal1[19]);
  Serial.print(" 36 ");
  Serial.print(vReal1[35]);
  Serial.print(" : ");
  Serial.print(vReal1[36]);
  Serial.print(" : ");
  Serial.print(vReal1[37]);
  Serial.print(" 42 ");
  Serial.print(vReal1[41]);
  Serial.print(" : ");
  Serial.print(vReal1[42]);
  Serial.print(" : ");
  Serial.println(vReal1[43]);  
  
  /*
  Serial.println(pic1, 2);
           for (int i = 2; i < 20; i++) { // i = 2 pour éliminer les basses fréquences < 78 Hz
            Serial.print(vReal1[i]);
            Serial.print("  ");
                 }        
            Serial.println("fin du vecteur FFT");
  */
//
// début des tests de frelons asiatiques
// 
       // on cherche si le pic = 234 +- 10%
      if (pic1 >210  && pic1 <=260) {   
       // les valeurs en 18, 36 et 42 sont-elles des pics ? 
       int test_pic  = 0;     
      if (vReal1[18] > (0.66* (vReal1[17]+ vReal1[19]))     ) test_pic++;   // pic en 351 en SAMPLES 2048   
      if (vReal1[36] > (0.66* (vReal1[35]+ vReal1[37]))     ) test_pic++;   // pic en 703 en SAMPLES 2048      
      if (vReal1[42] > (0.66* (vReal1[41]+ vReal1[43]))     ) test_pic++;   // pic en 820 en SAMPLES 2048
      if  (test_pic = 3) {
    // si la somme des trois pics secondaires est supérieure au pic principal => FRELON !!            
      double val_pic = vReal1[6];    
      double val_pic2 = (vReal1[18]+vReal1[36]+vReal1[42]);
      if (val_pic2 > val_pic )  {
        
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
    }
    }  
    }   // fin des tests           
    }   // fin de LOOP

/*    
void PrintVector(double *vData, uint16_t bufferSize, uint8_t scaleType)
// tracer de la FFT    voir la fin de l'exemple FFT_01 
{  for (uint16_t i = 0; i < bufferSize; i++)
  {    double abscissa;
    // Print abscissa value //
    switch (scaleType)
    { case SCL_INDEX:
        abscissa = (i * 1.0);
       break;
      case SCL_TIME:
        abscissa = ((i * 1.0) / SAMPLING_FREQUENCY);
      break;
      case SCL_FREQUENCY:
        abscissa = ((i * 1.0 * SAMPLING_FREQUENCY) / SAMPLES);
      break;
    }
    Serial.print(abscissa, 1);
    if(scaleType==SCL_FREQUENCY)  Serial.print(" Hz   ");
    Serial.print(" ");
    Serial.println(vData[i], 0);
  }
  Serial.println();
}
 */   
