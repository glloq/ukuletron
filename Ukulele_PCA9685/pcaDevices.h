#ifndef PCADEVICES_H
#define PCADEVICES_H

#include <Adafruit_PWMServoDriver.h>
#include "settings.h"

// Déclaration des objets PCA9685 globaux
extern Adafruit_PWMServoDriver pcaDevices[NUM_PCA_DEVICES];

// Fonction d'initialisation des PCA9685
void initPCADevices();

#endif
