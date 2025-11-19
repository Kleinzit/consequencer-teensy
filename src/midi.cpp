#include "midi.h"
#include <Arduino.h>

#ifdef USB_MIDI_SERIAL
void MIDI::playNote(int midiNote) {
    usbMIDI.sendNoteOn(midiNote, VELOCITY, MIDI_CHANNEL);
}

void MIDI::stopNote(int midiNote) {
    usbMIDI.sendNoteOff(midiNote, 0, MIDI_CHANNEL);
}
#else
// Stub implementations for non-USB-MIDI builds
void MIDI::playNote(int midiNote) {
    // No USB MIDI available
}

void MIDI::stopNote(int midiNote) {
    // No USB MIDI available
}
#endif
