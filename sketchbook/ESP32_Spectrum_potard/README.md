# Détecteur acoustique de frelon pour ESP8266

L'objectif est de faire un détecteur bas coût avec la possibilité d'ajuster la détection en fonction du frelon asiatique.
En effet le poids et la taille du frelon asiatique varient avec l'avancée de la saison et l'abondance de la nourriture,
ce qui entraine des variations du pic de détection qu'il faut pouvoir régler in situ, d'ou ce montage avec un potentiomètre

## Pré requis

- Installation de la carte ESP32 dans l'IDE Arduino (cf: https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/)
- Installation de la librairie ArduinoFFT (https://github.com/kosme/arduinoFFT) depuis le getionnaire de librairies de l'IDE Arduino


## Raccordements

- le potentiomètre de règlage de la valeur du pic de fréquence est relié au GPIO 32, ADC1_CH4 (cf: potardPin 32)

- le micro (ici type MAX9814) est relié sur le GPIO 33, ADC1_CH5 (cf: microPIN 33)

## Détection


Le potentiomètre permet de varier de 100 à 1000 Hz la fréquence de détection.

cf ligne 74, 4095 étant la valeur maximale de l'ADC, 100 correspond à 0 et 1000 à 4095:
```
int range = map(adc0, 0, 4095, 100, 1000);
```

la féquence de référence devrait se situer aux alentours de 230.


Il semblerait toutefois que les harmoniques soient plus facilement détectables.

Sans avoir vraiment d'explication, la fréquence 460 fonctionne mieux avec l'esp8266 que 230, et 920 pour l'ESP32.

À vérifier en "conditions réelles".