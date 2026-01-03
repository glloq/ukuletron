#ifndef MIDIHANDLER_H
#define MIDIHANDLER_H

#include <MIDIUSB.h>
#include "Ukulele.h"
/***********************************************************************************************
----------------------------    MIDI message handler    ----------------------------------------
************************************************************************************************/
class MidiHandler {
  private:
    Ukulele& _ukulele;
    void processMidiEvent(midiEventPacket_t midiEvent);

  public:
    MidiHandler(Ukulele &ukulele);
    void readMidi();
};

#endif // MIDIHANDLER_H
