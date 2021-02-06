/**
* Envoi de deux coordonnées sous la forme: 100,50
* suivi d'une validation par la touche Entrée
* les valeurs de 100 et de 50 seront converties en int
* pour pouvoir être utilisées pour orienter les servos
*
* Les GPIO 1 & 3 de la liaison série étant occupés
* par la liaison du bus I2C, nous utilisons une
* liaison série via Bluetooth
*/


#include "BluetoothSerial.h"
BluetoothSerial SerialBT;

String readString; // récupération de la chaîne: String

String angleX; // données String
String angleY; // données String

// Utilisées pour la conversion
int integerAngleX = 0;
int integerAngleY = 0;

// Délimiteur de fin de chaîne (retour chariot)
char endMarker = '\n';

int ind1; // emplacement du caractère , (virgule)
int ind2;


void setup() {

  Serial.begin(115200);
  Serial.println("Entree coordonnees avec delimiteur");

  // Définition du nom du périphérique Bluetooth
  SerialBT.begin("ESP32_VV_Detecteur");
}

void loop() {


  if (SerialBT.available())  {
    char c = SerialBT.read();  // gets one byte from serial buffer

    if (c == endMarker) {

      SerialBT.println();
      SerialBT.print("captured String is : ");
      SerialBT.println(readString); //prints string to serial port out

      ind1 = readString.indexOf(',');  //finds location of first ,
      angleX = readString.substring(0, ind1);   //captures first data String
      ind2 = readString.indexOf(',', ind1+1 );   //finds location of second ,
      angleY = readString.substring(ind1+1);

      integerAngleX = angleX.toInt();
      integerAngleY = angleY.toInt();

      SerialBT.print("angleX = ");
      SerialBT.print(angleX);
      SerialBT.print(" / integerAngleX = ");
      SerialBT.println(integerAngleX);
      SerialBT.print("angleY = ");
      SerialBT.print(angleY);
      SerialBT.print(" / integerAngleY = ");
      SerialBT.print(integerAngleY);

      SerialBT.println();
      SerialBT.println();

      readString=""; //clears variable for new input
      angleX="";
      angleY="";

    }
    else {
      readString += c; //makes the string readString
    }
  }
}
