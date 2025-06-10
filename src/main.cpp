#include <Arduino.h>
#include <avr/pgmspace.h> // For storing strings in program memory
#include <LiquidCrystal.h>

//Arduino pin definations
#define MORSE_IN 8
#define MODE_SELECT_PIN A0

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

// LCD pin configuration: RS, E, D4, D5, D6, D7
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

void setup() {
  Serial.begin(9600);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MORSE_IN, INPUT);
  lcd.begin(16, 2); // Initialize a 16x2 LCD
  lcd.clear();
}

void loop() {
  String input = "";
  Mode previousMode = currentMode; // Track previous mode

  // Get an english letter and it's morse representation
  char letter = nextLetter();
  String morseCode = getMorseEntry(letter);

  // construct display string based on mode
  String displayString = String(letter);
  if (currentMode == LEARN) {
    displayString += ": " + morseCode;
  }
  // Display the target on the LCD (first line)
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(displayString);

  while (input != morseCode) {
    modeSelect();
    if (currentMode != previousMode) {
      break; // Exit the while loop if mode changes
    }
    char buttonPress = detectButtonPress();
    // Show user input on second line
    lcd.setCursor(0, 1);
    lcd.print("                "); // Clear second line
    lcd.setCursor(0, 1);
    lcd.print(input);

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
      
      if (input == morseCode) {
        Serial.println(" : Correct input!");
        // Optionally, you can show a success message on the LCD here
        // lcd.setCursor(0, 1);
        // lcd.print("Correct!");
        input = ""; // Reset input after correct entry
        morseCode = ""; // Reset morseCode to empty to trigger while loop exit 
      }
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
  // Now handled by LCD logic above, so this can be empty or used for debug
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