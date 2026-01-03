#ifndef UKULELE_H
#define UKULELE_H

#include "settings.h"
#include "ukuleleString.h"
#include "Servo.h"

class Ukulele {
public:
    Ukulele();
    void playMidiNote(int midiNote, int velocity); // note On
    void stopMidiNote(int midiNote);               // note Off
    void update();                                 // mise à jour des états (transition PWM + désactivation automatique)
    void initMusic();                              // test de jeu sur chaque corde
private:
    int findString(int midiNote);                  // retourne l'indice de la corde jouable
    UkuleleString* _strings[4];                    // tableau des cordes
};

#endif
