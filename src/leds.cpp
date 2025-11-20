#include "leds.h"
#include <Arduino.h>

// Helper function to convert row/col to LED index (handles zig-zag pattern)
int LEDs::getLEDIndex(int row, int col) {
    // Even rows (0, 2, 4, 6): LEDs go forward (left to right)
    // Odd rows (1, 3, 5, 7): LEDs go backward (right to left)
    if (row % 2 == 0) {
        // Even row: index = row * 4 + col
        return row * 4 + col;
    } else {
        // Odd row: index = row * 4 + (3 - col)
        return row * 4 + (3 - col);
    }
}

void LEDs::init() {
    // Initialize all 3 LED strips
    FastLED.addLeds<WS2812B, LED_STRIP_1_PIN, GRB>(strip1, NUM_LEDS_STRIP_1);
    FastLED.addLeds<WS2812B, LED_STRIP_2_PIN, GRB>(strip2, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<WS2812B, LED_STRIP_3_PIN, GRB>(strip3, NUM_LEDS_PER_STRIP);
    FastLED.setBrightness(196);  // Cap brightness at 196
    FastLED.clear();
    FastLED.show();
}

void LEDs::runRainbowAnimation(int currentStep, bool enableStrip3) {
    unsigned long currentTime = millis();
    
    // Only update animation every 50ms for slower, smoother animation
    if (currentTime - lastAnimationUpdate < 50) {
        return;
    }
    lastAnimationUpdate = currentTime;
    
    // Create rainbow effect across all strips with step highlighting
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
        CRGB color;
        
        // Check if this LED should be highlighted (current step column)
        bool isStepLED = false;
        if (currentStep >= 0 && currentStep < 4) {
            // Check all 8 rows for this step's column
            for (int row = 0; row < 8; row++) {
                if (getLEDIndex(row, currentStep) == i) {
                    isStepLED = true;
                    break;
                }
            }
        }
        
        if (isStepLED) {
            // Current step: white color
            color = CRGB(128, 128, 128);
        } else {
            // Other LEDs: rainbow animation
            uint8_t hue = hueOffset + (i * 256 / NUM_LEDS_PER_STRIP);
            color = CHSV(hue, 255, 196);
        }
        
        
        // Strip 3: only set color if enabled, otherwise turn off
        if (enableStrip3) {
            strip3[i] = color;
        } else {
            strip3[i] = CRGB::Black;
        }
    }
    
    // Increment hue offset for animation (slower increment)
    hueOffset += 1;
    
    FastLED.show();
}

void LEDs::runIdleAnimation(uint16_t globalStep, uint8_t myAddress) {
    unsigned long currentTime = millis();
    
    // Only update animation every 50ms for smooth animation
    if (currentTime - lastAnimationUpdate < 50) {
        return;
    }
    lastAnimationUpdate = currentTime;
   
    // Calculate which board is currently filling (0-3)
    uint8_t activeBoard = (globalStep  % 200) / 50;
    
    // Calculate how many LEDs to fill on the active board (0-49)
    uint8_t fillCount = globalStep % 50;
    
    // Calculate rainbow color: full cycle through 256 hues in 8 cycles
    // Each cycle advances hue by 32 (256 / 8 = 32)
    uint8_t hue = (globalStep / 200) * 32;
    CRGB color = CHSV(hue, 255, 196);
    
    // Update strip 1 based on whether this board is active
    if (myAddress == activeBoard) {
        // This board is active - fill LEDs from 0 to fillCount with color
        for (int i = 0; i < NUM_LEDS_STRIP_1; i++) {
            if (i <= fillCount) {
                strip1[i] = color;
            } 
        }
    }
    
    // Strip 2: ignore for now (keep off)
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
        strip2[i] = CRGB::Black;
    }
    
    // Strip 3: off during idle animation
    for (int i = 0; i < NUM_LEDS_PER_STRIP; i++) {
        strip3[i] = CRGB::Black;
    }
    
    FastLED.show();
}

void LEDs::clear() {
    FastLED.clear();
    FastLED.show();
}

void LEDs::initTestLEDs() {
    // Initialize test LED pins as outputs (inverted logic - LOW = ON, HIGH = OFF)
    for (int i = 0; i < NUM_TEST_LEDS; i++) {
        pinMode(testLedPins[i], OUTPUT);
        digitalWrite(testLedPins[i], HIGH);  // HIGH = OFF (LEDs sink current)
    }
}


void LEDs::updateTestLEDs(uint8_t bitMask) {
    // Update LEDs based on bit mask (each bit represents a button state)
    // Inverted logic: LOW = ON, HIGH = OFF (LEDs sink current)
    for (int i = 0; i < NUM_TEST_LEDS; i++) {
        if (bitMask & (1 << i)) {
            digitalWrite(testLedPins[i], LOW);   // Bit set = LED ON
        } else {
            digitalWrite(testLedPins[i], HIGH);  // Bit clear = LED OFF
        }
    }
}
