// https://www.aranacorp.com/fr/gerez-plusieurs-arduino-avec-un-bus-i2c/
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
# define I2C_SLAVE_ADDRESS 11 // 12 pour l'esclave 2 et ainsi de suite

#define PAYLOAD_SIZE 2
const int PAN_ID = 0;   // Pan  (en bas)     sur le port 0 du PCA9685
const int TILT_ID = 1;  // Tilt (au dessus)  sur le port 1 du PCA9685 
const int servoPin = 2; // Gachette          sur le port 2 du PCA9685 
Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define SERVOMIN  150 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // this is the 'maximum' pulse length count (out of 4096)void setup()

void setup()
{
  Wire.begin(I2C_SLAVE_ADDRESS);
  Serial.begin(115200);  
  Serial.println(__FILE__);
   pinMode(A0, INPUT);   // commande du viseur en entrée     
   pinMode(A1, OUTPUT);  // relais de la vanne d'air => D1
   pinMode(LED_BUILTIN, OUTPUT); // pour faire beau
     
  // Arduino slave sur I2C
  Serial.println("----------------------I am Slave1");
  delay(1000);               
  Wire.onRequest(requestEvents);
  Wire.onReceive(receiveEvents);
   pwm.begin();
   pwm.setPWMFreq(60);  
}

void loop(){
      pwm.setPWM(PAN_ID,  0, angleToPulse(120) );
      pwm.setPWM(TILT_ID, 0, angleToPulse(120) );      
  for( int angle =60; angle<81; angle ++){ // 0 181
    delay(100);
    pwm.setPWM(PAN_ID, 0, angleToPulse(angle) );
    Serial.print("PAN  angle : ");
    Serial.println(angle);    
        delay(100);
    }
  for( int angle =120; angle>101; angle --){ // 0 181
    delay(100);
    pwm.setPWM(TILT_ID, 0, angleToPulse(angle) );
    Serial.print("Tilt  angle : ");
    Serial.println(angle);    
        delay(100);
  }
      delay(500);
  for( int angle =100; angle>81; angle --){ // 0 181
    delay(100);
    pwm.setPWM(PAN_ID, 0, angleToPulse(angle) );
    }
  for( int angle =100; angle<121; angle ++){ // 0 181
    delay(100);
    pwm.setPWM(TILT_ID, 0, angleToPulse(angle) );
  }     
    delay(500);
   int angle_pan = 80;
   int angle_tilt = 110;        
    Serial.print("      Tilt   angle1 " );
    Serial.println(angle_tilt);    
    pwm.setPWM(PAN_ID, 0, angleToPulse(angle_pan) );    
    pwm.setPWM(TILT_ID, 0, angleToPulse(angle_tilt) );
    Serial.print("PAN  angle : ");
    Serial.print(angle_pan);     
    Serial.print("  Tilt  angle : ");
    Serial.println(angle_tilt);   
     
   delay(500);
   while(1);
}

int angleToPulse(int ang){
   int pulse = map(ang,00, 180, SERVOMIN,SERVOMAX);// map angle of 0 to 180 to Servo min and Servo max 
   Serial.print("Angle: ");Serial.print(ang);
   Serial.print(" pulse: ");Serial.println(pulse);
   return pulse;
}
int n = 0;

void requestEvents()
{
  Serial.println(F("---> recieved request"));
  Serial.print(F("sending value : "));
  Serial.println(n);
  Wire.write(n);
}

void receiveEvents(int numBytes)
{  
  Serial.println(F("---> recieved events"));
  int n = Wire.read();/// int en plus
  Serial.print(numBytes);
  Serial.println(F("bytes recieved"));
  Serial.print(F("recieved value : "));
  Serial.println(n);
}
