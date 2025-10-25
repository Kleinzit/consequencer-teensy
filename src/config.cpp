#include "config.h"

const int rowPins[ROWS] = {0, 1, 2, 3};
const int colPins[COLS] = {4, 5, 6, 7, 8, 9, 10, 11};

const int sequencerNotes[ROWS] = {60, 64, 67, 71}; // C, E, G, B
const char* noteNames[ROWS] = {"C4", "E4", "G4", "B4"};

const unsigned long stepDuration = 250; // 120 BPM
const unsigned long debounceDelay = 50;
