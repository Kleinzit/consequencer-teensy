#pragma once
#include "config.h"

class Matrix {
public:
    void init();
    void scan();
    void setKeyPressCallback(void (*callback)(int row, int col));
    void setKeyReleaseCallback(void (*callback)(int row, int col));
    void getKeyState(bool state[ROWS][COLS]);

private:
    bool keyState[ROWS][COLS] = {false};
    bool lastKeyState[ROWS][COLS] = {false};
    unsigned long lastDebounceTime[ROWS][COLS] = {0};
    
    void (*onKeyPress)(int row, int col) = nullptr;
    void (*onKeyRelease)(int row, int col) = nullptr;
};
