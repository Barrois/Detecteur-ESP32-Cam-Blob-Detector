# Test de passage de paramètres via le port série

## Utilisation de la virgule en tant que délimiteur

Afin de pouvoir indiquer les angles X (pan) et Y (tilt) aux deux servos,

nous projetons d'écrire ces coordonnées sur le port série depuis l'ESP32 vers l'Arduino.


Example pour X à 125° et Y à 145°
````
Serial.println("125,145")
````

## Passage d'un id de commande

Pour permettre un ajout d'autres paramètres (dans le futur), ou d'une autre action.

Nous ajoutons en premier paramètre un id de commande, la valeur 1 sera "déplacement des servos"

ainsi l'ESP32 enverra en premier la valeur 1, pour un ordre type "déplacement de servos":

Example pour X à 125° et Y à 145°
````
Serial.println("1,125,145")
````

## Tester depuis le moniteur série

Une fois le sketch chargé, il est possible de tester en indiquant dans le moniteur série, dans le prompt en haut de la fenêtre la chaîne de caractère suivante, suivi d'une validation par la touche Entrée:

````
1,24,67
````

le message en retour devrait être:

````
captured String is : 1,24,67
command = 1 / angleX = 24 / integerAngleX = 24
integerCommand = 1 / angleY = 67 / integerAngleY = 67
Deplacement du servo !
````