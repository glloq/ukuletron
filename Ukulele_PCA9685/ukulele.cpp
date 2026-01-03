#include "ukulele.h"
#include "Arduino.h"

Ukulele::Ukulele() {
    for (int i = 0; i < 4; i++) {
        if (DEBUG) {
            Serial.print("ukulele--creation corde :");
            Serial.println(i);
        }
        _strings[i] = new UkuleleString(
            SERVO_PINS[i],
            OPEN_STRING_NOTE[i],
            NUM_FRETS[i],
            SERVO_CENTER_ANGLE[i],
            STRING_FRET_MAPPINGS[i],  // Passage du mapping spécifique à la corde
            i  // Indice de la corde pour accéder aux paramètres PWM
        );
    }
}

void Ukulele::initMusic(){
    if (DEBUG) {
        Serial.println("ukulele-- init music start serial");
    }
    // Parcours de chaque corde pour jouer une série de notes (test)
    for (int i = 0; i < 4; i++) {
        for (int j = 1; j < NUM_FRETS[i]; j++) {
            _strings[i]->playNote(OPEN_STRING_NOTE[i] + j, true);
            delay(300);
            _strings[i]->playNote(OPEN_STRING_NOTE[i] + j, false);
            delay(300);
        }
    }
}

void Ukulele::playMidiNote(int midiNote, int velocity) {
    int stringIndex = findString(midiNote);
    if (stringIndex != -1) {
        _strings[stringIndex]->playNote(midiNote, true);
    }
}

void Ukulele::stopMidiNote(int midiNote) {
    for (int i = 0; i < 4; i++) {
        if (_strings[i]->getCurrentMidiNote() == midiNote) {
            _strings[i]->playNote(midiNote, false);
            break;
        }
    }
}

int Ukulele::findString(int midiNote) {
    for (int i = 3; i >= 0; i--) {
        if (_strings[i]->isPlayable(midiNote)) {
            return i;
        }
    }
    return -1;
}

void Ukulele::update() {
    // Mise à jour de toutes les cordes (gestion transition PWM et timeouts)
    for (int i = 0; i < 4; i++) {
        _strings[i]->update();
    }
}
