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
int integerAngleX = 0;
int integerAngleY = 0;

// Délimiteur de fin de chaîne (retour chariot)
char endMarker = '\n';

int ind1; // emplacement du caractère , (virgule)
int ind2;


void setup() {

  Serial.begin(115200);
  Serial.println("Entree coordonnees avec delimiteur");
}

void loop() {


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
}
