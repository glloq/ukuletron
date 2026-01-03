#include "ukuleleString.h"
#include "Arduino.h"
#include "pcaDevices.h"

UkuleleString::UkuleleString(int servoPin, int baseMidiNote, int numFrets, int angle,
                             const FretMapping* fretMapping, int stringIndex)
    : baseMidiNote(baseMidiNote), numFrets(numFrets), angleZero(angle),
      fretMapping(fretMapping), stringIndex(stringIndex)
{
    if (DEBUG) {
        Serial.print("ukuleleString--creation objet UkuleleString pour corde ");
        Serial.println(stringIndex);
    }

    // Initialisation des états pour chaque frette
    for (int i = 0; i < numFrets; i++) {
        active[i] = false;
        inHoldPhase[i] = false;
        extinctionTime[i] = 0;
        holdTransitionTime[i] = 0;

        // Initialiser le canal PWM à 0 (OFF)
        uint8_t pcaIdx = fretMapping[i].pcaIndex;
        uint8_t channel = fretMapping[i].channel;
        pcaDevices[pcaIdx].setPWM(channel, 0, 0);
    }

    playing = false;
    servoMovingToA = true;
    currentMidiNote = -1;

    // Initialisation du servo
    servo.attach(servoPin);
    servo.write(angleZero - ANGLE_GRATTAGE);
    delay(1000);
    servo.write(angleZero);
}

void UkuleleString::playNote(int note, bool isNoteOn) {
    int fretIndex = note - baseMidiNote; // 0 correspond à la corde à vide

    if (DEBUG) {
        Serial.print("ukuleleString--Corde ");
        Serial.print(stringIndex);
        Serial.print(" - Note :");
        Serial.print(note);
        Serial.print(" - ");
        Serial.println(isNoteOn ? "on" : "off");
    }

    if (isNoteOn) {
        currentMidiNote = note;
        if (fretIndex == 0) { // open string
            // Enregistrer que la corde est en cours d'utilisation même en open string
            playing = true;
            pluck();
        } else {
            // Pour une note jouée sur une frette, active le solénoïde correspondant
            activateFret(fretIndex - 1);
            delay(20);  // Délai pour laisser le solénoïde se positionner
            pluck();
        }
    } else {
        mute();
        if (fretIndex != 0) {
            desactivateFret(fretIndex - 1);
        }
        // Réinitialisation de l'état de la corde
        playing = false;
        currentMidiNote = -1;
    }
}

void UkuleleString::update() {
    if (playing) {
        for (int i = 0; i < numFrets; i++) {
            if (active[i]) {
                // Vérifier si transition attaque → maintien nécessaire
                if (!inHoldPhase[i] && millis() > holdTransitionTime[i]) {
                    transitionToHold(i);
                }

                // Vérifier si désactivation automatique nécessaire (timeout)
                if (millis() > extinctionTime[i]) {
                    desactivateFret(i);
                }
            }
        }
    }
}

int UkuleleString::getCurrentMidiNote() {
    return currentMidiNote;
}

bool UkuleleString::isPlayable(int midiNote) {
    if (!playing) {
        if (midiNote >= baseMidiNote && midiNote <= baseMidiNote + numFrets) {
            return true;
        }
    }
    return false;
}

void UkuleleString::activateFret(int fretIndex) {
    uint8_t pcaIdx = fretMapping[fretIndex].pcaIndex;
    uint8_t channel = fretMapping[fretIndex].channel;

    // Récupérer le duty cycle d'attaque pour cette corde
    uint16_t attackDuty = ATTACK_DUTY_BY_STRING[stringIndex];

    if (DEBUG) {
        Serial.print("ukuleleString--Activation frette ");
        Serial.print(fretIndex);
        Serial.print(" - PCA ");
        Serial.print(pcaIdx);
        Serial.print(" - Canal ");
        Serial.print(channel);
        Serial.print(" - Attack PWM: ");
        Serial.println(attackDuty);
    }

    // Phase d'attaque : PWM à la valeur d'attaque (typiquement 100%)
    pcaDevices[pcaIdx].setPWM(channel, 0, attackDuty);

    // Mise à jour des états
    active[fretIndex] = true;
    inHoldPhase[fretIndex] = false;
    playing = true;

    // Programmer la transition vers le maintien après ATTACK_PHASE_DURATION ms
    holdTransitionTime[fretIndex] = millis() + ATTACK_PHASE_DURATION;

    // Programmer la désactivation automatique après MAX_ACTIVATION_TIME
    extinctionTime[fretIndex] = millis() + MAX_ACTIVATION_TIME;
}

void UkuleleString::transitionToHold(int fretIndex) {
    uint8_t pcaIdx = fretMapping[fretIndex].pcaIndex;
    uint8_t channel = fretMapping[fretIndex].channel;

    // Récupérer le duty cycle de maintien pour cette corde
    uint16_t holdDuty = HOLD_DUTY_BY_STRING[stringIndex];

    if (DEBUG) {
        Serial.print("ukuleleString--Transition vers maintien - Frette ");
        Serial.print(fretIndex);
        Serial.print(" - Hold PWM: ");
        Serial.println(holdDuty);
    }

    // Phase de maintien : PWM réduit (typiquement 30-50%)
    pcaDevices[pcaIdx].setPWM(channel, 0, holdDuty);

    // Marquer comme étant en phase de maintien
    inHoldPhase[fretIndex] = true;
}

void UkuleleString::desactivateFret(int fretIndex) {
    uint8_t pcaIdx = fretMapping[fretIndex].pcaIndex;
    uint8_t channel = fretMapping[fretIndex].channel;

    if (DEBUG) {
        Serial.print("ukuleleString--Désactivation frette ");
        Serial.println(fretIndex);
    }

    // Désactiver le PWM (0% duty cycle)
    pcaDevices[pcaIdx].setPWM(channel, 0, 0);

    // Réinitialiser les états
    active[fretIndex] = false;
    inHoldPhase[fretIndex] = false;
    playing = false;
}

void UkuleleString::pluck() {
    if (servoMovingToA) {
        servoGoToA();
    } else {
        servoGoToB();
    }
    servoMovingToA = !servoMovingToA;
}

void UkuleleString::servoGoToA() {
    servo.write(angleZero + ANGLE_GRATTAGE);
}

void UkuleleString::servoGoToB() {
    servo.write(angleZero - ANGLE_GRATTAGE);
}

void UkuleleString::mute() {
    servo.write(angleZero);
}
