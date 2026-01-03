#ifndef UKULELESTRING_H
#define UKULELESTRING_H

#include "Arduino.h"
#include "Servo.h"
#include "Wire.h"
#include "settings.h"
#include "pcaDevices.h"  // Pour accéder aux pcaDevices globaux (PCA9685)

class UkuleleString {
public:
    // Constructeur : reçoit la broche du servo, la note à vide, le nombre de frettes,
    // l'angle central, le tableau de mapping des frettes, et l'indice de la corde
    UkuleleString(int servoPin, int baseMidiNote, int numFrets, int angle,
                  const FretMapping* fretMapping, int stringIndex);

    // Joue ou arrête une note (note = MIDI Note)
    void playNote(int note, bool isNoteOn);

    // Mise à jour (transition attaque→maintien et désactivation automatique)
    void update();

    // Retourne la note MIDI actuellement jouée
    int getCurrentMidiNote();

    // Vérifie si la note MIDI peut être jouée sur cette corde
    bool isPlayable(int midiNote);

private:
    Servo servo;
    bool playing;
    int angleZero;
    bool servoMovingToA;
    int currentMidiNote;
    int baseMidiNote;
    int numFrets;
    int stringIndex;  // Indice de la corde (0-3) pour accéder aux paramètres PWM
    const FretMapping* fretMapping; // Tableau de mapping pour cette corde

    long extinctionTime[9];    // Timeout pour désactivation automatique
    long holdTransitionTime[9]; // Temps de transition attaque → maintien
    bool active[9];             // État d'activation de chaque frette
    bool inHoldPhase[9];        // true si en phase de maintien, false si en phase d'attaque

    void activateFret(int fretIndex);
    void desactivateFret(int fretIndex);
    void transitionToHold(int fretIndex);  // Nouvelle méthode pour transition PWM
    void pluck();
    void mute();
    void servoGoToA();
    void servoGoToB();
};

#endif
