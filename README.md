# Detecteur-ESP32-Cam : Blob Detector

- Détection et prise de photo du frelon asiatique en vol stationnaire en moins d'une seconde.

- Localisation d'un Blob dans une photo JPEG/jpg en 300ms pour une résolution 320x240.

Après analyse des fréquences émises lors du vol stationnaire du frelon devant une ruche, 
nous avons mis au point un filtre FFT qui donne satisfaction puisqu'il a permit de détecter des frelons dans 5 sites différents en France durant l'été 2020 : Limoux, Limoges, Toulouse, Nice et Caen. 

Afin de valider le filtre acoustique, nous l'avons monté sur un ESP32-Cam pour faire un photo de l'intrus, c'est la détection qui dure moins d'une seconde.
L'analyse de la photo et la localisation du frelon sur la photo est réalisée en moins de 300ms en direct par l'ESP32-CAM.

Le principe de cette analyse de la photo est basé sur le contraste et la taille du blob:
La photo est prise depuis le dessus de la ruche en plongée devant un écran blanc.
Les frelons, les abeilles et les résidus parsèment la photo et le pixel ne plus noir n'est pas forcement la cible.
Par contre en agglomérant des blocs de 4x4 pixels, on obtient des blobs dont le plus noir est le frelon : plus gros qu'une abeille et plus noir qu'une feuille.

Les coordonnées seront transmises à un Pan-Tilt guidé par un Arduino et équipé d'un laser pour déclencher une riposte.

Il y a deux solutions en cours de  recherche :

- 1 -  Un ESP32-CAM détecte acoustiquement le Frelon asiatique, prend la photo en 320x240, l'analyse et transmet les coordonées à un arduino qui  anime un Pan-Tilt équipé d'un laser . Actuellement c'est la solution explorée, la détection prends moins d'une seconde et la localisation 300ms.

- 2 - un EPS32 équipé d'un shield OV7670 et d'un Shield SD ,  détecte le frelon , prend la photo en 80x60, l'analyse et anime le Pan-Tilt équipé du laser.   

Ce qui est fait : la détection, la prise de photo, l'analyse et la localisation en pixel dans la phot

La détection par FFT et la sauvegarde sur carte SD :  ESP_CAM_timelapse_detector_Etienne_0.ino 

L'analyse en partant des photos mises en mémoires SPIFFS et la localisation du Blob : decoder_jpeg_08_1.ino 

La prise de photo et l'analyse de blob grace à l'excellent EloquentArduino : https://github.com/eloquentarduino/EloquentArduino
et son : https://eloquentarduino.github.io/2020/01/motion-detection-with-esp32-cam-only-arduino-version/
Une détection d'un blob 4x4 dans une photo 320x240 en 180ms !!! = : ESP32CameraNaiveMotionDetection_1.ino

Ce qui reste à faire :
- envoyer et recevoir les coordonnées  de l'ESP32-CAM à l'Arduino via Tx-Rx
- déclencher le tir du laser 10W alimenté en 12v 5A !!!
- sauver la photo avec les coordonnées du tir pour la postérité.
