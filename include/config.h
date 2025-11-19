#pragma once

// Test LEDs (sink current: LOW = ON, HIGH = OFF)
#define TEST_LED_1 1
#define TEST_LED_2 2
#define TEST_LED_3 3
#define TEST_LED_4 4
#define NUM_TEST_LEDS 4

// Button matrix (1 row x 4 columns)
#define BUTTON_ROW_PIN 38
#define BUTTON_COL_1 20
#define BUTTON_COL_2 21
#define BUTTON_COL_3 22
#define BUTTON_COL_4 23
#define NUM_BUTTON_COLS 4

// DIP switches for addressing and master/slave mode selection
#define DIP0_PIN 10  // Address bit 0 (LSB)
#define DIP1_PIN 11  // Address bit 1 (MSB)
#define DIP2_PIN 12  // LOW = Master, HIGH = Slave

// WS2812B RGB LED strips (3 strips, up to 32 LEDs each)
#define LED_STRIP_1_PIN 16
#define LED_STRIP_2_PIN 41
#define LED_STRIP_3_PIN 39
#define NUM_LEDS_PER_STRIP 32

// Shift Register for output LEDs
#define SHIFT_REG_SCLK 27  // Serial Clock
#define SHIFT_REG_RCLK 36  // Register Clock (Latch)
#define SHIFT_REG_DATA 26  // Serial Data
#define NUM_SHIFT_REGISTER_BITS 32

// RS-485 Communication (Serial7)
#define RS485_SERIAL Serial7
#define RS485_BAUD 115200
#define RS485_TX_PIN 28
#define RS485_RX_PIN 29
#define RS485_DIR_PIN 30  // HIGH = transmit, LOW = receive

// MIDI settings
#define MIDI_CHANNEL 1
#define VELOCITY 100

// Sequencer matrix configuration
#define ROWS 8
#define COLS 4

// Matrix pin arrays
extern const int rowPins[ROWS];
extern const int colPins[COLS];

// Sequencer configuration
extern const int sequencerNotes[ROWS];
extern const char* noteNames[ROWS];
extern const unsigned long stepDuration;
extern const unsigned long debounceDelay;
