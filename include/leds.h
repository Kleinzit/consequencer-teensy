#pragma once
#include <FastLED.h>
#include "config.h"

class LEDs {
public:
    void init();
    void update(bool grid[ROWS][COLS], int currentStep);
    void clear();

private:
    CRGB leds[NUM_LEDS];
};
