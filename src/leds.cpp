#include "leds.h"

void LEDs::init() {
    FastLED.addLeds<WS2812B, LED_DATA_PIN, RGB>(leds, NUM_LEDS);
    FastLED.setBrightness(50);
    FastLED.clear();
    FastLED.show();
}

void LEDs::update(bool grid[ROWS][COLS], int currentStep) {
    FastLED.clear();
    
    // Show first row (C4) notes in red
    for (int col = 0; col < COLS; col++) {
        if (grid[0][col]) {
            leds[col] = CRGB(255, 0, 0);
        }
    }
    
    // Current step in white
    leds[currentStep] = CRGB(255, 255, 255);
    
    FastLED.show();
}

void LEDs::clear() {
    FastLED.clear();
    FastLED.show();
}
