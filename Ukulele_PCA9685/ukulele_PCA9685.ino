#include "settings.h"
#include "pcaDevices.h"
#include "ukulele.h"
#include "midiHandler.h"
#include "Arduino.h"

Ukulele* ukulele = nullptr;
MidiHandler* midiHandler = nullptr;

void setup() {
    Serial.begin(115200);

    while (!Serial) {
        delay(10); // Attendre que la connexion série soit établie
    }

    Serial.println("====================================");
    Serial.println("  UKULELE MIDI USB - Version PCA9685");
    Serial.println("  Contrôle PWM des solénoïdes");
    Serial.println("====================================");

    // Initialiser le bus I2C (Wire)
    Wire.begin();
    Wire.setClock(400000); // I2C Fast Mode (400kHz) pour réduire la latence

    Serial.println("setup--Initialisation I2C (400kHz)");

    // Initialiser les modules PCA9685
    initPCADevices();

    // Créer l'objet Ukulele (initialise les cordes)
    Serial.println("setup--Création objet Ukulele");
    ukulele = new Ukulele();

    // Créer le gestionnaire MIDI
    Serial.println("setup--Création MidiHandler");
    midiHandler = new MidiHandler(*ukulele);

    // Test des cordes (joue une gamme sur chaque corde)
    Serial.println("setup--Test des cordes (initMusic)");
    ukulele->initMusic();

    Serial.println("====================================");
    Serial.println("  Prêt à recevoir des notes MIDI!");
    Serial.println("====================================");
}

void loop() {
    midiHandler->readMidi();  // Gestion des messages MIDI
    ukulele->update();        // Gestion transitions PWM + extinction électroaimants
}
