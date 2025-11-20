#include "config.h"

const int rowPins[ROWS] = {38, 40, 13, 14, 15, 17, 18, 19};
const int colPins[COLS] = {20, 21, 22, 23};

const int sequencerNotes[3][ROWS] = {
    {60, 61, 62, 63, 64, 65, 66, 67},
    {68, 69, 70, 71, 72, 73, 74, 75},
    {68, 76, 77, 72, 78, 79, 80, 81}
};
const char* noteNames[ROWS] = {"C4", "C#4", "D4", "D#4", "E4", "F4", "F#4", "G4"};

const unsigned long stepDuration = 125;  // 120 BPM
const unsigned long debounceDelay = 50;
const unsigned long POLL_INTERVAL = 50;
