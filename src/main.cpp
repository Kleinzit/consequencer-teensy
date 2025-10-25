#include <Arduino.h>
#include "config.h"
#include "rs485.h"

#ifdef MASTER_MODE
// Master mode: sequencer, display, MIDI, RS-485
#include "sequencer.h"
#include "display.h"
#include "leds.h"

Sequencer sequencer;
Display display;
LEDs leds;
RS485 rs485;

void handleKeyPress(int row, int col);
void updateDisplay();

void setup() {
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(115200);
    delay(1000); // Wait for serial connection
    
    Serial.println("=== MASTER MODE ===");
    Serial.println("Initializing sequencer...");
    sequencer.init();
    
    Serial.println("Initializing display...");
    display.init();
    
    Serial.println("Initializing LEDs...");
    leds.init();
    
    Serial.println("Initializing RS-485...");
    rs485.init();
    
    updateDisplay();
    Serial.println("Master ready. Starting to poll slave...");
}

void loop() {
    static bool remoteKeyState[ROWS][COLS] = {false};
    static bool lastRemoteKeyState[ROWS][COLS] = {false};
    static unsigned long lastHeartbeat = 0;
    static unsigned long pollCount = 0;
    static unsigned long pollFailCount = 0;
    
    // Poll slave for keyboard state
    if (rs485.pollSlave(remoteKeyState)) {
        pollCount++;
        
        // Check for key changes
        for (int row = 0; row < ROWS; row++) {
            for (int col = 0; col < COLS; col++) {
                // Detect key press (was off, now on)
                if (remoteKeyState[row][col] && !lastRemoteKeyState[row][col]) {
                    handleKeyPress(row, col);
                }
                lastRemoteKeyState[row][col] = remoteKeyState[row][col];
            }
        }
    } else {
        pollFailCount++;
    }
    
    sequencer.update();
    updateDisplay();
    
    // Heartbeat and stats every 5 seconds
    if (millis() - lastHeartbeat > 5000) {
        Serial.print("Heartbeat: Master running | Polls: ");
        Serial.print(pollCount);
        Serial.print(" | Fails: ");
        Serial.print(pollFailCount);
        Serial.print(" | Success rate: ");
        if (pollCount + pollFailCount > 0) {
            Serial.print((pollCount * 100) / (pollCount + pollFailCount));
            Serial.println("%");
        } else {
            Serial.println("N/A");
        }
        lastHeartbeat = millis();
    }
    
    delay(1);
}

void handleKeyPress(int row, int col) {
    Serial.print("Remote key PRESS: [");
    Serial.print(row);
    Serial.print("][");
    Serial.print(col);
    Serial.println("] - Toggling step");
    
    sequencer.toggleStep(row, col);
    updateDisplay();
}

void updateDisplay() {
    static bool grid[ROWS][COLS];
    
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            grid[row][col] = sequencer.isStepActive(row, col);
        }
    }
    
    int currentStep = sequencer.getCurrentStep();
    leds.update(grid, currentStep);
    display.updateGrid(grid, currentStep);
}

#endif // MASTER_MODE

#ifdef SLAVE_MODE
// Slave mode: matrix scanning, RS-485 only
#include "matrix.h"

Matrix matrix;
RS485 rs485;

void handleKeyPress(int row, int col);
void handleKeyRelease(int row, int col);

void setup() {
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(115200);
    delay(1000); // Wait for serial connection
    
    Serial.println("=== SLAVE MODE ===");
    Serial.println("Initializing keyboard matrix...");
    matrix.init();
    matrix.setKeyPressCallback(handleKeyPress);
    matrix.setKeyReleaseCallback(handleKeyRelease);
    
    Serial.println("Initializing RS-485...");
    rs485.init();
    
    Serial.println("Slave ready. Waiting for poll requests...");
}

void loop() {
    static bool keyState[ROWS][COLS];
    static unsigned long lastHeartbeat = 0;
    
    // Scan the keyboard matrix
    matrix.scan();
    
    // Get current key state
    matrix.getKeyState(keyState);
    
    // Check for RS-485 poll requests and respond
    rs485.checkAndRespond(keyState);
    
    // Heartbeat every 5 seconds
    if (millis() - lastHeartbeat > 5000) {
        Serial.println("Heartbeat: Slave running...");
        lastHeartbeat = millis();
    }
    
    delay(1);
}

void handleKeyPress(int row, int col) {
    Serial.print("Key PRESS: [");
    Serial.print(row);
    Serial.print("][");
    Serial.print(col);
    Serial.println("]");
    digitalWrite(LED_PIN, HIGH);
}

void handleKeyRelease(int row, int col) {
    Serial.print("Key RELEASE: [");
    Serial.print(row);
    Serial.print("][");
    Serial.print(col);
    Serial.println("]");
    digitalWrite(LED_PIN, LOW);
}

#endif // SLAVE_MODE