//
// fonctionne sur un D1 mini pro
//
/**
* Envoi de deux coordonnées sous la forme: 1,100,50
* suivi d'une validation par la touche Entrée
*
* la première valeur 1 sera utilisé pour indiquer la
* commande à effectuer, si == moveServo alors ce sera
* un déplacement des servos
*
* les valeurs de 100 et de 50 seront converties en int
* pour pouvoir être utilisées pour orienter les servos
*
*/

String readString; // récupération de la chaîne: String

String command; // données String
String angleX; // données String
String angleY; // données String

const int moveServo = 1; // id commande de servo

// Utilisé pour indiquer la commande à effectuer
int integerCommand = 0;
// Utilisées pour la conversion
int integerAngleX = 59; // le milieu du champ
int integerAngleY = 4;  // le milieu du champ

// Délimiteur de fin de chaîne (retour chariot)
char endMarker = '\n';


int ind1; // emplacement du caractère , (virgule)
int ind2;
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

const int PAN_ID = 0;   // Pan  (en bas)     sur le port 0 du PCA9685
const int TILT_ID = 1;  // Tilt (au dessus)  sur le port 1 du PCA9685

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();
#define SERVOMIN  150 // this is the 'minimum' pulse length count (out of 4096)
#define SERVOMAX  600 // this is the 'maximum' pulse length count (out of 4096)

void setup() {
    Serial.begin(9600);
    Serial.println(__FILE__);
    pinMode(D5, OUTPUT);      // commande du viseur en sortie
      pinMode(LED_BUILTIN, OUTPUT);
   Serial.println("............ Servo test!");
   //Wire.begin(PIN_SDA, PIN_SCL);
   Wire.begin();
   pwm.begin();
   pwm.setPWMFreq(60);

        // on se positionne au milieu de la cible
    Serial.print("     Milieu  : angle X : " );
    Serial.print(integerAngleX);
    Serial.print(", angle Y :" );
    Serial.println(integerAngleY);
    delay(100);
    pwm.setPWM(PAN_ID, 0, angleToPulse(integerAngleX) );
    pwm.setPWM(TILT_ID, 0, angleToPulse(integerAngleY) );
    delay(100);
      // on allume le laser pour voir
           digitalWrite(D5, HIGH);
           digitalWrite(LED_BUILTIN, HIGH);
           Serial.println("...................on allume le laser" );
      delay(500);
     // on éteint le laser
           digitalWrite(D5, LOW );
           digitalWrite(LED_BUILTIN, LOW);
           Serial.println("...................on eteint le laser" );

}
void loop() {


  // on écoute l'Esp32-Cam
  //    Serial.println(" on ecoute Esp32-Cam ");

 if (Serial.available())  {
      char c = Serial.read();  // gets one byte from serial buffer

    if (c == endMarker) {

      Serial.println();
      Serial.print("captured String is : ");
      Serial.println(readString); //prints string to serial port out


      ind1 = readString.indexOf(',');  //finds location of first ,
      ind2 = readString.indexOf(',', ind1+1 );   //finds location of second ,

      command = readString.substring(0, ind1);//captures first data String
      angleX = readString.substring(ind1+1,ind2);
      angleY = readString.substring(ind2+1);

      integerCommand = command.toInt();
      integerAngleX = angleX.toInt();
      integerAngleY = angleY.toInt();

      Serial.print("command = ");
      Serial.print(command);
      Serial.print(" / angleX = ");
      Serial.print(angleX);
      Serial.print(" / integerAngleX = ");
      Serial.println(integerAngleX);
      Serial.print("integerCommand = ");
      Serial.print(integerCommand);
      Serial.print(" / angleY = ");
      Serial.print(angleY);
      Serial.print(" / integerAngleY = ");
      Serial.println(integerAngleY);

      if (integerCommand == moveServo) {
        Serial.println("Deplacement du servo !");
        // on tire sur la cible 
        Serial.print("     On tire sur la cible  : angle X : " );
        Serial.print(integerAngleX);
        Serial.print(", angle Y :" );
        Serial.println(integerAngleY);
        pwm.setPWM(PAN_ID, 0, angleToPulse(integerAngleX) );
        pwm.setPWM(TILT_ID, 0, angleToPulse(integerAngleY) );
        delay(500);
        // on allume le laser pour voir
        digitalWrite(D5, HIGH);
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("...................on allume le laser" );
        delay(2000);  // temps pour le tir
        // on éteint le laser
        digitalWrite(D5, LOW );
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("...................on eteint le laser" );

        //  on se positionne au milieu de la cible pour la photo suivante 
        Serial.print("     on tire au Milieu de la cible : " );
        Serial.print(59);
        Serial.print(", et :" );
        Serial.println(4);
        pwm.setPWM(PAN_ID, 0, angleToPulse(59) );
        pwm.setPWM(TILT_ID, 0, angleToPulse(4) );
        delay(500);
        // on allume le laser  
        digitalWrite(D5, HIGH);
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("...................on allume le laser" );
        delay(2000);   // temps pour la photo
        // on éteint le laser
        digitalWrite(D5, LOW );
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("...................on eteint le laser" );
      }

      Serial.println();

      readString=""; //clears variable for new input
      command="";
      angleX="";
      angleY="";

    }
    else {
      readString += c; //makes the string readString
    }
 }


     /*
// pour test
     // on allume le laser
           digitalWrite(D5, HIGH);
           digitalWrite(LED_BUILTIN, HIGH);
    Serial.println("...................on allume le laser" );
 //    Milieu  : angle X : 59, angle Y :4

   int a = 10; // delta horizontal
   int b = 10; // delta vertical
   int c = 1000;// duréee de pause au coin
   // talon
   int d = 59; // milieu horizontal
   int e =  4; // milieu vertical

    Serial.print("     Milieu  : angle X : " );
    int integerAngleX = d;
    int integerAngleY = e;
    Serial.print(integerAngleX);
    Serial.print(", angle Y :" );
    Serial.println(integerAngleY);
    pwm.setPWM(PAN_ID, 0, angleToPulse(integerAngleX) );
    pwm.setPWM(TILT_ID, 0, angleToPulse(integerAngleY) );
    delay(c);

   //  coin en haut gauche
    integerAngleX = d+a; // H  59 + 10 = 69
    integerAngleY = e+b; // V  4 +10  = 14
    Serial.print("      coin en haut gauche   " );
    Serial.println(integerAngleX);
    pwm.setPWM(PAN_ID, 0, angleToPulse(integerAngleX) );
    pwm.setPWM(TILT_ID, 0, angleToPulse(integerAngleY) );
    delay(c);

    integerAngleX = d-a;
    integerAngleY = e+b;
    Serial.print("      coin en haut  droit " );
    Serial.println(integerAngleX);
    pwm.setPWM(PAN_ID, 0, angleToPulse(integerAngleX) );
    pwm.setPWM(TILT_ID, 0, angleToPulse(integerAngleY) );
    delay(c);

    integerAngleX = d-a;
    integerAngleY = e-b;
    Serial.print("      coin en bas  droit  " );
    Serial.println(integerAngleX);
    pwm.setPWM(PAN_ID, 0, angleToPulse(integerAngleX) );
    pwm.setPWM(TILT_ID, 0, angleToPulse(integerAngleY) );
    delay(c);

    integerAngleX = d+a;
    integerAngleY = e-b;
    Serial.print("      coin en bas  gauche  " );
    Serial.println(integerAngleX);
    pwm.setPWM(PAN_ID, 0, angleToPulse(integerAngleX) );
    pwm.setPWM(TILT_ID, 0, angleToPulse(integerAngleY) );
    delay(c);

    delay(200);
           digitalWrite(LED_BUILTIN, LOW);

delay(2000);
        // on éteint le laser
           digitalWrite(D5, LOW );
           digitalWrite(LED_BUILTIN, HIGH);
       Serial.println("...................on eteint le laser" );
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);

    // on éteint le laser
           digitalWrite(D5, LOW );
           digitalWrite(LED_BUILTIN, HIGH);
       Serial.println("...................on eteint le laser" );
        delay(200);
        digitalWrite(LED_BUILTIN, LOW);
*/
 }
int angleToPulse(int ang){
   int pulse = map(ang,00, 180, SERVOMIN,SERVOMAX);// map angle of 0 to 180 to Servo min and Servo max
   Serial.print("Angle: ");Serial.print(ang);
   Serial.print(" pulse: ");Serial.println(pulse);
   return pulse;
}
