#ifndef SETTINGS_H
#define SETTINGS_H
#include <stdint.h>

// ----------------------
// Paramètres généraux
// ----------------------
#define DEBUG 1

// Broches des servomoteurs (Arduino Micro Pro, par exemple)
constexpr int SERVO_PINS[4] = { 5, 6, 9, 10 };
constexpr int ANGLE_GRATTAGE = 15; // angle de déplacement du servo pour le grattage

// ----------------------
// Gestion des PCA9685 (PWM pour solénoïdes)
// ----------------------
constexpr int NUM_PCA_DEVICES = 2;        // On utilise 2 PCA9685 (16 canaux chacun)
constexpr int MAX_OUTPUTS_PER_PCA = 16;   // Maximum de sorties PWM par PCA9685

// Adresses I2C des PCA9685
constexpr uint8_t PCA_ADDRESSES_GLOBAL[NUM_PCA_DEVICES] = {0x40, 0x41};

// ----------------------
// Paramètres PWM
// ----------------------
constexpr int PWM_FREQUENCY = 500;        // Fréquence PWM en Hz (recommandé: 500-1000 Hz)
constexpr int PWM_RESOLUTION = 4096;      // Résolution 12-bit du PCA9685

// Duty cycles par défaut (valeurs sur 4096)
// Ces valeurs sont configurables selon vos besoins
constexpr uint16_t DEFAULT_ATTACK_DUTY = 4095;   // 100% - Phase d'attaque initiale
constexpr uint16_t DEFAULT_HOLD_DUTY = 1638;     // 40% - Phase de maintien

// Timing de la phase d'attaque (en millisecondes)
constexpr int ATTACK_PHASE_DURATION = 40;  // Durée de la phase d'attaque à 100%

// ----------------------
// Configuration PWM par corde (PERSONNALISABLE)
// ----------------------
// Vous pouvez ajuster ces valeurs pour adapter la force de frappe
// et le maintien selon chaque corde ou position de frette

// Duty cycle d'attaque par corde (0-4095, 4095 = 100%)
constexpr uint16_t ATTACK_DUTY_BY_STRING[4] = {
    4095,  // Corde 1 - Force maximale (100%)
    4095,  // Corde 2 - Force maximale (100%)
    3686,  // Corde 3 - Force réduite à 90% (moins de tension)
    3277   // Corde 4 - Force réduite à 80% (corde la plus fine)
};

// Duty cycle de maintien par corde (0-4095)
constexpr uint16_t HOLD_DUTY_BY_STRING[4] = {
    1638,  // Corde 1 - 40% de maintien
    1638,  // Corde 2 - 40% de maintien
    1434,  // Corde 3 - 35% de maintien
    1229   // Corde 4 - 30% de maintien (moins de force nécessaire)
};

// Option: Duty cycle variable par frette (optionnel, avancé)
// Décommentez et adaptez si vous voulez une force différente selon la frette
// Par exemple, première frette = plus de force, dernière frette = moins de force
/*
constexpr uint16_t ATTACK_DUTY_BY_FRET[12] = {
    4095, // Frette 0 (corde à vide - non utilisé)
    4095, // Frette 1 - Force maximale
    4095, // Frette 2
    3891, // Frette 3 - Commence à réduire
    3686, // Frette 4
    3482, // Frette 5
    3277, // Frette 6
    3072, // Frette 7
    2867, // Frette 8
    2663, // Frette 9
    2458, // Frette 10
    2253  // Frette 11
};
*/

// ----------------------
// Mapping des solénoïdes (frettes) pour chaque corde
// ----------------------

// Structure de mapping associant une frette à un PCA et un canal
struct FretMapping {
    uint8_t pcaIndex; // Indice du PCA (0 ou 1)
    uint8_t channel;  // Canal PWM sur le PCA (0 à 15)
};

// Corde 1 (9 solénoïdes) : toutes sur PCA0, canaux 0 à 8
constexpr FretMapping STRING1_MAPPING[9] = {
    {0, 0}, {0, 1}, {0, 2}, {0, 3}, {0, 4}, {0, 5}, {0, 6}, {0, 7}, {0, 8}
};

// Corde 2 (9 solénoïdes) : 7 sur PCA0 (canaux 9 à 15) et 2 sur PCA1 (canaux 0 et 1)
constexpr FretMapping STRING2_MAPPING[9] = {
    {0, 9}, {0, 10}, {0, 11}, {0, 12}, {0, 13}, {0, 14}, {0, 15}, {1, 0}, {1, 1}
};

// Corde 3 (8 solénoïdes) : toutes sur PCA1, canaux 2 à 9
constexpr FretMapping STRING3_MAPPING[8] = {
    {1, 2}, {1, 3}, {1, 4}, {1, 5}, {1, 6}, {1, 7}, {1, 8}, {1, 9}
};

// Corde 4 (6 solénoïdes) : toutes sur PCA1, canaux 10 à 15
constexpr FretMapping STRING4_MAPPING[6] = {
    {1, 10}, {1, 11}, {1, 12}, {1, 13}, {1, 14}, {1, 15}
};

// Tableaux récapitulatifs pour utilisation dans le code
constexpr const FretMapping* STRING_FRET_MAPPINGS[4] = {
    STRING1_MAPPING,
    STRING2_MAPPING,
    STRING3_MAPPING,
    STRING4_MAPPING
};

constexpr int NUM_FRETS[4] = {9, 9, 8, 6}; // Nombre de solénoïdes (frettes) par corde

// ----------------------
// Autres paramètres
// ----------------------
constexpr int MAX_ACTIVATION_TIME = 5000; // Durée maximale d'activation d'un électroaimant (ms)
constexpr bool WAIT_DESACTIVATE = true;   // Attente après réception d'un note off
constexpr int TIME_WAIT_DESACTIVATE = 200;  // Délai en ms

constexpr int OPEN_STRING_NOTE[4] = {67, 60, 64, 69};   // Note MIDI à vide pour chaque corde
constexpr int SERVO_CENTER_ANGLE[4] = {130, 90, 100, 95}; // Angle central des servos

// ----------------------
// GUIDE DE CALIBRATION PWM
// ----------------------
//
// Pour ajuster les valeurs PWM selon vos solénoïdes :
//
// 1. ATTACK_DUTY (phase d'attaque) :
//    - Commencez à 100% (4095)
//    - Si trop fort/bruyant : réduisez progressivement (3686=90%, 3277=80%)
//    - Minimum recommandé : 70% (2867) pour garantir une frappe rapide
//
// 2. HOLD_DUTY (phase de maintien) :
//    - Commencez à 40% (1638)
//    - Si la corde ne reste pas appuyée : augmentez (50%=2048, 60%=2458)
//    - Si trop chaud après 30s : réduisez (30%=1229, 25%=1024)
//    - Testez avec des notes longues (> 5 secondes)
//
// 3. ATTACK_PHASE_DURATION :
//    - Par défaut : 40ms
//    - Si la frappe est molle : augmentez (50ms, 60ms)
//    - Si vibration audible : réduisez (30ms, 25ms)
//
// 4. PWM_FREQUENCY :
//    - Par défaut : 500 Hz (bon compromis)
//    - Si bruit audible : augmentez (800Hz, 1000Hz)
//    - Si perte de force : réduisez (300Hz, 400Hz)
//
// 5. Test de température :
//    - Activez un solénoïde pendant 2 minutes
//    - Température acceptable : < 50°C
//    - Si > 60°C : réduisez HOLD_DUTY
//

#endif
