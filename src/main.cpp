#include <Arduino.h>
#include "rs485.h"
#include "leds.h"
#include "shift_register.h"
#include "matrix.h"
#include "config.h"

// Pin definitions
#define DIP2_PIN 12  // LOW = Master, HIGH = Slave

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

const unsigned long POLL_INTERVAL = 50; 

// LED pattern state (32 bits for 8x4 matrix)
uint32_t ledPattern = 0x00000000;

// Basic sequencer state (4 steps)
int currentStep = 0;
unsigned long lastStepTime = 0;
const unsigned long stepDuration = 250;  // 120 BPM

// Matrix callbacks
void onMatrixKeyPress(int row, int col) {
    Serial.print("Key pressed: Row ");
    Serial.print(row);
    Serial.print(", Col ");
    Serial.println(col);
    
    // Calculate LED bit index (column-major order: col * ROWS + row)
    int ledIndex = col * ROWS + row;
    
    // Toggle the corresponding LED bit
    ledPattern ^= (1UL << ledIndex);
    
    // Write new pattern to shift register
    shiftReg.write(ledPattern);
    
    Serial.print("LED Index: ");
    Serial.print(ledIndex);
    Serial.print(", New Pattern: 0x");
    Serial.println(ledPattern, HEX);
}

void onMatrixKeyRelease(int row, int col) {
    Serial.print("Key released: Row ");
    Serial.print(row);
    Serial.print(", Col ");
    Serial.println(col);
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
    matrix.setKeyReleaseCallback(onMatrixKeyRelease);
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

void loop() {
    unsigned long currentTime = millis();
    
    // Update sequencer step (only master increments)
    if (isMaster && currentTime - lastStepTime >= stepDuration) {
        currentStep = (currentStep + 1) % 16;  // Cycle through 0-15
        lastStepTime += stepDuration;  // Use scheduled time, not actual time
    }
    
    // Slaves get step from RS485
    if (!isMaster) {
        currentStep = rs485.getLastReceivedStep();
    }
    
    // Calculate local column to highlight (0-3) based on global step (0-15) and address
    // Each board handles 4 steps: board 0 = steps 0-3, board 1 = steps 4-7, etc.
    int localCol = -1;  // -1 means don't highlight
    int boardStartStep = myAddress * 4;
    int boardEndStep = boardStartStep + 3;
    if (currentStep >= boardStartStep && currentStep <= boardEndStep) {
        localCol = currentStep % 4;
    }
    
    // Run rainbow animation on LED strips with current step highlighting
    leds.runRainbowAnimation(localCol);
    
    // Scan matrix for key presses
    matrix.scan();
   
    if (isMaster) {
        // MASTER MODE: Round-robin polling
        if (currentTime - lastPollTime > POLL_INTERVAL) {
            lastPollTime = currentTime;  // Update poll time
            
            // Get the current slave address to poll
            uint8_t slaveAddr = slaveAddresses[currentSlaveIndex];
            
            Serial.print("Polling slave ");
            Serial.print(slaveAddr);
            Serial.print("... ");
            
            // Send request with current step and wait for response
            if (rs485.sendRequestToSlave(slaveAddr, currentStep)) {
                slaveStatus[slaveAddr] = 1;  // Mark as responding
                Serial.println("OK!");
            } else {
                slaveStatus[slaveAddr] = 0;  // Mark as not responding
                Serial.println("TIMEOUT");
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
        }
    } else {
        // SLAVE MODE: Listen and respond
        rs485.listenAndRespond(myAddress);
        delay(1);
    }
}
