# Detecteur-ESP32-Cam
Détection et prise de photo du frelon asiatique en vol stationnaire

Après analyse des fréquences émises lors du vol stationnaire du frelon devant une ruche, 
nous avons mis au point un filtre FFT qui donne satisfaction puisqu'il a permit de détecter des frelons dans 5 sites différents en France durant l'été 2020 : Limoux, Limoges, Toulouse, Nice et Caen. 
Afin de valider le filtre acoustique, nous l'avons monté sur un ESP32-Cam pour faire un photo de l'intrus. 
L'analyse de la photo et la localisation du frelon sur la photo est réalisée en moins de 300ms en direct par l'ESP32-CAM.
Le principe de cette analyse de la photo est basé sur le contraste : La photo est prise depuis le dessus de la ruche en plongée sur un écran blanc.
Les frelons, les abeilles et les residus parsèment la photo. Le pixel ne plus noirs n'est pas forcement la cible.
Par contre en agglomérant des blocs de 4x4 pixels, on obtient des "blobs" dont le plus noir est le frelon plus gros qu'une abeille. 
Les coordonnées sont transmises à un Pan-Tilt guidé par un Arduino et équipé d'un laser pour déclencher une riposte.

Il y a deux solutions en cours de  recherche :

- 1 -  Un ESP32-CAM détecte acoustiquement le Frelon asiatique, prend la photo en 320x240, l'analyse et transmet les coordonées à un arduino qui  anime un Pan-Tilt équipé d'un laser . Actuellement c'est la solution explorée, la détection prends moins d'une seconde et la localisation 300ms.

- 2 - un EPS32 équipé d'un shield OV7670 et d'un Shield SD ,  détecte le frelon , prend la photo en 80x60, l'analyse et anime le Pan-Tilt équipé du laser.   

Ce qui est fait : la détection, la prise de photo, et une partie de l'analyse 

La détection par FFT et la sauvegarde sur car SD :  ESP_CAM_timelapse_detector_Etienne_0.ino 
L'analyse et la localisation : decoder_jpeg_07_4.ino 

Ce qui reste à faire :
- garder la photo en mémoire SPIFFS pour gagner du temps
- envoyer et recevoir les coordonnées  de l'ESP32-CAM à l'Arduino via Tx-Rx
- déclencher le tir du laser 10W alimenté en 12v 5A !!!
- sauver la photo avec les coordonnées du tir pour la postérité.
