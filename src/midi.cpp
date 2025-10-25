#include "midi.h"
#include <Arduino.h>

void MIDI::playNote(int midiNote) {
    usbMIDI.sendNoteOn(midiNote, VELOCITY, MIDI_CHANNEL);
}

void MIDI::stopNote(int midiNote) {
    usbMIDI.sendNoteOff(midiNote, 0, MIDI_CHANNEL);
}
