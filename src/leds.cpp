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
    FastLED.addLeds<WS2812B, LED_STRIP_1_PIN, GRB>(strip1, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<WS2812B, LED_STRIP_2_PIN, GRB>(strip2, NUM_LEDS_PER_STRIP);
    FastLED.addLeds<WS2812B, LED_STRIP_3_PIN, GRB>(strip3, NUM_LEDS_PER_STRIP);
    FastLED.setBrightness(196);  // Cap brightness at 196
    FastLED.clear();
    FastLED.show();
}

void LEDs::runRainbowAnimation(int currentStep) {
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
        
        // Set the same color on all three strips at this position
        strip1[i] = color;
        strip2[i] = color;
        strip3[i] = color;
    }
    
    // Increment hue offset for animation (slower increment)
    hueOffset += 1;
    
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
