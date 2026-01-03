// pcaDevices.cpp
#include "pcaDevices.h"
#include "Arduino.h"

// Définition des objets PCA9685 globaux
Adafruit_PWMServoDriver pcaDevices[NUM_PCA_DEVICES];

void initPCADevices() {
    if (DEBUG) {
        Serial.println("pcaDevices--Initialisation des PCA9685...");
    }

    for (int i = 0; i < NUM_PCA_DEVICES; i++) {
        // Créer l'objet avec l'adresse I2C correspondante
        pcaDevices[i] = Adafruit_PWMServoDriver(PCA_ADDRESSES_GLOBAL[i]);

        // Initialiser le module
        if (!pcaDevices[i].begin()) {
            if (DEBUG) {
                Serial.print("pcaDevices--ERREUR: Impossible d'initialiser PCA9685 #");
                Serial.print(i);
                Serial.print(" à l'adresse 0x");
                Serial.println(PCA_ADDRESSES_GLOBAL[i], HEX);
            }
            // Continuer malgré l'erreur pour permettre le debugging
        } else {
            if (DEBUG) {
                Serial.print("pcaDevices--PCA9685 #");
                Serial.print(i);
                Serial.print(" initialisé à l'adresse 0x");
                Serial.println(PCA_ADDRESSES_GLOBAL[i], HEX);
            }
        }

        // Configurer la fréquence PWM (identique pour tous les canaux d'un PCA9685)
        pcaDevices[i].setPWMFreq(PWM_FREQUENCY);

        if (DEBUG) {
            Serial.print("pcaDevices--Fréquence PWM configurée à ");
            Serial.print(PWM_FREQUENCY);
            Serial.println(" Hz");
        }

        // Initialiser tous les canaux à 0 (OFF)
        for (int channel = 0; channel < 16; channel++) {
            pcaDevices[i].setPWM(channel, 0, 0);
        }
    }

    if (DEBUG) {
        Serial.println("pcaDevices--Initialisation PCA9685 terminée");
    }
}
