#pragma once
#include <Arduino.h>
#include "config.h"

class ShiftRegister {
public:
    void init();
    void write(uint32_t data);
    void clear();
    void runTestPattern(int patternNum);
    
private:
    void clockOut(uint32_t data);
    void latch();
};

