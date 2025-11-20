#include <Arduino.h>
#include "rs485.h"
#include "leds.h"
#include "shift_register.h"
#include "matrix.h"
#include "midi.h"
#include "config.h"

// Global objects
RS485 rs485;
LEDs leds;
ShiftRegister shiftReg;
Matrix matrix;
bool isMaster = false;
uint8_t myAddress = 0;
uint8_t slaveAddresses[3] = {0, 0, 0};  // Addresses of the 3 slaves
uint8_t currentSlaveIndex = 0;  // Round-robin index
uint8_t slaveStatus[4] = {0, 0, 0, 0};  // Status of each address (0=off, 1=on)
unsigned long lastPollTime = 0;

// LED pattern state (32 bits for 8x4 matrix)
uint32_t ledPattern = 0x00000000;

// Button state for all 4 boards (master tracks all)
uint32_t allBoardStates[4] = {0, 0, 0, 0};

uint32_t previousBoardStates[4] = {0, 0, 0, 0};

unsigned long lastBoardStateChangeTime = 0;

// Basic sequencer state (4 steps)
int currentStep = 0;
unsigned long lastStepTime = 0;

// Idle animation state
uint16_t idleAnimationStep = 0;
unsigned long lastIdleStepTime = 0;

// Start/Stop button state
bool isRunning = true;
bool lastButtonState = HIGH;
unsigned long lastButtonPressTime = 0;
int currentNoteSet = 0;

// Matrix callbacks
void onMatrixKeyPress(int row, int col) {
    // Calculate LED bit index (column-major order: col * ROWS + row)
    int ledIndex = col * ROWS + row;
    
    // Toggle the corresponding LED bit
    ledPattern ^= (1UL << ledIndex);
    
    // Write new pattern to shift register
    shiftReg.write(ledPattern);
}

// Display function for button state output
void displayButtonStates() {
    // Line 1: Step indicator (16 steps grouped by 4 to match button grid)
    for (int step = 0; step < 16; step++) {
        if (step == currentStep) {
            Serial.print('x');
        } else {
            Serial.print('.');
        }
        // Add space after every 4 steps (except the last one)
        if ((step % 4 == 3) && (step < 15)) {
            Serial.print(' ');
        }
    }
    Serial.println();
    
    // Lines 2-9: Button state grid (8 rows)
    // Each row shows all 4 boards side-by-side
    for (int row = ROWS - 1; row >= 0; row--) {
        Serial.print('\r');  // Return to beginning of line
        for (int board = 0; board < 4; board++) {
            // Get button state for this board
            uint32_t boardState = allBoardStates[board];
            
            // Display 4 columns for this board
            for (int col = 0; col < COLS; col++) {
                // Calculate bit index (column-major: col * ROWS + row)
                int bitIndex = col * ROWS + row;
                bool buttonState = (boardState >> bitIndex) & 1;
                
                // x = OFF (bit 0), o = ON (bit 1)
                Serial.print(buttonState ? 'o' : 'x');
            }
            
            // Add space between boards (except after the last one)
            if (board < 3) {
                Serial.print(' ');
            }
        }
        Serial.println();
    }
    
    Serial.println();  // Extra blank line for readability
    
}

void setup() {
    // Initialize USB Serial for debugging
    Serial.begin(115200);
    delay(1000);
    
    // Initialize RS485 (reads DIP switches)
    rs485.init();
    
    // Get this card's address from DIP switches
    myAddress = rs485.getAddress();
    
    // Read DIP switch to determine master/slave mode
    pinMode(DIP2_PIN, INPUT_PULLUP);
    delay(10);
    isMaster = (digitalRead(DIP2_PIN) == LOW);
    
    // Initialize start/stop button
    pinMode(START_STOP_BUTTON_PIN, INPUT_PULLUP);
    
    // Print mode and address
    Serial.print("=== ");
    if (isMaster) {
        Serial.print("MASTER");
    } else {
        Serial.print("SLAVE");
    }
    Serial.print(" MODE === Address: ");
    Serial.println(myAddress);
    
    // If master, determine which addresses are slaves (all except ours)
    if (isMaster) {
        uint8_t slaveIndex = 0;
        for (uint8_t addr = 0; addr < 4; addr++) {
            if (addr != myAddress) {
                slaveAddresses[slaveIndex] = addr;
                slaveIndex++;
            }
        }
        Serial.print("Slave addresses: ");
        Serial.print(slaveAddresses[0]);
        Serial.print(", ");
        Serial.print(slaveAddresses[1]);
        Serial.print(", ");
        Serial.println(slaveAddresses[2]);
        
        // Master's own LED is always on
        slaveStatus[myAddress] = 1;
    }
    
    // Initialize RGB LED strips
    leds.init();
    
    // Initialize test LEDs
    leds.initTestLEDs();
    
    // Initialize shift register
    shiftReg.init();
    Serial.println("Shift register initialized");
    
    // Initialize matrix with callbacks
    matrix.init();
    matrix.setKeyPressCallback(onMatrixKeyPress);
    Serial.println("Matrix initialized");
    
    // Run through test leds to indicate mode
    for (int j = 0; j < (isMaster ? 1 : 2); j++) {
        leds.updateTestLEDs(0x01);  // LED 0
        delay(100);
        leds.updateTestLEDs(0x02);  // LED 1
        delay(100);
        leds.updateTestLEDs(0x04);  // LED 2
        delay(100);
        leds.updateTestLEDs(0x08);  // LED 3
        delay(100);
        leds.updateTestLEDs(0x00);  // All off
        delay(100);
    }
    
    Serial.println("Ready!");
}

void resetSequencer() {
    // Stop all currently playing MIDI notes
    for (int row = 0; row < ROWS; row++) {
        MIDI::stopNote(sequencerNotes[1][row]);
        MIDI::stopNote(sequencerNotes[2][row]);
        MIDI::stopNote(sequencerNotes[0][row]);
    }
    
    // Reset step to 0
    currentStep = 0;
    
    // Clear LED strips
    leds.clear();
    
    // Set all matrix LEDs on when stopped
    ledPattern = 0xFFFFFFFF;
    shiftReg.write(ledPattern);
}

void checkStartStopButton() {
    bool currentButtonState = digitalRead(START_STOP_BUTTON_PIN);
    unsigned long currentTime = millis();
    
    // Check for button press (HIGH to LOW transition) with debouncing
    if (currentButtonState == LOW && lastButtonState == HIGH) {
        // Check if enough time has passed since last press (debounce)
        if (currentTime - lastButtonPressTime > debounceDelay) {
            // Toggle running state
            isRunning = !isRunning;
            
            if (isRunning) {
                // Starting: cycle to next note set
                currentNoteSet = (currentNoteSet + 1) % 3;
                lastStepTime = currentTime;
                
                // Clear all matrix LEDs when starting
                ledPattern = 0x00000000;
                shiftReg.write(ledPattern);
                leds.clear();
            } else {
                // Stopping: reset sequencer
                resetSequencer();
                leds.clear();
            }
            
            lastButtonPressTime = currentTime;
        }
    }
    
    lastButtonState = currentButtonState;
}

void runSequencer() {
    unsigned long currentTime = millis();
    
    // Only advance steps and play MIDI when running
    if (isRunning && isMaster && currentTime - lastStepTime >= stepDuration) {
        currentStep = (currentStep + 1) % 16;
        lastStepTime += stepDuration;
        
        int board = currentStep / 4;
        int localCol = currentStep % 4;
        for (int row = 0; row < ROWS; row++) {
            int bitIndex = localCol * ROWS + row;
            if ((allBoardStates[board] >> bitIndex) & 1) {
                MIDI::playNote(sequencerNotes[currentNoteSet][row]);
            } 
        }
    }
    
    // When stopped, master sets currentStep to 255
    if (!isRunning && isMaster) {
        currentStep = 255;
    }
    
    // Advance idle animation when stopped (master only)
    if (!isRunning && isMaster && currentTime - lastIdleStepTime >= 50) {
        idleAnimationStep = (idleAnimationStep + 1) % 1600;
        lastIdleStepTime = currentTime;
    }
    
    // Slaves get step and idle step from RS485
    if (!isMaster) {
        static int lastSlaveStep = 0;
        currentStep = rs485.getLastReceivedStep();
        idleAnimationStep = rs485.getLastReceivedIdleStep();
        
        // Detect stop/start transitions
        if (currentStep == 255 && lastSlaveStep != 255) {
            // Transition to stopped: set all LEDs on
            ledPattern = 0xFFFFFFFF;
            shiftReg.write(ledPattern);
            isRunning = false;
            leds.clear();
        } else if (currentStep != 255 && lastSlaveStep == 255) {
            // Transition to running: clear all LEDs
            ledPattern = 0x00000000;
            shiftReg.write(ledPattern);
            isRunning = true;
            leds.clear();
        }
        
        lastSlaveStep = currentStep;
    }
    
    // Determine if strip3 should be enabled (disabled when step == 255, which indicates stopped)
    bool enableStrip3 = (currentStep != 255);
    
    if (isRunning) {
        // When running: show rainbow animation on strip 3 with step highlighting
        // Calculate local column to highlight (0-3) based on global step (0-15) and address
        // Each board handles 4 steps: board 0 = steps 0-3, board 1 = steps 4-7, etc.
        int localCol = -1;  // -1 means don't highlight
        int boardStartStep = myAddress * 4;
        int boardEndStep = boardStartStep + 3;
        if (currentStep >= boardStartStep && currentStep <= boardEndStep) {
            localCol = currentStep % 4;
        }
        
        // Run rainbow animation on LED strips with current step highlighting
        leds.runRainbowAnimation(localCol, enableStrip3);
    } else {
        // When stopped: show synchronized idle animation on strip 1
        leds.runIdleAnimation(idleAnimationStep, myAddress);
    }
    
    // Scan matrix for key presses
    matrix.scan();
   
    if (isMaster) {
        // MASTER MODE: Round-robin polling
        
        if (ledPattern != previousBoardStates[myAddress]) {
            previousBoardStates[myAddress] = ledPattern;
            lastBoardStateChangeTime = currentTime;
        }
        // Update master's own button state
        allBoardStates[myAddress] = ledPattern;
        
        if (currentTime - lastPollTime > POLL_INTERVAL) {
            lastPollTime = currentTime;  // Update poll time
            
            // Get the current slave address to poll
            uint8_t slaveAddr = slaveAddresses[currentSlaveIndex];
            
            // Send request with current step, idle step, and wait for response with button state
            // When stopped, send step 255 as a special indicator
            uint8_t stepToSend = isRunning ? currentStep : 255;
            uint32_t slaveButtonState = 0;
            if (rs485.sendRequestToSlave(slaveAddr, stepToSend, idleAnimationStep, slaveButtonState)) {
                slaveStatus[slaveAddr] = 1;  // Mark as responding
                if (slaveButtonState != previousBoardStates[slaveAddr]) {
                    previousBoardStates[slaveAddr] = slaveButtonState;
                    lastBoardStateChangeTime = currentTime;
                }
                allBoardStates[slaveAddr] = slaveButtonState;  // Save button state
            } else {
                slaveStatus[slaveAddr] = 0;  // Mark as not responding
            }
            
            // Update LED display to show current status
            uint8_t ledMask = 0;
            for (int i = 0; i < 4; i++) {
                if (slaveStatus[i]) {
                    ledMask |= (1 << i);
                }
            }
            leds.updateTestLEDs(ledMask);
            
            // Move to next slave (round-robin)
            currentSlaveIndex = (currentSlaveIndex + 1) % 3;
            
            // Display current state of all boards
            displayButtonStates();
        }
        // if its been 32 bars since the last board state change and we are running, reset the sequencer
        if (currentTime - lastBoardStateChangeTime > 32 * stepDuration * 16 && isRunning) {
            resetSequencer();
            isRunning = false;
        }
    } else {
        // SLAVE MODE: Listen and respond with button state
        rs485.listenAndRespond(myAddress, ledPattern);
        delay(1);
    }
   
}

void loop() {
    if (isMaster) {
        checkStartStopButton();
    }
    runSequencer();
}