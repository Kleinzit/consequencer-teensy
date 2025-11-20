#pragma once
#include <Arduino.h>

class RS485 {
public:
    void init();
    uint8_t getAddress();
    bool sendRequestToSlave(uint8_t slaveAddress, uint8_t step, uint32_t& buttonState);
    void listenAndRespond(uint8_t myAddress, uint32_t buttonState);
    uint8_t getLastReceivedStep() const { return lastReceivedStep; }

private:
    uint8_t lastReceivedStep = 0;
};
