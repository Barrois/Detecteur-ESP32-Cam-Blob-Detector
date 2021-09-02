# Déviation du bus I2C de l'Esp32-Cam avec deux plaques FPC 24P 0.5mm




Utilisation de headers mâles 2.54mm

![Plaques_FPC_24P_0.5mm.jpg](Plaques_FPC_24P_0.5mm.jpg?raw=true "Plaques_FPC_24P_0.5mm.jpg")


Les deux plaques FPC 24P 0.5mm assemblées:


![Plaques assemblées](Plaques_assembl%C3%A9es.jpg?raw=true "Deux plaques FPC assemblées")


Les plaques, la nappe, la caméra et l'ESP32-Cam


![Les plaques, la nappe, la caméra et l'ESP32-Cam](Plaques_FPC_assemblees_ESP32-Cam.jpg?raw=true "Les plaques, la nappe, la caméra et l'ESP32-Cam")

Les connecteurs pour l'I2C:


![schéma SDA et SCL](FPC-24P_0.5mm__deviation_I2C_Camera.jpg?raw=true "schéma SDA et SCL")


## Exemple d'utilisation du bus I2C

```
#include <Wire.h>

const int PIN_SDA = 26;  // port 26 du esp32-cam sur le SDA directement sur la puce
const int PIN_SCL = 27;  // port 27 du esp32-cam sur le SCL directement sur la puce
void setup()
{

  Serial.begin(115200);
  Wire.begin(PIN_SDA, PIN_SCL);
}
```