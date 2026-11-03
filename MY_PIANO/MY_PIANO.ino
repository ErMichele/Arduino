#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// --- PIN DEFINITIONS ---
const uint8_t BUZZER_PIN    = 2;
const uint8_t JOY_SW_PIN    = 3;
const uint8_t SD_CS_PIN     = 4;
const uint8_t JOY_X_PIN     = A0; // Horizontal: Mode Switching
const uint8_t JOY_Y_PIN     = A1; // Vertical: File Scrolling

const uint8_t NOTE_PINS[7]  = {5, 6, 7, 8, 9, 10, A2};
const uint16_t NOTE_FREQS[7] = {262, 294, 330, 349, 392, 440, 494}; // C4, D4, E4, F4, G4, A4, B4
const char* NOTE_NAMES[7]   = {"C4", "D4", "E4", "F4", "G4", "A4", "B4"};

enum Mode { INSTANT_PLAY, RECORDING, SAVED_PLAYBACK };

// --- CLASS DEFINITIONS ---

class DisplayController {
private:
  LiquidCrystal_I2C lcd;
public:
  DisplayController() : lcd(0x27, 16, 2) {}

  void init() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    showText("Arduino Piano", "Initializing...");
  }

  void showText(const char* line1, const char* line2) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(line1);
    lcd.setCursor(0, 1);
    lcd.print(line2);
  }
};

class Keyboard {
private:
  const uint8_t* pins;
  const uint16_t* freqs;
  uint8_t count;

public:
  Keyboard(const uint8_t* p, const uint16_t* f, uint8_t c) : pins(p), freqs(f), count(c) {}

  void init() {
    for (uint8_t i = 0; i < count; i++) {
      pinMode(pins[i], INPUT);
    }
  }

  int getPressedNoteIndex() {
    for (uint8_t i = 0; i < count; i++) {
      if (digitalRead(pins[i]) == HIGH) {
        return i;
      }
    }
    return -1;
  }

  uint16_t getFrequency(uint8_t index) {
    if (index < count) return freqs[index];
    return 0;
  }
};

class StorageManager {
private:
  uint8_t csPin;

public:
  StorageManager(uint8_t cs) : csPin(cs) {}

  bool init() {
    return SD.begin(csPin);
  }

  // Scans files and removes any that do not follow the naming format "melodyX.txt"
  void cleanInvalidFiles() {
    File root = SD.open("/");
    while (true) {
      File entry = root.openNextFile();
      if (!entry) break;

      String name = entry.name();
      entry.close();

      name.toLowerCase();
      if (!entry.isDirectory()) {
        if (!name.startsWith("melody") || !name.endsWith(".txt")) {
          SD.remove((char*)name.c_str());
        }
      }
    }
    root.close();
  }

  uint8_t getNextMelodyNumber() {
    uint8_t index = 1;
    while (true) {
      String fileName = "melody" + String(index) + ".txt";
      if (!SD.exists(fileName)) {
        return index;
      }
      index++;
    }
  }

  void saveNote(const String& filename, uint16_t freq, unsigned long duration) {
    File file = SD.open(filename, FILE_WRITE);
    if (file) {
      file.print(freq);
      file.print(",");
      file.println(duration);
      file.close();
    }
  }

  void playMelody(const String& filename, uint8_t buzzerPin) {
    File file = SD.open(filename, FILE_READ);
    if (!file) return;

    while (file.available()) {
      String line = file.readStringUntil('\n');
      line.trim();
      if (line.length() == 0) continue;

      int commaIndex = line.indexOf(',');
      if (commaIndex != -1) {
        uint16_t freq = line.substring(0, commaIndex).toInt();
        unsigned long duration = line.substring(commaIndex + 1).toInt();

        if (freq > 0) {
          tone(buzzerPin, freq);
          delay(duration);
          noTone(buzzerPin);
        } else {
          delay(duration);
        }
        delay(50); // Small pause between notes
      }
    }
    file.close();
  }
};

// --- GLOBAL SYSTEM OBJECTS ---
DisplayController display;
Keyboard keyboard(NOTE_PINS, NOTE_FREQS, 7);
StorageManager storage(SD_CS_PIN);

Mode currentMode = INSTANT_PLAY;
uint8_t selectedMelodyIndex = 1;
bool isRecording = false;
String currentRecFile = "";
unsigned long noteStartTime = 0;
int lastNotePressed = -1;

void setup() {
  pinMode(JOY_SW_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  display.init();
  keyboard.init();

  if (!storage.init()) {
    display.showText("SD Card Error!", "Check Connections");
    while (true); // Halt execution if SD card fails
  }

  storage.cleanInvalidFiles();
  delay(1000);
  display.showText("Mode: Instant", "Play Notes");
}

void loop() {
  int xVal = analogRead(JOY_X_PIN);
  int yVal = analogRead(JOY_Y_PIN);
  static unsigned long lastNavTime = 0;

  // Debounce filter for joystick navigation
  if (millis() - lastNavTime > 300) {
    
    // --- HORIZONTAL (X-AXIS): SWITCH MODES ---
    if (xVal < 300) { // Left -> Instant Play Mode
      currentMode = INSTANT_PLAY;
      display.showText("Mode: Instant", "Play Notes");
      lastNavTime = millis();
    } 
    else if (xVal > 700) { // Right -> Saved Melody Mode
      currentMode = SAVED_PLAYBACK;
      String fileToDisplay = "melody" + String(selectedMelodyIndex) + ".txt";
      display.showText("Mode: Saved", fileToDisplay.c_str());
      lastNavTime = millis();
    }

    // --- VERTICAL (Y-AXIS): CHANGE FILES (Only in Saved Playback Mode) ---
    if (currentMode == SAVED_PLAYBACK) {
      if (yVal < 300) { // Up -> Previous File
        if (selectedMelodyIndex > 1) {
          selectedMelodyIndex--;
          String fileToDisplay = "melody" + String(selectedMelodyIndex) + ".txt";
          display.showText("Mode: Saved", fileToDisplay.c_str());
          lastNavTime = millis();
        }
      } 
      else if (yVal > 700) { // Down -> Next File (if it exists)
        String nextFile = "melody" + String(selectedMelodyIndex + 1) + ".txt";
        if (SD.exists(nextFile)) {
          selectedMelodyIndex++;
          display.showText("Mode: Saved", nextFile.c_str());
          lastNavTime = millis();
        }
      }
    }
  }

  // --- JOYSTICK SWITCH CLICK ---
  static bool lastSwState = HIGH;
  bool swState = digitalRead(JOY_SW_PIN);
  if (lastSwState == HIGH && swState == LOW) { // Button Press Event
    delay(50); // Debounce
    if (currentMode == INSTANT_PLAY) {
      // Toggle Recording Mode
      if (!isRecording) {
        isRecording = true;
        uint8_t newNum = storage.getNextMelodyNumber();
        currentRecFile = "melody" + String(newNum) + ".txt";
        display.showText("REC Started...", currentRecFile.c_str());
      } else {
        isRecording = false;
        display.showText("REC Saved!", currentRecFile.c_str());
        delay(1000);
        display.showText("Mode: Instant", "Play Notes");
      }
    } else if (currentMode == SAVED_PLAYBACK) {
      String fileToPlay = "melody" + String(selectedMelodyIndex) + ".txt";
      if (SD.exists(fileToPlay)) {
        display.showText("Playing...", fileToPlay.c_str());
        storage.playMelody(fileToPlay, BUZZER_PIN);
        display.showText("Mode: Saved", fileToPlay.c_str());
      } else {
        display.showText("File Not Found", fileToPlay.c_str());
      }
    }
  }
  lastSwState = swState;

  // --- NOTE PLAYING LOGIC (Instant Mode) ---
  if (currentMode == INSTANT_PLAY) {
    int noteIndex = keyboard.getPressedNoteIndex();

    if (noteIndex != -1) {
      if (lastNotePressed == -1) {
        // New note pressed
        lastNotePressed = noteIndex;
        noteStartTime = millis();
        tone(BUZZER_PIN, keyboard.getFrequency(noteIndex));
      }
    } else {
      if (lastNotePressed != -1) {
        // Note released
        noTone(BUZZER_PIN);
        unsigned long duration = millis() - noteStartTime;

        if (isRecording && duration > 20) {
          storage.saveNote(currentRecFile, keyboard.getFrequency(lastNotePressed), duration);
        }
        lastNotePressed = -1;
      }
    }
  }
}