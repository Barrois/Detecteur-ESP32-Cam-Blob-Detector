#define CAMERA_MODEL_AI_THINKER

#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include <SPIFFS.h>
#include <EloquentArduino.h>
#include <eloquentarduino/io/serial_print.h>
#include <eloquentarduino/vision/camera/ESP32Camera.h>
#include <eloquentarduino/vision/io/writers/JpegWriter.h>

#define FRAME_SIZE FRAMESIZE_QVGA
#define PIXFORMAT PIXFORMAT_GRAYSCALE
#define W 320
#define H 240
#define w 80 //32
#define h 60 //24
#define DIFF_THRESHOLD 15
#define MOTION_THRESHOLD 0.15

// delete the second definition if you want to turn on code benchmarking
#define timeit(label, code) { uint32_t start = millis(); code; uint32_t duration = millis() - start; eloquent::io::print_all("It took ", duration, " millis for ", label); }
#define timeit(label, code) code;

using namespace Eloquent::Vision;

camera_fb_t *frame;
Camera::ESP32Camera camera(PIXFORMAT);
uint8_t downscaled[w * h];
IO::Decoders::GrayscaleRandomAccessDecoder decoder;
//IO::Decoders::Red565RandomAccessDecoder decoder;
Processing::Downscaling::Center<W / w, H / h> strategy;
Processing::Downscaling::Downscaler<W, H, w, h> downscaler(&decoder, &strategy);
Processing::MotionDetector<w, h> motion;
IO::Writers::JpegWriter<W, H> jpegWriter;


void capture();
void save();
void stream_downscaled();
void stream();
unsigned int pictureNumber = 0;
String path;
/*
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
const int PIN_SDA = 1;  // port 1 du esp32-cam sur le SDA
const int PIN_SCL = 3;  // port 3 du esp32-cam sur le SCL

const int PAN_ID = 0;  // Pan  sur le port 0 du PCA9685
const int TILT_ID = 1; // Tilt sur le port 1 du PCA9685

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define SERVOMIN  150 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // this is the 'maximum' pulse length count (out of 4096)
*/
int angle_pan , angle_til ; // position du point noir transmise au Wemos

unsigned long previousMillis = 0;// on stocke le temps de la derniere execution

const long interval = 5000; // intervale entre chaque execution (milliseconds)

int debug_1 = 2; // mettre debug_1 =1  pour envoyer sur le Serial 
                 // mettre debug_1 =2 pour sauvegarder le fichier dowscale sur SD 
                 // mettre debug_1 =3 pour envoyer sur le Serial des résultats
void setup() {
  Serial.begin(9600);
 // Serial.println(__FILE__);
  SPIFFS.begin(true);
  delay(100);

  //Initialize MicroSD
 // Serial.print("Initializing the MicroSD card module... ");
  initMicroSDCard();

//  Wire.begin(PIN_SDA, PIN_SCL);
//  pwm.begin();
//  pwm.setPWMFreq(60);

  camera.begin(FRAME_SIZE);
}

void loop() {
 // pwm.setPWM(PAN_ID,  0, angleToPulse(angle_pan_1));
 //pwm.setPWM(TILT_ID, 0, angleToPulse(angle_til_1));
  capture();
 //Path where new picture will be saved in SD Card
  path = "/picture" + String(pictureNumber) +".jpg";
 //  Serial.printf("Picture file name: %s\n", path.c_str());
  //Take and Save Photo
  takeSavePhoto(path);
  delay(100);
if (debug_1 ==2) {  // enregistrer le tableau "downscaled" vers la carte SD
save_downscaled_numbered();  
  delay(100);
}
  pictureNumber++;
 // while(1);
}

void capture() {
  timeit("capture frame", frame = camera.capture());
  // scale image from size H * W to size h * w
  timeit("downscale", downscaler.downscale(frame->buf, downscaled));

  eloquent::io::print_all("START OF BERNARD");

  int ind1=0;
if (debug_1 ==1) {
// écriture du tableau des nuances de gris de la photo
    Serial.println("tableau i = w , j = h");
    for(int i = 0; i < h; i++) {
    for(int j = 0; j < w; j++) {
      ind1 ++;
      Serial.print(downscaled[ind1]);
      Serial.print(";");
    }
    Serial.println();
    }
}

  // bornes de fenétrage en pixels mesurées sur la photo
  int by1 = 19; //  valeur de la première ligne sur le tableau Excel downscaled[ind1]
  int by2 = 44; //  valeur de la dernière ligne
  // delta_y by2 - by1 = 44-19 =  25 à 50 cm
  int bx1 = 30; //  valeur de la première colonne sur le tableau Excel downscaled[ind1]
  int bx2 = 55; //  valeur de la dernière colonne
  // delta_x bx2 - bx1 = 55-30 =  25 à 50 cm
  // le carré de  delta_y , delta_x engendre le déplacement 20 , 20 °
  // le rapport de conversion angle / pixel est donc de 20/25 en y et 20/25 en x
  int noir = 255;
  int ix_noir, iy_noir , ind_noir;
  int ind2;
if (debug_1 ==1) {
  Serial.print(" le point de depart est : ");
  Serial.print(ind2);
  Serial.print(" :  ");
  Serial.print(by1);
  Serial.print(" , ");
  Serial.println(bx1);
}
  for(int i = by1; i < by2; i++) {
    ind2 = (i-1) * w + bx1 ; // decalage d'un rang : w = 80
    for(int j = bx1; j < bx2; j++) {
     if (debug_1 ==1) {
     //  Coordonnees du point courant le plus noir en pixel      
     Serial.print(" le point courant : ");
     Serial.print(ind2);
     Serial.print(" noir = ");
     Serial.print(downscaled[ind2]);
     Serial.print("  : x = ");
     Serial.print(i);
     Serial.print(" , y =  ");
     Serial.println(j);
     }
      // recherche du plus noir
      if ( noir > downscaled[ind2]){
        noir = downscaled[ind2];
        iy_noir = i;
        ix_noir = j;
        ind_noir = ind2;
      }
      if (debug_1 ==1) {
      //  Coordonnees du point courant par ligne le plus noir en pixel      
      Serial.print(" le plus noir est : ");
      Serial.print(noir);
      Serial.print(" en : ");
      Serial.print(ind_noir);
      Serial.print("  : y = ");
      Serial.print(iy_noir);
      Serial.print(" , x =  ");
      Serial.println(ix_noir);
      }
       ind2++;
    }
  }
     if (debug_1 ==1) {
     // Coordonnees du point le plus noir dans la fenetre en pixel 
      Serial.print(" le plus noir est : ");
      Serial.print(noir);
      Serial.print(" en : ");
      Serial.print(ind_noir);
      Serial.print("  : y = ");
      Serial.print(iy_noir);
      Serial.print(" , x =  ");
      Serial.println(ix_noir);  
  }
  // la on a le y, x du plus noir sous forme de coordonnées en pixel
  // il faut transformer les y,x de pixel en angles pour le Pan-Tilt
  // il faut et il suffit de connaitre deux points.
  // On choisit les deux coins opposés  
  // le coin haut droite     est en by1 ,bx1  = 19 , 30 en pixel et 49 , 13 en angle
  int angle_pan_1 = 49;   // by1 = 19
  int angle_til_1 = 13;   // bx1 = 30
  // le coin en bas à gauche est en by2 ,bx2  = 44 , 55 en pixel et 69 , -7 en angle  
  int angle_pan_2 = 69;   // by2 = 44
  int angle_til_2 = -7;   // bx2 = 55
  // attention comme la photo est tournée d'un quart de tour à gauche, 
  //        le Pan  qui correspond à l'axe horiontale, varie suivant Y dans le tableau downscaled
  //                 à Y croissant de 19 à 44 correspond Pan    croissant de 49 à 69
  //        le Tilt qui correspond à l'axe verticale , varie suivant X dans le tableau downscaled
  //                 à X croissant de 30 à 50 correspond Tilt décroissant de 13 à -7
  // règle de trois étendue :
  float rapport_y = float (float (angle_pan_1 - angle_pan_2) / float (by1 - by2));
  float rapport_x = float (float (angle_til_1 - angle_til_2) / float (bx1 - bx2));  
    angle_pan = angle_pan_1 - rapport_y * (by1 - iy_noir) ; 
    angle_til = angle_til_1 - rapport_x * (bx1 - ix_noir); 

    // envoi des commandes vers le Pan-Tilt en direct
    // pwm.setPWM(PAN_ID,  0, angleToPulse(angle_pan));
    // pwm.setPWM(TILT_ID, 0, angleToPulse(angle_til)); 
    // delay (500);
 
   // envoi des angles du point le plus noir avec le préfixe "1," vers le PAN-Tilt par RxTx
   String angles = "1," + String(angle_pan) + "," + String(angle_til) + " , noir : " + String(noir) + " iy_noir = " + String(iy_noir) + " ix_noir = " + String(ix_noir) + " photo " + String(pictureNumber);
   Serial.println(angles);  // envoi des angles vers le PAN-Tilt par RxTx
   delay (2000);
 if (debug_1 ==3) {   
   angles = "2," + String(angle_pan) + "," + String(angle_til) + " , noir : " + String(noir) + " iy_noir = " + String(iy_noir) + " ix_noir = " + String(ix_noir) + " photo " + String(pictureNumber); 
   angles = angles + " ; angle_pan_1 " + String(angle_pan_1) + " < angle_pan " + String(angle_pan)+ " < angle_pan_2 " + String(angle_pan_2) + "; angle_til_1 " + String(angle_til_1)+ " > angle_til " + String(angle_til)  + " > angle_til_2 " + String(angle_til_2) ;
   angles = angles + " by1 " + String(by1) + " by2 " + String(by2) + " bx1 " + String(bx1) + " bx2 " + String(bx2) ;
   angles = angles + " rapport_y " + String(rapport_y) + " rapport_x " + String(rapport_x);
   Serial.println(angles);  // envoi des angles vers le PAN-Tilt par RxTx
   delay (500);
      }
}

void save() {
  File imageFile = SPIFFS.open("/capture.jpg", "wb");
  uint8_t quality = 30;

  eloquent::io::print_all("The image will be saved as /capture.jpg");
  jpegWriter.write(imageFile, frame->buf, PIXFORMAT, quality);

  eloquent::io::print_all("Saved");

  // Save picture to microSD card

  //Path where new picture will be saved in SD Card
  String path = "/picture" + String(pictureNumber) +".jpg";
 // Serial.printf("Picture file name: %s\n", path.c_str());

  fs::FS &fs = SD_MMC;
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  }
  else {
   // file.write(frame->buf, frame->len); // payload (image), payload length
    file.write(imageFile);
 //   Serial.printf("Saved file to path: %s\n", path.c_str());
  }
  file.close();
  imageFile.close();
  pictureNumber++;
  delay(100);
}
void stream() {
    eloquent::io::print_all("START OF FRAME");
    jpegWriter.write(Serial, frame->buf, PIXFORMAT, 30);
    eloquent::io::print_all("END OF FRAME");
}
void stream_downscaled() {
    eloquent::io::print_all("START OF DOWNSCALED");
    eloquent::io::print_array(downscaled, w * h);
    eloquent::io::print_all("END OF DOWNSCALED");
   //   pour ecrire le fichier a exploiter avec excel
}
void initMicroSDCard(){
  // Start Micro SD card
 // Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
 //   Serial.println("SD Card Mount Failed");
    return;
  } else {
 //   Serial.println("SD Card Mount OK");
  }
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
 //    Serial.println("No SD Card attached");
    return;
  } else {
 //   Serial.println("SD Card attached OK");
  }
}
void takeSavePhoto(String path){
  // Take Picture with Camera
  camera_fb_t  * fb = esp_camera_fb_get();
  if(!fb) {
  //  Serial.println("Camera capture failed");
    return;
  }
  // Save picture to microSD card
  fs::FS &fs = SD_MMC;
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
  //  Serial.println("Failed to open file in writing mode");
  }
  else {
    uint8_t *jpeg;
    size_t len;
    if (fmt2jpg(frame->buf, W * H, W, H, PIXFORMAT, 30, &jpeg, &len)) {
      file.write(jpeg, len);
 //     Serial.printf("Saved file to path: %s\n", path.c_str());
      free(jpeg);
    }
    else {
   //   Serial.println("Not saved");
    }
  }
  file.close();
  //return the frame buffer back to the driver for reuse
  esp_camera_fb_return(fb);
}
/*
int angleToPulse(int ang){
   int pulse = map(ang,00, 180, SERVOMIN,SERVOMAX);// map angle of 0 to 180 to Servo min and Servo max
   Serial.print("Angle: ");Serial.print(ang);
   Serial.print(" pulse: ");Serial.println(pulse);
   return pulse;
}
*/
static esp_err_t save_downscaled_numbered(){
 // Serial.print("ecrire downscaled => SD  ");
 // Serial.println(pictureNumber);
  char *filename = (char*)malloc(21 + sizeof(pictureNumber));
  sprintf(filename, "/sdcard/downscaled_%d.txt", pictureNumber);
 // Serial.println(filename);
  FILE *file = fopen(filename, "w");
  if (file != NULL)  {
    int ii=0;
      for (int i = 0; i < h; i++) {
      for (int j = 0; j < w; j++) {  
        ii ++; 
          char x_buffer[20];
          dtostrf(downscaled[ii], 3,0, x_buffer);
      fprintf(file,x_buffer);
      fprintf(file,";"); 
      }
      fprintf(file,"\n"); 
       }
       }  
       else  {
    Serial.println("Could not open file");
  }
  fclose(file);
  free(filename);
}
