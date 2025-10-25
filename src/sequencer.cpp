#ifdef MASTER_MODE

#include "sequencer.h"
#include "midi.h"
#include <Arduino.h>

void Sequencer::init() {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            sequencerGrid[row][col] = false;
        }
        activeNotes[row] = false;
    }
    currentStep = 0;
    lastStepTime = millis();
    playing = true;
}

void Sequencer::update() {
    if (!playing) return;
    
    unsigned long currentTime = millis();
    if (currentTime - lastStepTime >= stepDuration) {
        currentStep = (currentStep + 1) % COLS;
        playStep();
        lastStepTime = currentTime;
        digitalWrite(LED_PIN, currentStep == 0 ? HIGH : LOW);
    }
}

void Sequencer::toggleStep(int row, int col) {
    sequencerGrid[row][col] = !sequencerGrid[row][col];
}

void Sequencer::playStep() {
    stopAllNotes();
    
    for (int row = 0; row < ROWS; row++) {
        if (sequencerGrid[row][currentStep]) {
            MIDI::playNote(sequencerNotes[row]);
            activeNotes[row] = true;
        }
    }
}

void Sequencer::stopAllNotes() {
    for (int row = 0; row < ROWS; row++) {
        if (activeNotes[row]) {
            MIDI::stopNote(sequencerNotes[row]);
            activeNotes[row] = false;
        }
    }
}

#endif // MASTER_MODE
