#include "rs485.h"
#include <Arduino.h>

void RS485::init() {
    RS485_SERIAL.begin(RS485_BAUD);
    // Clear any existing data in the buffer
    while (RS485_SERIAL.available()) {
        RS485_SERIAL.read();
    }
}

#ifdef MASTER_MODE
bool RS485::pollSlave(bool keyState[ROWS][COLS]) {
    // Send poll request
    RS485_SERIAL.write(POLL_REQUEST);
    RS485_SERIAL.flush();
    
    // Wait for response with timeout
    return receiveKeyState(keyState);
}
#endif

#ifdef SLAVE_MODE
void RS485::checkAndRespond(bool keyState[ROWS][COLS]) {
    // Check if there's a poll request
    if (RS485_SERIAL.available() > 0) {
        uint8_t received = RS485_SERIAL.read();
        
        if (received == POLL_REQUEST) {
            // Send current key state
            sendKeyState(keyState);
        }
    }
}
#endif

void RS485::sendKeyState(bool keyState[ROWS][COLS]) {
    uint8_t data[ROWS];
    
    // Pack the 2D boolean array into bytes
    // Each byte represents one row, with each bit representing a column
    for (int row = 0; row < ROWS; row++) {
        data[row] = 0;
        for (int col = 0; col < COLS; col++) {
            if (keyState[row][col]) {
                data[row] |= (1 << col);
            }
        }
    }
    
    // Send the packed data
    RS485_SERIAL.write(data, ROWS);
    RS485_SERIAL.flush();
}

bool RS485::receiveKeyState(bool keyState[ROWS][COLS]) {
    unsigned long startTime = millis();
    int bytesReceived = 0;
    uint8_t data[ROWS];
    
    // Wait for all bytes with timeout
    while (bytesReceived < ROWS && (millis() - startTime) < TIMEOUT_MS) {
        if (RS485_SERIAL.available() > 0) {
            data[bytesReceived] = RS485_SERIAL.read();
            bytesReceived++;
        }
    }
    
    // Check if we received all bytes
    if (bytesReceived != ROWS) {
        return false;
    }
    
    // Unpack the bytes into the 2D boolean array
    for (int row = 0; row < ROWS; row++) {
        for (int col = 0; col < COLS; col++) {
            keyState[row][col] = (data[row] & (1 << col)) != 0;
        }
    }
    
    return true;
}

