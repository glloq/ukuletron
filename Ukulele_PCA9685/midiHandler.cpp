#include "midiHandler.h"
#include "Arduino.h"

MidiHandler::MidiHandler(Ukulele &ukulele) : _ukulele(ukulele) {
    if (DEBUG) {
        Serial.println("midiHandler--creation");
    }
}

void MidiHandler::readMidi() {
    midiEventPacket_t midiEvent;
    do {
        midiEvent = MidiUSB.read();
        if (midiEvent.header != 0) {
            processMidiEvent(midiEvent);
        }
    } while (midiEvent.header != 0);
}

void MidiHandler::processMidiEvent(midiEventPacket_t midiEvent) {
    byte messageType = midiEvent.byte1 & 0xF0;
    byte channel = midiEvent.byte1 & 0x0F;
    byte note = midiEvent.byte2;
    byte velocity = midiEvent.byte3;

    if (messageType == 0x90 && velocity > 0) { // Note On
        _ukulele.playMidiNote(note, velocity);
    } else if (messageType == 0x80 || (messageType == 0x90 && velocity == 0)) { // Note Off
        _ukulele.stopMidiNote(note);
    }
}