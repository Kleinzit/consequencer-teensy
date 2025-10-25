#include "matrix.h"
#include <Arduino.h>

void Matrix::init() {
    for (int i = 0; i < ROWS; i++) {
        pinMode(rowPins[i], OUTPUT);
        digitalWrite(rowPins[i], HIGH);
    }
    
    for (int i = 0; i < COLS; i++) {
        pinMode(colPins[i], INPUT_PULLUP);
    }
    
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            keyState[row][col] = false;
            lastKeyState[row][col] = false;
            lastDebounceTime[row][col] = 0;
        }
    }
}

void Matrix::scan() {
    unsigned long currentTime = millis();
    
    for (int row = 0; row < ROWS; row++) {
        for (int i = 0; i < ROWS; i++) {
            digitalWrite(rowPins[i], (i == row) ? LOW : HIGH);
        }
        
        delayMicroseconds(10);
        
        for (int col = 0; col < COLS; col++) {
            bool currentState = !digitalRead(colPins[col]);
            
            if (currentState != lastKeyState[row][col]) {
                lastDebounceTime[row][col] = currentTime;
            }
            
            if ((currentTime - lastDebounceTime[row][col]) > debounceDelay) {
                if (currentState != keyState[row][col]) {
                    keyState[row][col] = currentState;
                    
                    if (currentState && onKeyPress) {
                        onKeyPress(row, col);
                    } else if (!currentState && onKeyRelease) {
                        onKeyRelease(row, col);
                    }
                }
            }
            
            lastKeyState[row][col] = currentState;
        }
    }
    
    for (int i = 0; i < ROWS; i++) {
        digitalWrite(rowPins[i], HIGH);
    }
}

void Matrix::setKeyPressCallback(void (*callback)(int row, int col)) {
    onKeyPress = callback;
}

void Matrix::setKeyReleaseCallback(void (*callback)(int row, int col)) {
    onKeyRelease = callback;
}

void Matrix::getKeyState(bool state[ROWS][COLS]) {
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            state[row][col] = keyState[row][col];
        }
    }
}
