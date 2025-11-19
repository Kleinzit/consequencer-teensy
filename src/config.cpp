#include "config.h"

const int rowPins[ROWS] = {38, 40, 13, 14, 15, 17, 18, 19};
const int colPins[COLS] = {20, 21, 22, 23};

const int sequencerNotes[ROWS] = {60, 62, 64, 65, 67, 69, 71, 72}; // C, D, E, F, G, A, B, C
const char* noteNames[ROWS] = {"C4", "D4", "E4", "F4", "G4", "A4", "B4", "C5"};

// const unsigned long stepDuration = 250; // 120 BPM
const unsigned long debounceDelay = 50;
