#include "rs485.h"
#include "config.h"

// Pin definitions
#define RS485_SERIAL Serial7
#define RS485_DIR_PIN 30

// Protocol constants
#define REQUEST_HEADER_1 0xD1
#define REQUEST_HEADER_2 0xCC
#define ACK_BYTE_1 0xA5
#define ACK_BYTE_2 0x55
#define TIMEOUT_MS 25

void RS485::init() {
    // Initialize DIP switch pins for address reading
    pinMode(DIP0_PIN, INPUT_PULLUP);
    pinMode(DIP1_PIN, INPUT_PULLUP);
    
    // Initialize direction control pin
    pinMode(RS485_DIR_PIN, OUTPUT);
    digitalWrite(RS485_DIR_PIN, LOW);  // Start in receive mode
    
    // Initialize serial
    RS485_SERIAL.begin(115200);
    
    // Clear any existing data in the buffer
    while (RS485_SERIAL.available()) {
        RS485_SERIAL.read();
    }
}

uint8_t RS485::getAddress() {
    // Read address from DIP switches (0-3)
    // DIP switches are active LOW (pulled up by default)
    uint8_t bit0 = (digitalRead(DIP0_PIN) == LOW) ? 1 : 0;
    uint8_t bit1 = (digitalRead(DIP1_PIN) == LOW) ? 1 : 0;
    return (bit1 << 1) | bit0;
}

bool RS485::sendRequestToSlave(uint8_t slaveAddress, uint8_t step, uint32_t& buttonState) {
    // Clear any stale data in receive buffer
    while (RS485_SERIAL.available()) {
        RS485_SERIAL.read();
    }
    
    // Set to transmit mode
    digitalWrite(RS485_DIR_PIN, HIGH);
    delayMicroseconds(10);  // Wait for driver to enable (MAX3485 requires 300ns)
    
    // Send 4-byte request: [0xD1, 0xCC, address, step]
    RS485_SERIAL.write(REQUEST_HEADER_1);
    RS485_SERIAL.write(REQUEST_HEADER_2);
    RS485_SERIAL.write(slaveAddress);
    RS485_SERIAL.write(step);
    RS485_SERIAL.flush();
    
    delayMicroseconds(10);
    // Switch back to receive mode
    digitalWrite(RS485_DIR_PIN, LOW);
   
    // Wait for 6-byte response: [0xA5, 0x55, byte0, byte1, byte2, byte3]
    unsigned long startTime = millis();
    uint8_t bytesReceived = 0;
    uint8_t response[6] = {0, 0, 0, 0, 0, 0};
    
    while ((millis() - startTime) < TIMEOUT_MS) {
        if (RS485_SERIAL.available() > 0) {
            response[bytesReceived] = RS485_SERIAL.read();
            bytesReceived++;
            
            if (bytesReceived == 6) {
                // Check if response matches ACK pattern
                if (response[0] == ACK_BYTE_1 && response[1] == ACK_BYTE_2) {
                    // Extract 32-bit button state (little-endian)
                    buttonState = ((uint32_t)response[2]) |
                                  ((uint32_t)response[3] << 8) |
                                  ((uint32_t)response[4] << 16) |
                                  ((uint32_t)response[5] << 24);
                    return true;
                }
                return false;  // Wrong response
            }
        }
    }
    
    return false;  // Timeout
}

void RS485::listenAndRespond(uint8_t myAddress, uint32_t buttonState) {
    // Check if we have at least 4 bytes available for a complete request
    if (RS485_SERIAL.available() >= 4) {
        uint8_t byte1 = RS485_SERIAL.read();
        
        // Check if this could be the start of a request
        if (byte1 == REQUEST_HEADER_1) {
            uint8_t byte2 = RS485_SERIAL.read();
            uint8_t byte3 = RS485_SERIAL.read();
            uint8_t byte4 = RS485_SERIAL.read();
            
            // Check if request header is valid
            if (byte2 == REQUEST_HEADER_2) {
                // Store the step data (all slaves read this, regardless of address)
                lastReceivedStep = byte4;
                
                // Check if request is addressed to us
                if (byte3 == myAddress) {
                    // Valid request for this slave - send ACK + button state
                    digitalWrite(RS485_DIR_PIN, HIGH);
                    delayMicroseconds(10);  // Wait for driver to enable
                    
                    // Send 6-byte response: [ACK1, ACK2, byte0, byte1, byte2, byte3]
                    RS485_SERIAL.write(ACK_BYTE_1);
                    RS485_SERIAL.write(ACK_BYTE_2);
                    RS485_SERIAL.write((uint8_t)(buttonState & 0xFF));
                    RS485_SERIAL.write((uint8_t)((buttonState >> 8) & 0xFF));
                    RS485_SERIAL.write((uint8_t)((buttonState >> 16) & 0xFF));
                    RS485_SERIAL.write((uint8_t)((buttonState >> 24) & 0xFF));
                    RS485_SERIAL.flush();

                    delayMicroseconds(10);
                    digitalWrite(RS485_DIR_PIN, LOW);
                    
                    Serial.println("ACK + button state sent to master");
                }
            }
        }
    }
}
