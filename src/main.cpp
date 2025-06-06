#include <Arduino.h>
#include <avr/pgmspace.h> // For storing strings in program memory

//Arduino pin definations
#define MORSE_IN 8
#define MODE_SELECT_PIN A0
#define SUBMIT_BUTTON_PIN 9  // 1. Add this line

//Function declarations
char detectButtonPress();
char* getMorseEntry(int index);
void modeSelect();
void practice();
void display(String message);
char nextLetter();

//Constants
const uint32_t DEBOUNCETIME = 50; // Minimum time to debounce a button press
const uint32_t DASHTIME = 200; // Minimum time to consider a button press as a dash
const uint32_t REFRESHTIME = 1000; // Time to refresh input state

bool learnMode, testMode, transmitMode;

enum Mode {
  LEARN,
  TEST,
  TRANSMIT,
  NONE
};

Mode currentMode = NONE;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MORSE_IN, INPUT);
  pinMode(SUBMIT_BUTTON_PIN, INPUT_PULLUP); // 2. Set pin 9 as input with pull-up
}

void loop() {
  modeSelect();
  if (currentMode == LEARN || currentMode == TEST) {
    practice();
  } 
  else if (currentMode == TRANSMIT) {
    Serial.println("Transmit mode not implemented yet.");
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

void practice() {
  String input = "";
  while (currentMode == LEARN || currentMode == TEST) {
    char letter = nextLetter();
    String morseCode = getMorseEntry(letter);
    String displayString;
    if (currentMode == LEARN) {
      displayString = String(letter) + ": " + morseCode;
    } else { // TEST
      displayString = String(letter);
    }
    display(displayString);

    while (input != morseCode && (currentMode == LEARN || currentMode == TEST)) {
      modeSelect();
      char buttonPress = detectButtonPress();

      // 3. Read the submit button state
      bool submitPressed = digitalRead(SUBMIT_BUTTON_PIN) == LOW;

      if (buttonPress == 'd' || buttonPress == '\0') {
        continue; // debounce, ignore this press
      }
      else if (buttonPress == 'r') {
        Serial.println("Refreshing input state...");
        input = ""; // Reset input state
        continue; // Refresh input state
      }
      else if (buttonPress == '.' || buttonPress == '-') {
        input += buttonPress;
        Serial.print(buttonPress);
        if (currentMode == LEARN && input == morseCode) {
          Serial.println(" : Correct input!");
          input = ""; // Reset input after correct entry
          morseCode = ""; // Reset morseCode to empty to trigger while loop exit 
        }
      }

      // 4. Only compare in TEST mode when submit button is pressed
      if (currentMode == TEST && submitPressed) {
        if (input == morseCode) {
          Serial.println(" : Correct input!");
        } else {
          Serial.println(" : Incorrect input!");
        }
        input = ""; // Reset input after checking
        morseCode = ""; // Reset morseCode to empty to trigger while loop exit 
        delay(300); // Simple debounce for submit button
      }
    }
  }
}

char nextLetter() {
  int randomIndex = random(0, 26); 
  return randomIndex + 'A'; 
}

void display(String message ) {
  Serial.println(message);
}

void modeSelect() {
  int potValue = analogRead(MODE_SELECT_PIN);
  if (potValue < 250) {
    if (currentMode != LEARN) Serial.println("Entering learn mode...");
    currentMode = LEARN;
  }
  else if (potValue > 1023 - 250) {
    if (currentMode != TEST) Serial.println("Entering test mode...");
    currentMode = TEST;
  }
  else {
    if (currentMode != TRANSMIT) Serial.println("Entering transmit mode...");
    currentMode = TRANSMIT;
  }
}