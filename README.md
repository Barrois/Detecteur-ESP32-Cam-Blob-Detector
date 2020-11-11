# Detecteur-ESP32-Cam
Détection et prise de photo du frelon asiatique en vol stationnaire

Après analyse des fréquences émises lors du vol stationnaire du frelon devant une ruche, 
nous avons mis au point un filtre FFT qui donne satisfaction puisqu'il a permit de détecter des frelons dans 5 sites différents en France durant tout l'été 2020 : Limoux, Limoges, Toulouse, Nice et Caen. 
Afin de valider le filtre, je l'ai monté sur un ESP32-Cam pour faire un photo de l'intrus. 
Maintenant l'analyse de la photo et la détermination de la position du frelon devant la ruche est en cours
Puis nous transmettrons les coordonnées à un Pan-Tilt guidé par un Arduino et équipé d'un laser pour déclencher une riposte.

Il y a deux solutions en cours de  recherche :

- 1 -  Un ESP32-CAM détecte acoustiquement le Frelon asiatique, prend la photo, l'analyse et transmet les coordonées à un arduino qui  anime un Pan-Tilt équipé d'un laser .

- 2 - un EPS32 équipé d'un shield OV7670 et d'un Shield SD ,  détecte le frelon , prend la photo, l'analyse et anime le Pan-Tilt équipé du laser.   

Ce qui est fait : la détection, la prise de photo, et une partie de l'analyse 

Ce qui reste à faire :
- garder la photo en mémoire SPIFFS pour gagner du temps
- diminuer la résolution de la photo pour faciliter l'analyse
-  déterminer la position X-Y à partir de la résolution X-Pixel, Y-Pixel
- envoyer et recevoir les coordonnées  de l'ESP32-CAM à l'Arduino via Tx-Rx
- déclencher le tir du laser  10W alimenté en 12v 5A !!!
- sauver la photo avec les coordonnées du tir pour la postérité .
