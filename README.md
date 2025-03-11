> [!NOTE]
>  travail en cours => je refait le code et le support prorpement 

# Orchestrion-Ukulele

Ce projet utilise un Arduino Leonardo (ou micro) pour contrôler un ukulele acoustique à l'aide de signaux MIDI. 
L'Ukulele est équipé d'électroaimants/solenoides pour actionner les frettes et d'un servomoteur pour actionner le "grattage" sur chaque corde.
l'objectif est d'avoir un instrument fonctionnel (qui sonne correctement) qui est pas trop moche et qui est facilement maintenable !

Le contrôleur MIDI permet de jouer des notes recues par le cable USB, 
      

###### Ameliorations possible : 
  - utiliser un module  wifi ou bluetooth pour la communication midi => utiliser un eps32 et adapter le midiHandler pour fonctionner en sans fil
  - utiliser une prise midi classique => code a adapter 
  - ajouter un systeme de Led activé pendant les notes actives
  - utiliser des dissipateur thermique (support solenoides avec plaque alu) 
  - utiliser un decoupage PWM pour reduire l'intensité de frappe des solenoides et reduire la surchauffe
  - ajouter un systeme de mesure de temperature et de ventilation 

## schemas de principe 
![schema principe](https://raw.githubusercontent.com/glloq/Orchestrion_ukulele/main/img/schemas%20principev2.png?raw=true)
pour des raisons de reactivité, j'ai choisi d'utiliser des solenoides pour faire les accords sur chaque frette.
le design choisi obligera de faire des percages pour les tiges tranversant le manche, on en profitera pour retirer le manche et recoller la parties avec les frettes sur une planche => il faudra donc recuperer et fixer le corps et la tete du manche proprement !!

### Systeme de doigts
l'idée est d'utiliser des tiges en metal qui serons actionné par les solenoides, le dessous des doigts sera couvert d'une mousse ou d'un tissu pour limiter le bruits  
<img src="https://raw.githubusercontent.com/glloq/Orchestrion_ukulele/main/img/doigts.png" alt="Your image title" width=40% height=40%/>
> [!TIP]
> il est possible d'utiliser des serre cables pour rendre le systeme de doigts plus solide 

> [!WARNING]
> Le code actuel est fait pour 9 solenoides sur la premiere et seconde cordes, 8 sur la troisieme et 6 sur la quatrieme
> il est possible de changer cet arrangement en adaptant le fichier settings.h a votre cas d'utilisation

### Systeme de grattage

Pour des raions de simplicité nous tiliserons un servomoteur par corde pour alterner le grattage de la corde 
<img src="https://github.com/glloq/Orchestrion_ukulele/blob/main/img/grattage%20partsd.png" alt="Your image title" width=40% height=40%/>

il faut garder un design propre, il faudra donc cacher les mecanismes de grattage derriere le corps du ukulele ou une planche de bois sur les cotés.

### Schemas branchements
![schema electronique](https://github.com/glloq/Orchestrion_ukulele/blob/main/img/schemas.png?raw=true)

### Matériel requis
- Alimentation 5v 2A
- un cable USB pour l'arduino
- Arduino Leonardo(ou compatible) 
- Ukulele 12 frettes
- le suppport en bois ou autre 
- 4 servomoteurs pour le grattage des 4 cordes
- 2 MCP23017 : pour l'extension des pins de l'arduino
- 32 électroaimants; avec alimentation + fusible adapté + driver (mofset par exemple) + diode de roue libre pour chaque solenoides
  
> [!WARNING]
> eviter des solenoides avec un trop gros amperage pour limiter la chauffe.

> [!TIP]
> des solenoides de 7N fonctionent sur la plupart des frettes mais il faudra probablement prevoir un peut plus elevé pour la corde la plus epaisse sur la premiere frette !



## Installation logicielle

#### Bibliothèques requises

- [MidiUSB.H](https://github.com/arduino-libraries/MIDIUSB)
- [Adafruit_MCP23X17.h](https://github.com/adafruit/Adafruit-MCP23017-Arduino-Library)
- Arduino.h


## Options de configuration

Le fichier `Settings.h` contient plusieurs options de configuration pour personnaliser le fonctionnement du contrôleur Arduino Ukulele MIDI. 
Vous pouvez modifier ces options avant de téléverser le code sur votre Arduino:

- le nombre de frettes par cordes et le mcp utilisé
- le temps d'activation maximum de l'electroaimant en continu
- l'angle "zero" du servo de grattage par corde : c'est la position du servo contre la corde
- l'angle de grattage
- L'activation du debuggage via serial


#### Preparations
1. Ouvrez le fichier "ukulele-servo-init.ino" de ce dépôt
2. Sélectionnez le port série approprié et le type de carte dans le menu Outils de l'IDE Arduino.
3. Téléversez le code sur votre Arduino.
4. il faut rentrer l'angle du servo (entre 20 et 160) dans le moniteur serie et valider avec entré jusqu'a ce que le pick soit en contact avec la corde
5. noter la valeur de l'angle et passer à la corde suivante en envoyant "ok"
6. faire les meme manipulations que précedement avec les 3 autres servos 

#### Installation
1. Clonez ou téléchargez ce dépôt.
2. Ouvrez le fichier ukulele.ino dans l'IDE Arduino.
3. Installez la bibliothèque MIDIUSB et adafruit MCP23x17 si vous ne l'avez pas déjà fait.
4. faites les modifications necessaire à votre montage dans settings.h
5. Connectez votre Arduino Leonardo à votre ordinateur via un câble USB.
6. Sélectionnez le port série approprié et le type de carte dans le menu Outils de l'IDE Arduino.
7. Téléversez le code sur votre Arduino Leonardo.
8. Connectez votre Arduino à un hôte MIDI et profitez de votre ukulele contrôlé par MIDI !

## Licence

Ce projet est sous licence "je partage mon taf gratuitement si tu veut faire de l'argent dessus demande avant et on partage :D"
