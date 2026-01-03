# Ukuletron - Version PCA9685 avec contrôle PWM

## 📋 Vue d'ensemble

Cette version du code utilise des modules **PCA9685** pour contrôler les solénoïdes avec modulation PWM, permettant :

✅ **Réduction thermique** : -40°C en température d'équilibre
✅ **Économie d'énergie** : -58% sur notes longues
✅ **Force de frappe ajustable** : Paramètres PWM configurables par corde
✅ **Stratégie "holding"** : Frappe puissante (100%) puis maintien doux (30-50%)
✅ **Durabilité accrue** : Moins de stress thermique sur les solénoïdes

---

## 🔧 Matériel requis

### Composants principaux

| Composant | Quantité | Notes |
|-----------|----------|-------|
| Arduino Leonardo/Micro | 1× | Compatible MIDI USB |
| PCA9685 PWM Driver | 2× | 16 canaux chacun (32 total) |
| Solénoïdes 5V 7N | 32× | Force recommandée |
| MOSFET N-Channel | 32× | IRF540N, IRLZ44N ou équivalent |
| Diode de roue libre | 32× | 1N4007 ou 1N5819 (Schottky) |
| Résistances 1kΩ | 32× | Protection gate MOSFET |
| Servomoteurs | 4× | Pour grattage des cordes |
| Alimentation 5V | 1× | **4-5A minimum** (upgrade de 2A) |

### Adresses I2C

```
PCA9685 #1 : 0x40 (canaux 0-15)
PCA9685 #2 : 0x41 (canaux 0-15)
```

---

## 🔌 Schéma de câblage

### Circuit driver solénoïde (répété 32×)

```
PCA9685                MOSFET               Solénoïde
┌────────┐            ┌─────┐              ┌─────────┐
│        │            │     │              │         │
│ Canal  ├────[1kΩ]───┤ G   │              │         │
│  PWM   │            │     D├──────────────┤+        │
│        │            │     │              │         │
└────────┘            │   S │              │         │
                      └─────┘              └─────────┘
                         │                      │
                        GND    ┌─────────┐      │
                               │ Diode   │      │
                               │  1N4007 │      │
                               └─────────┘      │
                                    │           │
                                    └───────────┘
                                         │
                                        +5V
```

**Légende** :
- **G** = Gate du MOSFET (commande PWM via résistance 1kΩ)
- **D** = Drain (connecté au +5V via solénoïde)
- **S** = Source (connecté à GND)
- **Diode** : Cathode (barre) vers +5V, Anode vers D

### Connexions I2C

```
Arduino          PCA9685 #1         PCA9685 #2
SDA (Pin 2) ─────┬─ SDA ─────────────── SDA
SCL (Pin 3) ─────┼─ SCL ─────────────── SCL
                 │
                GND

Alimentation :
  +5V ───────────┬─ VCC (PCA #1) ─────── VCC (PCA #2)
  GND ───────────┴─ GND (PCA #1) ─────── GND (PCA #2)
```

**⚠️ Important** :
- Utilisez des câbles courts pour I2C (< 20cm idéalement)
- Ajoutez des condensateurs 100µF sur VCC/GND de chaque PCA9685
- Utilisez une alimentation **commune** pour Arduino et solénoïdes

---

## 📚 Bibliothèques Arduino requises

### Installation via le gestionnaire de bibliothèques

1. **Adafruit PWM Servo Driver Library**
   ```
   Sketch > Include Library > Manage Libraries
   Rechercher : "Adafruit PWM Servo Driver"
   Installer la dernière version
   ```

2. **MIDIUSB**
   ```
   Rechercher : "MIDIUSB"
   Installer la version officielle Arduino
   ```

3. **Wire** (incluse par défaut avec Arduino)

### Vérification des bibliothèques

Votre sketch doit inclure :
```cpp
#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <MIDIUSB.h>
#include <Servo.h>
```

---

## ⚙️ Configuration des paramètres PWM

### Fichier : `settings.h`

#### Paramètres PWM globaux

```cpp
constexpr int PWM_FREQUENCY = 500;         // Fréquence PWM (Hz)
constexpr int ATTACK_PHASE_DURATION = 40;  // Durée phase d'attaque (ms)
```

#### Duty cycles par corde (personnalisables)

**Phase d'attaque** (frappe initiale) :
```cpp
constexpr uint16_t ATTACK_DUTY_BY_STRING[4] = {
    4095,  // Corde 1 - 100% (forte)
    4095,  // Corde 2 - 100%
    3686,  // Corde 3 - 90% (plus légère)
    3277   // Corde 4 - 80% (la plus fine)
};
```

**Phase de maintien** (holding) :
```cpp
constexpr uint16_t HOLD_DUTY_BY_STRING[4] = {
    1638,  // Corde 1 - 40%
    1638,  // Corde 2 - 40%
    1434,  // Corde 3 - 35%
    1229   // Corde 4 - 30%
};
```

### Table de conversion duty cycle

| % | Valeur (0-4095) | Usage typique |
|---|-----------------|---------------|
| 100% | 4095 | Attaque maximale |
| 90% | 3686 | Attaque réduite |
| 80% | 3277 | Attaque douce |
| 50% | 2048 | Holding fort |
| 40% | 1638 | **Holding recommandé** |
| 30% | 1229 | Holding léger |
| 25% | 1024 | Holding minimal |

---

## 🎯 Procédure de calibration

### Étape 1 : Test de force (duty cycle d'attaque)

1. **Téléverser le code** avec valeurs par défaut
2. **Lancer initMusic()** au démarrage (test automatique)
3. **Écouter le son** :
   - Son clair et net → ✅ Force correcte
   - Buzz/vibration → ⚠️ Réduire ATTACK_DUTY (essayer 90% = 3686)
   - Son étouffé → ⚠️ Augmenter ATTACK_DUTY (si < 100%)
4. **Ajuster** dans `settings.h` si nécessaire

### Étape 2 : Test thermique (duty cycle de maintien)

1. **Jouer une note longue** (> 30 secondes)
2. **Mesurer la température** du solénoïde (thermomètre IR ou toucher)
   - < 40°C → ✅ Excellent
   - 40-50°C → ✅ Acceptable
   - 50-60°C → ⚠️ Chaud, réduire HOLD_DUTY de 5-10%
   - > 60°C → 🔥 Trop chaud, réduire HOLD_DUTY de 20%

3. **Tester le maintien** :
   - Corde bien appuyée → ✅ OK
   - Corde relâchée (buzz) → ⚠️ Augmenter HOLD_DUTY de 5%

### Étape 3 : Ajustement fin

**Si bruit audible (vibration PWM)** :
```cpp
constexpr int PWM_FREQUENCY = 800;  // Augmenter à 800-1000 Hz
```

**Si transition trop rapide** (frappe molle) :
```cpp
constexpr int ATTACK_PHASE_DURATION = 60;  // Augmenter à 50-80 ms
```

**Si transition trop lente** (vibration audible) :
```cpp
constexpr int ATTACK_PHASE_DURATION = 30;  // Réduire à 25-35 ms
```

---

## 📖 Structure du code

### Fichiers principaux

```
Ukulele_PCA9685/
├── ukulele_PCA9685.ino      # Sketch principal Arduino
├── settings.h                # Configuration (PWM, pins, mappings)
├── pcaDevices.h/cpp          # Gestion des PCA9685
├── ukuleleString.h/cpp       # Contrôle d'une corde (PWM)
├── ukulele.h/cpp             # Orchestration des 4 cordes
├── midiHandler.h/cpp         # Réception MIDI USB
└── README_PCA9685.md         # Ce fichier
```

### Flux de contrôle PWM

```
Note MIDI ON
     ↓
activateFret()
     ├─→ setPWM(ATTACK_DUTY)  ─── Phase attaque (40ms, 100%)
     └─→ Démarrer timer
                ↓
           millis() > 40ms ?
                ↓
     transitionToHold()
           setPWM(HOLD_DUTY)  ─── Phase maintien (40%, jusqu'à Note OFF)
                ↓
Note MIDI OFF
     ↓
desactivateFret()
     └─→ setPWM(0)  ─────────── Extinction
```

### Différences clés avec version MCP23017

| Aspect | MCP23017 (ancien) | PCA9685 (nouveau) |
|--------|-------------------|-------------------|
| Contrôle | Digital ON/OFF | PWM 12-bit |
| Puissance | 100% constant | 100% → 40% |
| Température | 80°C (30s) | 47°C (30s) |
| Force ajustable | ❌ Non | ✅ Oui (par corde) |
| Code | `digitalWrite()` | `setPWM()` |
| Fichiers modifiés | settings.h, ukuleleString.cpp | + pcaDevices.h/cpp |

---

## 🚀 Installation et téléversement

### Étape 1 : Préparation

1. **Cloner/télécharger** le dossier `Ukulele_PCA9685/`
2. **Installer les bibliothèques** (voir section précédente)
3. **Vérifier le câblage** (PCA9685 + MOSFETs + solénoïdes)

### Étape 2 : Configuration

1. **Ouvrir** `settings.h`
2. **Adapter** les mappings si nécessaire :
   ```cpp
   constexpr int NUM_FRETS[4] = {9, 9, 8, 6};  // Votre configuration
   ```
3. **Ajuster** les angles servos si déjà calibrés :
   ```cpp
   constexpr int SERVO_CENTER_ANGLE[4] = {130, 90, 100, 95};
   ```

### Étape 3 : Téléversement

1. **Connecter** l'Arduino Leonardo via USB
2. **Ouvrir** `ukulele_PCA9685.ino` dans l'IDE Arduino
3. **Sélectionner** : `Tools > Board > Arduino Leonardo`
4. **Sélectionner** : `Tools > Port > (votre port)`
5. **Téléverser** (Ctrl+U ou bouton →)

### Étape 4 : Test

1. **Ouvrir** le Moniteur série (115200 baud)
2. **Observer** les messages de debug :
   ```
   ====================================
     UKULELE MIDI USB - Version PCA9685
     Contrôle PWM des solénoïdes
   ====================================
   setup--Initialisation I2C (400kHz)
   pcaDevices--PCA9685 #0 initialisé à l'adresse 0x40
   pcaDevices--PCA9685 #1 initialisé à l'adresse 0x41
   ```
3. **Écouter** le test automatique (`initMusic()`)
4. **Envoyer** des notes MIDI via USB

---

## 🐛 Dépannage

### Problème : "Impossible d'initialiser PCA9685"

**Causes possibles** :
- Mauvais câblage I2C (SDA/SCL inversés)
- Adresse I2C incorrecte
- Alimentation insuffisante

**Solutions** :
1. Vérifier les connexions SDA/SCL
2. Scanner les adresses I2C avec un sketch de test :
   ```cpp
   #include <Wire.h>
   void setup() {
     Wire.begin();
     Serial.begin(115200);
     for (byte addr = 1; addr < 127; addr++) {
       Wire.beginTransmission(addr);
       if (Wire.endTransmission() == 0) {
         Serial.print("Device found at 0x");
         Serial.println(addr, HEX);
       }
     }
   }
   ```
3. Vérifier que VCC = 5V et GND commun

### Problème : Solénoïdes ne s'activent pas

**Causes possibles** :
- MOSFET mal câblé
- Diode de roue libre inversée
- Duty cycle trop faible

**Solutions** :
1. Vérifier le câblage MOSFET (Gate ← PWM, Drain ← Solénoïde+, Source ← GND)
2. Vérifier la diode (cathode vers +5V)
3. Augmenter temporairement ATTACK_DUTY à 4095 (100%)

### Problème : Surchauffe persistante

**Solutions** :
1. Réduire HOLD_DUTY (essayer 25% = 1024)
2. Réduire MAX_ACTIVATION_TIME à 3000ms (settings.h:156)
3. Vérifier l'alimentation (sous-tension → surcourant)
4. Ajouter dissipateurs thermiques (plaque alu)

### Problème : Bruit/vibration audible

**Solutions** :
1. Augmenter PWM_FREQUENCY à 800-1000 Hz
2. Vérifier les connexions mécaniques (visserie desserrée)
3. Réduire ATTACK_PHASE_DURATION si vibration en début de note

---

## 📊 Monitoring et debug

### Activer les messages debug

Dans `settings.h` :
```cpp
#define DEBUG 1  // 1 = activé, 0 = désactivé
```

### Messages clés à surveiller

**Initialisation PCA9685** :
```
pcaDevices--PCA9685 #0 initialisé à l'adresse 0x40
pcaDevices--Fréquence PWM configurée à 500 Hz
```

**Activation frette** :
```
ukuleleString--Activation frette 2 - PCA 0 - Canal 2 - Attack PWM: 4095
```

**Transition holding** :
```
ukuleleString--Transition vers maintien - Frette 2 - Hold PWM: 1638
```

---

## 🎵 Utilisation avec MIDI

### Connexion MIDI

1. **Connecter** l'Arduino Leonardo via USB à votre ordinateur
2. **Sélectionner** "Arduino Leonardo" comme périphérique MIDI dans votre DAW
3. **Envoyer** des notes MIDI (canal 1 par défaut)

### Mapping MIDI

| Corde | Note ouverte | MIDI Note | Frettes |
|-------|--------------|-----------|---------|
| 1 (Sol) | G4 | 67 | 67-75 (9 frettes) |
| 2 (Do) | C4 | 60 | 60-68 (9 frettes) |
| 3 (Mi) | E4 | 64 | 64-71 (8 frettes) |
| 4 (La) | A4 | 69 | 69-74 (6 frettes) |

### Logiciels testés

✅ **Compatible** :
- Ableton Live
- FL Studio
- Reaper
- MuseScore (lecture MIDI)
- Hairless MIDI Serial (pour Serial MIDI)

---

## 📈 Optimisations avancées

### Réduire la latence I2C

Dans `ukulele_PCA9685.ino` :
```cpp
Wire.setClock(400000);  // Fast Mode (400kHz) - déjà activé par défaut
```

### PWM variable par frette (optionnel)

Décommentez dans `settings.h` :
```cpp
constexpr uint16_t ATTACK_DUTY_BY_FRET[12] = {
    4095, 4095, 3891, 3686, 3482, 3277, ...
};
```

Modifiez `ukuleleString.cpp:123` :
```cpp
uint16_t attackDuty = ATTACK_DUTY_BY_FRET[fretIndex];  // Au lieu de ATTACK_DUTY_BY_STRING
```

### Désactiver le test automatique

Dans `ukulele_PCA9685.ino:36`, commentez :
```cpp
// ukulele->initMusic();  // Désactivé pour démarrage rapide
```

---

## 📄 Licence

Ce projet est sous licence "je partage mon taf gratuitement si tu veux faire de l'argent dessus demande avant et on partage :D"

---

## 🙏 Crédits

- **Code original** : Projet Ukuletron (MCP23017)
- **Adaptation PWM** : Version PCA9685
- **Étude thermique** : `ETUDE_PCA9685_SOLENOIDES.md`
- **Bibliothèques** :
  - Adafruit PWM Servo Driver Library
  - Arduino MIDIUSB

---

## 📞 Support

Pour toute question ou problème :
1. Vérifier ce README
2. Consulter `ETUDE_PCA9685_SOLENOIDES.md` pour la théorie
3. Activer DEBUG et analyser les messages série
4. Ouvrir une issue sur le dépôt GitHub

**Bon ukulele automatisé ! 🎸🤖**
