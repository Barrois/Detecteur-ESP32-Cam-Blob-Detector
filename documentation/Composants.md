# Detecteur-ESP32-Cam

## Composants

- un ESP32-CAM

- un micro type Max981

- (une carte Sd)

- un interrupteur pour l'alimenation

- un interrupteur pour désactiver la sortie du micro au démarrage de l'ESP32 (port GPIO0 utilisé pour passer en mode flash)

![ESP32-CAM et micro Max981](ESP32-CAM_Max981.jpg?raw=true "ESP32-CAM et micro Max981")


## Montage

La sortie du Max981 est reliée sur l'entrée GIPO0 de l'ESP32-CAM.

Un interrupteur permet de déconnecter les deux au démarrage de l'ESP-32 afin qu'il ne passe pas en mode "flash".

## Idées de boitier "étanches"

exemple d'utilsation boîte de dérivation:


![Boitier_ESP32-CAM_micro_switchs](Boitier_ESP32-CAM_micro_switchs_power_et_mic.jpg?raw=true "Boitier_ESP32-CAM_micro_switchs")

![Boitier_ESP32-CAM_micro_switchs](Boitier_ESP32-CAM_micro_switchs_power_et_mic_inside.jpg?raw=true "Boitier_ESP32-CAM_micro_switchs")


## Autres info./outils

### Entrée audio pour ESP-32: schéma

l'idée serait de pouvoir jouer un enregistrement audio d'un vol de frelon afin de tester l'algo/filtre de détection

![ESP32_FFT_VU_entree_audio_jack](ESP32_FFT_VU_entree_audio_jack.jpg?raw=true "ESP32_FFT_VU_entree_audio_jack.jpg")

cf: https://github.com/s-marley/ESP32_FFT_VU/blob/master/Linein_bb.png