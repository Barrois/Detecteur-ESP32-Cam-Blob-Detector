# Detecteur-ESP32-Cam : Blob Detector Localisator

- Détection et prise de photo du frelon asiatique en vol stationnaire en moins d'une seconde.

- Localisation d'un Blob dans une photo JPEG/jpg en 180ms pour une résolution 320x240.

Après analyse des fréquences émises lors du vol stationnaire du frelon devant une ruche,
nous avons mis au point un filtre FFT qui donne satisfaction puisqu'il a permis de détecter des frelons dans 5 sites différents en France durant l'été 2020 : Limoux, Limoges, Toulouse, Nice et Caen.

Afin de valider le filtre acoustique, nous l'avons monté sur un ESP32-Cam pour faire un photo de l'intrus, c'est la détection qui dure moins d'une seconde.
L'analyse de la photo et la localisation du frelon sur la photo est réalisée en moins de 300ms en direct par l'ESP32-CAM.

Le principe de cette analyse de la photo est basé sur le contraste et la taille du blob:
La photo est prise depuis le dessus de la ruche en plongée devant un écran blanc.
Les frelons, les abeilles et les résidus parsèment la photo et le pixel ne plus noir n'est pas forcement la cible.
Par contre en agglomérant des blocs de 4x4 pixels, on obtient des blobs dont le plus noir est le frelon : plus gros qu'une abeille et plus noir qu'une feuille.

Les coordonnées seront transmises à un Pan-Tilt guidé par un Arduino et équipé d'un laser pour déclencher une riposte.

Il y a deux solutions en cours de  recherche :

- 1 -  Un ESP32-CAM détecte acoustiquement le Frelon asiatique, prend la photo en 320x240, l'analyse et transmet les coordonnées à un Arduino qui anime un Pan-Tilt équipé d'un laser. Actuellement c'est la solution explorée, la détection prend moins d'une seconde et la localisation 300ms.

- 2 - un ESP32 équipé d'un Shield OV7670 et d'un Shield SD, détecte le frelon, prend la photo en 80x60, l'analyse et anime le Pan-Tilt équipé du laser.

Ce qui est fait : la détection, la prise de photo, l'analyse et la localisation en pixel dans la phot

La détection par FFT et la sauvegarde sur carte SD : ESP_CAM_timelapse_detector_Etienne_0.ino

L'analyse en partant des photos mises en mémoires SPIFFS et la localisation du Blob : decoder_jpeg_08_1.ino

La prise de photo et l'analyse de blob grâce à l'excellent EloquentArduino : https://github.com/eloquentarduino/EloquentArduino
et son : https://eloquentarduino.github.io/2020/01/motion-detection-with-esp32-cam-only-arduino-version/
Une détection d'un blob 4x4 dans une photo 320x240 en 180ms !!! = : ESP32CameraNaiveMotionDetection_1.ino

Ce qui reste à faire :
- envoyer et recevoir les coordonnées  de l'ESP32-CAM à l'Arduino via Tx-Rx
- déclencher le tir du laser 10W alimenté en 12v 5A !!!
- sauver la photo avec les coordonnées du tir pour la postérité.

La logique actuellement est la suivante :
- quand on allume l'esp32-cam
   -0 il lit l'EEProm pour savoir à quel numéro de fichier il en est : xxx
   -1 il fait une localisation qu'il sauvegarde sur la carte SD : fichier LOCAxxx.txt

 Reste à faire : transmettre l'information au Pan-Tilt

   -2 il fait une photo sauvegardée sur la carte SD : fichier CAPTURExxx.jpg
   -3 il se met en position d'écoute sur le port 0
   -4 il fait la FFT
   -5 si le filtre de la FFT est positif, il commence à compter
   -6 si le compteur du filtre atteint 5 reconnaissances en moins d'une seconde
        il collecte les 5 FFT qui ont déclenchées l'alarme et les stocke sur la carte SD : fichier FFTxxx.txt
        puis l'esp32-cam redémarre pour faire la localisation et la photo
   -7 sinon il recommence en -4


Cette nouvelle version "ESP_CAM_detector_localisor_Patrick_0_1" (en hommage à Patrick DESSEAUX qui m'aide beaucoup)
marque un tournant dans la détection : après son analyse, seule la première harmonique est testée : 230Hz +/- 10%.
Et sans doute allons-nous modifier aussi le dispositif suivant son idée : un micro sur le devant de la ruche et un micro d'ambiance,
afin de supprimer les faux positifs dus à l'environnement (mobylette et autres..) et donc raccourcir le temps de détection.



Encore merci à tous
