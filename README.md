# Detecteur-ESP32-Cam
Détection et prise de photo du frelon asiatique en vol stationnaire

Après analyse des fréquences émises lors du vol stationnaire du frelon devant une ruche, 
nous avons mis au point un filtre FFT qui donne satisfaction puisqu'il a permit de détecter des frelons dans 5 sites différents en France durant tout l'été 2020 : Limoux, Limoges, Toulouse, Nice et Caen. 
Afin de valider le filtre, je l'ai monté sur un ESP32-Cam pour faire un photo de l'intrus. 
Maintenant l'analyse de la photo et la détermination de la position du frelon devant la ruche est en cours
Puis nous transmettrons les coordonnées à un Pan-Tilt guidé par un Arduino et équipé d'un laser pour déclencher une riposte.



