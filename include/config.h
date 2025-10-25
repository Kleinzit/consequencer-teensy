#pragma once

// Pin definitions
#define LED_PIN 13

// WS2812B LED strip
#define LED_DATA_PIN 21
#define NUM_LEDS 8

// OLED display pins (Software SPI)
#define OLED_CLK    17
#define OLED_MOSI   16
#define OLED_CS     14
#define OLED_DC     15
#define OLED_RESET  22

// MIDI settings
#define MIDI_CHANNEL 1
#define VELOCITY 100

// Sequencer matrix configuration
#define ROWS 4
#define COLS 8

// Matrix pin arrays
extern const int rowPins[ROWS];
extern const int colPins[COLS];

// Sequencer configuration
extern const int sequencerNotes[ROWS];
extern const char* noteNames[ROWS];
extern const unsigned long stepDuration;
extern const unsigned long debounceDelay;

// RS-485 Communication (Serial7)
#define RS485_SERIAL Serial7
#define RS485_TX_PIN 28
#define RS485_RX_PIN 29
#define RS485_BAUD 115200
