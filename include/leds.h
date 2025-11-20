#pragma once
#include <FastLED.h>
#include "config.h"

class LEDs {
public:
    void init();
    void runRainbowAnimation(int currentStep = -1, bool enableStrip3 = true);  // Rainbow animation with optional step highlight
    void runIdleAnimation(uint16_t globalStep, uint8_t myAddress);  // Synchronized idle animation
    void clear();
    
    // Test LED functions
    void initTestLEDs();
    void updateTestLEDs(uint8_t bitMask);  // Each bit represents a button state

private:
    CRGB strip1[NUM_LEDS_STRIP_1];
    CRGB strip2[NUM_LEDS_PER_STRIP];
    CRGB strip3[NUM_LEDS_PER_STRIP];
    const int testLedPins[NUM_TEST_LEDS] = {TEST_LED_1, TEST_LED_2, TEST_LED_3, TEST_LED_4};
    uint8_t hueOffset = 0;  // For animating rainbow
    unsigned long lastAnimationUpdate = 0;  // Timing for animation
    uint16_t lastIdleStep = 0;  // Track last idle step to detect cycle completion
    uint8_t idleCycleCount = 0;  // Count completed 200-step cycles
    
    // Helper function to convert row/col to LED index (handles zig-zag pattern)
    int getLEDIndex(int row, int col);
};
