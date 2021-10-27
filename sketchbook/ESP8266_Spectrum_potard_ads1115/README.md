# Détecteur acoustique de frelon pour ESP8266

L'objectif est de faire un détecteur bas coût avec la possibilité d'ajuster la détection en fonction du frelon asiatique.
En effet le poids et la taille du frelon asiatique varient avec l'avancée de la saison et l'abondance de la nourriture,
ce qui entraine des variations du pic de détection qu'il faut pouvoir régler in situ, d'ou ce montage avec un potentiomètre

## Pré requis

- Installation de la carte ESP8266 dans l'IDE Arduino (cf: https://randomnerdtutorials.com/how-to-install-esp8266-board-arduino-ide/)
- Installation de la librairie ArduinoFFT (https://github.com/kosme/arduinoFFT) depuis le getionnaire de librairies de l'IDE Arduino
- Installation de la librairie Adafruit_ADS1X15 https://github.com/adafruit/Adafruit_ADS1X15

## Raccordements

- le micro est relié au seul ADC de l'esp8266, à savoir ADC0/A0
- le relais est relié sur la sortie D1, sauf que D1 est aussi SCL de l'I2C (D2 pour SDA)
il ne peut donc être enfiché directement sur les headers de l'ESP8266
- l'ESP8266 ne dispose que d'un ADC (A0), pour utiliser le potentiomètre de règlage de la valeur du pic de fréquence, nous utilisons un ADS1115 en I2C (D1 et D2)


## Schéma breadboard avec Fritzing


![Schema Breadboard](/sketchbook/ESP8266_Spectrum_potard_ads1115/ESP8266_Spectrum_bb.png "Schema Breadboard")


Le circuit de test sur breadboard :
![Circuit sur Breadboard](/sketchbook/ESP8266_Spectrum_potard_ads1115/ESP8266_potentiometre_ads11115_MAX9814.jpg "Circuit sur Breadboard")