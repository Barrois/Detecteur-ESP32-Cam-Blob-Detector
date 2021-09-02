# Déviation du bus I2C de l'Esp32-Cam avec deux plaques FPC 24P 0.5mm




Utilisation de headers mâles 2.54mm

![Plaques_FPC_24P_0.5mm.jpg](Plaques_FPC_24P_0.5mm.jpg?raw=true "Plaques_FPC_24P_0.5mm.jpg")


Les deux plaques FPC 24P 0.5mm assemblées:


![Plaques assemblées](Plaques_assembl%C3%A9es.jpg?raw=true "Deux plaques FPC assemblées")


Les plaques, la nappe, la caméra et l'ESP32-Cam


![Les plaques, la nappe, la caméra et l'ESP32-Cam](Plaques_FPC_assemblees_ESP32-Cam.jpg?raw=true "Les plaques, la nappe, la caméra et l'ESP32-Cam")

Les connecteurs pour l'I2C:


![schéma SDA et SCL](FPC-24P_0.5mm__deviation_I2C_Camera.jpg?raw=true "schéma SDA et SCL")


## Exemple d'utilisation du bus I2C: scanner

```
#include <Wire.h>

const int PIN_SDA = 26;  // port 26 du esp32-cam sur le SDA directement sur la puce
const int PIN_SCL = 27;  // port 27 du esp32-cam sur le SCL directement sur la puce
void setup()
{
  Serial.begin(115200);
  Wire.begin(PIN_SDA, PIN_SCL);
}

void loop()
{
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.print(address,HEX);
      Serial.println("  !");
      nDevices++;
    }
    else if (error==4) {
      Serial.print("Unknow error at address 0x");
      if (address<16){
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
  }
  else {
    Serial.println("done\n");
  }
  delay(5000);           // wait 5 seconds for next scan
}
```