#pragma once
#include "config.h"

class Sequencer {
public:
    void init();
    void update();
    void toggleStep(int row, int col);
    int getCurrentStep() const { return currentStep; }
    bool isStepActive(int row, int col) const { return sequencerGrid[row][col]; }
    bool isNoteActive(int row) const { return activeNotes[row]; }
    bool isPlaying() const { return playing; }

private:
    void playStep();
    void stopAllNotes();
    
    bool sequencerGrid[ROWS][COLS] = {false};
    int currentStep = 0;
    unsigned long lastStepTime = 0;
    bool playing = true;
    bool activeNotes[ROWS] = {false};
};
