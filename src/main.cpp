#include <Arduino.h>
#include <avr/pgmspace.h> // For storing strings in program memory

//Arduino pin definations
#define MORSE_IN 8
#define MODE_SELECT_PIN A0
#define SUBMIT_BUTTON_PIN 12

//Function declarations
char detectButtonPress();
String getMorseEntry(char letter);
void modeSelect();
void practice();
void display(String message);
char nextLetter();

//Constants
const uint32_t DEBOUNCETIME = 50; // Minimum time to debounce a button press
const uint32_t DASHTIME = 200; // Minimum time to consider a button press as a dash
const uint32_t REFRESHTIME = 1000; // Time to refresh input state

enum Mode {
  LEARN,
  TEST,
  NONE
};

Mode currentMode = NONE;

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MORSE_IN, INPUT);
  pinMode(SUBMIT_BUTTON_PIN, INPUT); // No pullup, using external pulldown
}

void loop() {
  String input = "";
  Mode previousMode = currentMode; // Track previous mode

  // Get an english letter and it's morse representation
  char letter = nextLetter();
  String morseCode = getMorseEntry(letter);

  // construct display strong based on mode
  String displayString = String(letter);
  if (currentMode == LEARN) {
    displayString += ": " + morseCode;  
  }
  display(displayString);

  while (input != morseCode) {
    modeSelect();
    if (currentMode != previousMode) {
      break; // Exit the while loop if mode changes
    }
    char buttonPress = detectButtonPress();

    // 3. Read the submit button state and detect rising edge
    static bool prevSubmitPressed = false;
    bool submitPressed = digitalRead(SUBMIT_BUTTON_PIN); // HIGH when pressed, LOW when not
    bool submitJustPressed = submitPressed && !prevSubmitPressed;
    prevSubmitPressed = submitPressed;

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

    // 4. Only compare in TEST mode when submit button is just pressed (edge detection)
    if (currentMode == TEST && submitJustPressed) {
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
    } else {
      uint32_t duration = currentMillis - lastMillis;
      if (duration > REFRESHTIME) { 
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
String getMorseEntry(char letter) {
  const int MORSE_TABLE_SIZE = sizeof(morseTable) / sizeof(morseEntry);
  if (letter < 'A' || letter > 'Z') {
    return ""; // Return empty string for invalid letters
  }
  return (char*) pgm_read_word(&morseTable[letter - 'A'].code);
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
  if (potValue < 1023/2 - 50) {
    if (currentMode != LEARN) Serial.println("Entering learn mode...");
    currentMode = LEARN;
  }
  else {
    if (currentMode != TEST) Serial.println("Entering test mode...");
    currentMode = TEST;
  }
}