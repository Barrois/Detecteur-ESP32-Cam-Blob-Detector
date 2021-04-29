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

#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
const int PIN_SDA = 1;  // port 1 du esp32-cam sur le SDA
const int PIN_SCL = 3;  // port 3 du esp32-cam sur le SCL

const int PAN_ID = 0;  // Pan  sur le port 0 du PCA9685
const int TILT_ID = 1; // Tilt sur le port 1 du PCA9685

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define SERVOMIN  150 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // this is the 'maximum' pulse length count (out of 4096)
  int angle_pan_1 =4;        // position des bords (inutile)
  int angle_pan_2 =36;
  int angle_til_1 =46;
  int angle_til_2 =78;

  int angle_pan_0 =21;       // position du milieu
  int angle_til_0 =61;

  int angle_pan , angle_til ; // position courante

unsigned long previousMillis = 0;// on stocke le temps de la derniere execution

const long interval = 5000; // intervale entre chaque execution (milliseconds)

void setup() {
  Serial.begin(115200);
  Serial.println(__FILE__);
  SPIFFS.begin(true);
  delay(100);
  Serial.println("Begin");

  //Initialize MicroSD
  Serial.print("Initializing the MicroSD card module... ");
  initMicroSDCard();

  Wire.begin(PIN_SDA, PIN_SCL);
  pwm.begin();
  pwm.setPWMFreq(60);

  camera.begin(FRAME_SIZE);

}

void loop() {
  pwm.setPWM(PAN_ID,  0, angleToPulse(angle_pan_1));
  pwm.setPWM(TILT_ID, 0, angleToPulse(angle_til_1));
  capture();
 //Path where new picture will be saved in SD Card
  path = "/picture" + String(pictureNumber) +".jpg";
  Serial.printf("Picture file name: %s\n", path.c_str());
  //Take and Save Photo
  takeSavePhoto(path);
  pictureNumber++;
  delay(100);

 // while(1);
}



void capture() {
  timeit("capture frame", frame = camera.capture());
  // scale image from size H * W to size h * w
  timeit("downscale", downscaler.downscale(frame->buf, downscaled));

  eloquent::io::print_all("START OF BERNARD");

  int ind1=0;
  Serial.println("tableau i = w , j = h");
  for(int i = 0; i < h; i++) {
    for(int j = 0; j < w; j++) {
      ind1 ++;
      Serial.print(downscaled[ind1]);
      Serial.print(";");
    }
    Serial.println();
  }
  // bornes de fenétrage
  int by1 = 22; //    9; 80/32*9
  int by2 = 47; //   19;
  int bx1 = 45; //   15;
  int bx2 = 60; //   20;
  int noir = 255;
  int ix_noir, iy_noir , ind_noir;
  int ind2;

  Serial.print(" le point de depart est : ");
  Serial.print(ind2);
  Serial.print(" :  ");
  Serial.print(by1);
  Serial.print(" , ");
  Serial.println(bx1);

  for(int i = by1; i < by2; i++) {
    ind2 = (i-1) * w + bx1 ; // decalage d'un rang
    for(int j = bx1; j < bx2; j++) {
    /*
     Serial.print(" le point courant : ");
     Serial.print(ind2);
     Serial.print(" noir = ");
     Serial.print(downscaled[ind2]);
     Serial.print("  : x = ");
     Serial.print(i);
     Serial.print(" , y =  ");
     Serial.println(j);
    */
      // recherche du plus noir
      if ( noir > downscaled[ind2]){
        noir = downscaled[ind2];
        iy_noir = i;
        ix_noir = j;
        ind_noir = ind2;
      }
      Serial.print(" le plus noir est : ");
      Serial.print(noir);
      Serial.print(" en : ");
      Serial.print(ind_noir);
      Serial.print("  : y = ");
      Serial.print(iy_noir);
      Serial.print(" , x =  ");
      Serial.println(ix_noir);

      ind2++;
    }
  }
  // la on a le y, x du plus noir
  // il faut transformer les y,x en angle pour le Pan-Tilt
  // point central = y,x = 25,30 => pan, tilt = 21,61
  // y = - 32/52 +   x = + 32/52 pour une photo en  80x60

  angle_pan  = angle_pan_0 - (32/52)* iy_noir ;
  angle_til = angle_til_0 +- (32/52)* ix_noir ;
  Serial.print("       Pan   angle " );
  Serial.println(angle_pan);
  Serial.print("      Tilt   angle " );
  Serial.println(angle_til);
  pwm.setPWM(PAN_ID,  0, angleToPulse(angle_pan));
  pwm.setPWM(TILT_ID, 0, angleToPulse(angle_til));
  delay (500);
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
  Serial.printf("Picture file name: %s\n", path.c_str());

  fs::FS &fs = SD_MMC;
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  }
  else {
   // file.write(frame->buf, frame->len); // payload (image), payload length
    file.write(imageFile);
    Serial.printf("Saved file to path: %s\n", path.c_str());
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
  Serial.println("Starting SD Card");
  if(!SD_MMC.begin()){
    Serial.println("SD Card Mount Failed");
    return;
  } else {
    Serial.println("SD Card Mount OK");
  }
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    Serial.println("No SD Card attached");
    return;
  } else {
    Serial.println("SD Card attached OK");
  }
}

void takeSavePhoto(String path){
  // Take Picture with Camera
  camera_fb_t  * fb = esp_camera_fb_get();

  if(!fb) {
    Serial.println("Camera capture failed");
    return;
  }

  // Save picture to microSD card
  fs::FS &fs = SD_MMC;
  File file = fs.open(path.c_str(), FILE_WRITE);
  if(!file){
    Serial.println("Failed to open file in writing mode");
  }
  else {
    uint8_t *jpeg;
    size_t len;
    if (fmt2jpg(frame->buf, W * H, W, H, PIXFORMAT, 30, &jpeg, &len)) {
      file.write(jpeg, len);
      Serial.printf("Saved file to path: %s\n", path.c_str());
      free(jpeg);
    }
    else {
      Serial.println("Not saved");
    }

  }
  file.close();

  //return the frame buffer back to the driver for reuse
  esp_camera_fb_return(fb);
}

int angleToPulse(int ang){
   int pulse = map(ang,00, 180, SERVOMIN,SERVOMAX);// map angle of 0 to 180 to Servo min and Servo max
   Serial.print("Angle: ");Serial.print(ang);
   Serial.print(" pulse: ");Serial.println(pulse);
   return pulse;
}
