#pragma once
#include "config.h"

class MIDI {
public:
    static void playNote(int midiNote);
    static void stopNote(int midiNote);
};
