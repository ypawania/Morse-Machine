#include <Arduino.h>
#include <avr/pgmspace.h> // For storing strings in program memory

// Morse code table
// Define a structure to hold Morse code letters and their corresponding codes
struct morseEntry {
  char letter;
  char* code;
};

//Arduino pin definations
#define MORSE_IN 8

//Function declarations
String detectButtonPress();
morseEntry getMorseEntry(int index);

//Constants
const uint32_t DEBOUNCETIME = 50; // Minimum time to debounce a button press
const uint32_t DASHTIME = 700; // Minimum time to consider a button press as a dash

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MORSE_IN, INPUT);
}

void loop() {
  detectButtonPress();
}

// Function to detect button press and return the type of press
// Returns "dot", "dash", or "debounce" based on the duration of the button press
String detectButtonPress() {
  uint32_t currentMillis = millis();
  static uint32_t lastMillis = 0;
  static bool buttonState = digitalRead(MORSE_IN);
  static bool lastButtonState = HIGH;

  if (buttonState != lastButtonState) {
    if (buttonState == HIGH) {
      lastMillis = currentMillis;
    }
    else {
      uint32_t duration = currentMillis - lastMillis;
      if (duration >= DASHTIME) {
        return "dash";
      }
      else if (duration >= DEBOUNCETIME) {
        return "dot";
      }
      else {
        return "debounce";
      }
    }
  }
}

// Morse code table
// Define a structure to hold Morse code letters and their corresponding codes
// The Morse code strings are stored in program memory to save RAM space
struct morseEntry {
  char letter;
  char* code;
};
const morseEntry morseTable[] PROGMEM = {
  {'A', ".-"},
  {'B', "-..."},
  {'C', "-.-."},
  {'D', "-.."},
  {'E', "."},
  {'F', "..-."},
  {'G', "--."},
  {'H', "...."},
  {'I', ".."},
  {'J', ".---"},
  {'K', "-.-"},
  {'L', ".-.."},
  {'M', "--"},
  {'N', "-."},
  {'O', "---"},
  {'P', ".--."},
  {'Q', "--.-"},
  {'R', ".-."},
  {'S', "..."},
  {'T', "-"},
  {'U', "..-"},
  {'V', "...-"},
  {'W', ".--"},
  {'X', "-..-"},
  {'Y', "-.--"},
  {'Z', "--.."}
};

// Function to get Morse Letter and Code for a given index
morseEntry getMorseEntry(int index) {
  const int MORSE_TABLE_SIZE = sizeof(morseTable) / sizeof(morseEntry);
  if (index < 0 || index > MORSE_TABLE_SIZE -1) {
    return {'?', ""}; // Return a default entry for invalid index
  }
  morseEntry entry;
  entry.letter = pgm_read_byte(&morseTable[index].letter);
  entry.code = (char*)pgm_read_word(&morseTable[index].code);
  return entry;
}

