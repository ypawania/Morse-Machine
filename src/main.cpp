#include <Arduino.h>
#include <avr/pgmspace.h> // For storing strings in program memory

//Arduino pin definations
#define MORSE_IN 8
#define MODE_SELECT_PIN A0

//Function declarations
char detectButtonPress();
char* getMorseEntry(int index);
void modeSelect();
void learn();
void display(char letter, char* morseCode);
char nextLetter();

//Constants
const uint32_t DEBOUNCETIME = 50; // Minimum time to debounce a button press
const uint32_t DASHTIME = 700; // Minimum time to consider a button press as a dash
const uint32_t REFRESHTIME = 4000; // Time to refresh input state

bool learnMode, testMode, transmitMode;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MORSE_IN, INPUT);
}

void loop() {
  /**learnMode = true;
  if (learnMode) {
    learn();
  } **/
  char buttonPress = detectButtonPress();
  if (buttonPress != '\0'){
    Serial.println(buttonPress);
  }

}

// Function to detect button press and return the type of press
// Returns "dot", "dash", or "debounce" based on the duration of the button press
char detectButtonPress() {
  uint32_t currentMillis = millis();
  static uint32_t lastMillis = 0;
  bool buttonState = digitalRead(MORSE_IN);
  static bool lastButtonState = buttonState;

  char result = '\0';

  if (buttonState != lastButtonState) {
    lastButtonState = buttonState;
    if (buttonState == HIGH) {
      lastMillis = currentMillis;
    }
    else {
      uint32_t duration = currentMillis - lastMillis;
      if (duration > REFRESHTIME) {  // used elsewhere to refresh input state
        result = 'r';
      }
      else if (duration >= DASHTIME) {
        result = '-';
      }
      else if (duration >= DEBOUNCETIME) {
        result = '.';
      }
      else {
        result = 'd';
      }
    }
  }
  return result;
}

// Morse code table
// Define a structure to hold Morse code letters and their corresponding codes
struct morseEntry {
  char letter;
  char* code;
};

// The Morse code strings are stored in program memory to save RAM space
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


// Function to get Morse code entry for a specific letter
char* getMorseEntry(char letter) {
  const int MORSE_TABLE_SIZE = sizeof(morseTable) / sizeof(morseEntry);
  if (letter < 'A' || letter > 'Z') {
    return ""; // Return empty string for invalid letters
  }
  return (char*) pgm_read_word(&morseTable[letter - 'A'].code);
}

void learn() {
  String input = "";
  Serial.println("Entering learn mode...");
  while (learnMode) {
    char letter = nextLetter();
    char* morseCode = getMorseEntry(letter);
    display(letter, morseCode);
    
    while (input != morseCode && learnMode) {
      char buttonPress = detectButtonPress();
      
      if (buttonPress == 'd') {
        continue; // debounce, ignore this press
      }
      else if (buttonPress == 'r') {
        Serial.println("Refreshing input state...");
        input = ""; // Reset input state
        continue; // Refresh input state
      }
      
      // If a valid button press is detected, append it to the input string
      else if (buttonPress == '.' || buttonPress == '-') {
        input += buttonPress;
        Serial.print("Detected button press: ");
        Serial.println(buttonPress);
      }
    }
  }
}

char nextLetter() {
  int randomIndex = random(0, 26); 
  return randomIndex + 'A'; 
}

void display(char letter, char* morseCode) {
  Serial.print(letter);
  Serial.print(": ");
  Serial.println(morseCode);
}

void modeSelect() {
  int potValue = analogRead(MODE_SELECT_PIN);
  if (potValue < 250) {
    learnMode = true;
    testMode = false;
    transmitMode = false;
  }
  else if (potValue > 1023 - 250) {
    testMode = true;
    learnMode = false;
    transmitMode = false;
  }
  else {
    transmitMode = true;
    learnMode = false;
    testMode = false;
  }

}