#pragma once
#include <FastLED.h>
#include "config.h"

class LEDs {
public:
    void init();
    void runRainbowAnimation(int currentStep = -1);  // Rainbow animation with optional step highlight
    void clear();
    
    // Test LED functions
    void initTestLEDs();
    void updateTestLEDs(uint8_t bitMask);  // Each bit represents a button state

private:
    CRGB strip1[NUM_LEDS_PER_STRIP];
    CRGB strip2[NUM_LEDS_PER_STRIP];
    CRGB strip3[NUM_LEDS_PER_STRIP];
    const int testLedPins[NUM_TEST_LEDS] = {TEST_LED_1, TEST_LED_2, TEST_LED_3, TEST_LED_4};
    uint8_t hueOffset = 0;  // For animating rainbow
    unsigned long lastAnimationUpdate = 0;  // Timing for animation
    
    // Helper function to convert row/col to LED index (handles zig-zag pattern)
    int getLEDIndex(int row, int col);
};
