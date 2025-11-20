#include "shift_register.h"

void ShiftRegister::init() {
    // Initialize pins as outputs
    pinMode(SHIFT_REG_SCLK, OUTPUT);
    pinMode(SHIFT_REG_RCLK, OUTPUT);
    pinMode(SHIFT_REG_DATA, OUTPUT);
    
    // Set initial states
    digitalWrite(SHIFT_REG_SCLK, LOW);
    digitalWrite(SHIFT_REG_RCLK, LOW);
    digitalWrite(SHIFT_REG_DATA, LOW);
    
    // Clear all outputs
    clear();
}

void ShiftRegister::clockOut(uint32_t data) {
    // Shift out 32 bits, MSB first
    for (int i = 31; i >= 0; i--) {
        // Set data line
        digitalWrite(SHIFT_REG_DATA, (data >> i) & 1);
        
        // Pulse clock high then low
        digitalWrite(SHIFT_REG_SCLK, HIGH);
        delayMicroseconds(1);
        digitalWrite(SHIFT_REG_SCLK, LOW);
        delayMicroseconds(1);
    }
}

void ShiftRegister::latch() {
    // Pulse latch to transfer data to output registers
    digitalWrite(SHIFT_REG_RCLK, HIGH);
    delayMicroseconds(1);
    digitalWrite(SHIFT_REG_RCLK, LOW);
    delayMicroseconds(1);
}

void ShiftRegister::write(uint32_t data) {
    clockOut(~data);
    latch();
}

void ShiftRegister::clear() {
    write(0x00000000);
}

void ShiftRegister::runTestPattern(int patternNum) {
    static uint32_t counter = 0;
    static int walkPosition = 0;
    static int nibblePosition = 0;
    
    switch (patternNum) {
        case 0:
            // All LEDs ON
            Serial.println("Pattern 0: All LEDs ON (0xFFFFFFFF)");
            write(0xFFFFFFFF);
            break;
            
        case 1:
            // All LEDs OFF
            Serial.println("Pattern 1: All LEDs OFF (0x00000000)");
            write(0x00000000);
            break;
            
        case 2:
            // Alternating bits (0xAAAAAAAA)
            Serial.println("Pattern 2: Alternating bits (0xAAAAAAAA)");
            write(0xAAAAAAAA);
            break;
            
        case 3:
            // Inverse alternating bits (0x55555555)
            Serial.println("Pattern 3: Inverse alternating (0x55555555)");
            write(0x55555555);
            break;
            
        case 4:
            // Walking single LED
            Serial.print("Pattern 4: Walking LED - Position ");
            Serial.println(walkPosition);
            write(1UL << walkPosition);
            walkPosition = (walkPosition + 1) % 32;
            break;
            
        case 5:
            // Walking nibble (4-bit pattern)
            Serial.print("Pattern 5: Walking nibble - Position ");
            Serial.println(nibblePosition);
            write(0x0FUL << nibblePosition);
            nibblePosition = (nibblePosition + 1) % 29;  // 32 - 4 + 1
            if (nibblePosition == 0) nibblePosition = 0;  // Reset
            break;
            
        case 6:
            // Binary counter
            Serial.print("Pattern 6: Binary counter - ");
            Serial.println(counter, HEX);
            write(counter);
            counter++;
            break;
            
        case 7:
            // Checkerboard pattern 1 (groups of 2)
            Serial.println("Pattern 7: Checkerboard 2-bit (0xCCCCCCCC)");
            write(0xCCCCCCCC);
            break;
            
        case 8:
            // Checkerboard pattern 2 (groups of 4)
            Serial.println("Pattern 8: Checkerboard 4-bit (0xF0F0F0F0)");
            write(0xF0F0F0F0);
            break;
            
        case 9:
            // Checkerboard pattern 3 (groups of 8)
            Serial.println("Pattern 9: Checkerboard 8-bit (0xFF00FF00)");
            write(0xFF00FF00);
            break;
            
        default:
            // Reset counters and start over
            counter = 0;
            walkPosition = 0;
            nibblePosition = 0;
            break;
    }
}

