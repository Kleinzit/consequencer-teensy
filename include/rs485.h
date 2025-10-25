#pragma once
#include <Arduino.h>
#include "config.h"

class RS485 {
public:
    void init();
    
#ifdef MASTER_MODE
    bool pollSlave(bool keyState[ROWS][COLS]);
#endif

#ifdef SLAVE_MODE
    void checkAndRespond(bool keyState[ROWS][COLS]);
#endif

private:
    static const uint8_t POLL_REQUEST = 0x01;
    static const unsigned long TIMEOUT_MS = 100;
    
    void sendKeyState(bool keyState[ROWS][COLS]);
    bool receiveKeyState(bool keyState[ROWS][COLS]);
};

