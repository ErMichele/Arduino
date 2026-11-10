/* ==========================================================
 * WIRING CONNECTIONS (Arduino Uno / Nano Pinout):
 * ==========================================================
 * 1. BUZZER:
 *    - Positive (Anode / Signal) -> Digital Pin 2 (BUZZER_PIN)
 *    - Negative (Cathode / GND)  -> GND (recommended with 100Ω resistor)
 *
 * 2. PUSH BUTTONS (7 Piano Keys, using internal pullups):
 *    - Button 1 (Note C4) -> Digital Pin 3 & GND
 *    - Button 2 (Note D4) -> Digital Pin 5 & GND
 *    - Button 3 (Note E4) -> Digital Pin 6 & GND
 *    - Button 4 (Note F4) -> Digital Pin 7 & GND
 *    - Button 5 (Note G4) -> Digital Pin 8 & GND
 *    - Button 6 (Note A4) -> Digital Pin 9 & GND
 *    - Button 7 (Note B4) -> Digital Pin 10 & GND
 *
 * 3. SD CARD MODULE (SPI Interface):
 *    - CS   -> Digital Pin 4 (SD_CS_PIN)
 *    - MOSI -> Digital Pin 11 (MOSI)
 *    - MISO -> Digital Pin 12 (MISO)
 *    - SCK  -> Digital Pin 13 (SCK)
 *    - VCC  -> 5V (or 3.3V depending on module specs)
 *    - GND  -> GND
 *
 * 4. JOYSTICK MODULE:
 *    - SW (Button) -> Analog Pin A0 (JOYSTICK_SW) [Input Pullup]
 *    - VRy (Y-axis)-> Analog Pin A1 (JOYSTICK_Y)  [Octave Shift / Song Select]
 *    - VRx (X-axis)-> Analog Pin A2 (JOYSTICK_X)  [Mode Switching]
 *    - VCC         -> 5V
 *    - GND         -> GND
 *
 * 5. LED INDICATOR:
 *    - Anode (+)   -> Analog Pin A3 (LED_PIN) through 220Ω resistor
 *    - Cathode (-) -> GND
 *
 * 6. I2C LCD DISPLAY (16x2 with I2C Backpack):
 *    - SDA -> Analog Pin A4 (SDA)
 *    - SCL -> Analog Pin A5 (SCL)
 *    - VCC -> 5V
 *    - GND -> GND
 * ========================================================== */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include "pitches.h"

// --- Pin Definitions ---
const int BUZZER_PIN = 2;
const int BUTTON_PINS[] = {3, 5, 6, 7, 8, 9, 10};
const int NUM_BUTTONS = 7;
const int SD_CS_PIN = 4;
const int JOYSTICK_SW = A0;
const int JOYSTICK_Y = A1;
const int JOYSTICK_X = A2;
const int LED_PIN = A3;

// --- Base Scale Frequencies (Octave 4) ---
const int BASE_NOTES[NUM_BUTTONS] = {
  NOTE_C4, NOTE_D4, NOTE_E4, NOTE_F4, NOTE_G4, NOTE_A4, NOTE_B4
};

const char* NOTE_NAMES[NUM_BUTTONS] = {
  "C4", "D4", "E4", "F4", "G4", "A4", "B4"
};

// --- System States ---
enum Mode { MODE_MEMORY, MODE_PLAY, MODE_RECORD };
Mode currentMode = MODE_PLAY;

class DisplayManager;
class SDManager;

// ==========================================
// 1. DISPLAY MANAGER CLASS (LCD Handler)
// ==========================================
class DisplayManager {
private:
  LiquidCrystal_I2C lcd;
  String lastLine1 = "";
  String lastLine2 = "";

public:
  DisplayManager() : lcd(0x27, 16, 2) {}

  void begin() {
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Arduino Piano"));
    lcd.setCursor(0, 1);
    lcd.print(F("Initializing..."));
    delay(1500);
  }

  void showPlayScreen(const char* noteName, int octave, Mode mode) {
    lcd.setCursor(0, 0);
    lcd.print(F("Mode: PLAY       "));
    lcd.setCursor(0, 1);
    lcd.print(F("Note: "));
    lcd.print(noteName);
    lcd.print(F(" Oct:"));
    lcd.print(octave);
    lcd.print(F("  "));
  }

  void showRecordScreen(unsigned long durationMs, int noteCount) {
    lcd.setCursor(0, 0);
    lcd.print(F("Mode: RECORD     "));
    lcd.setCursor(0, 1);
    lcd.print(F("T:"));
    lcd.print(durationMs / 1000);
    lcd.print(F("s Notes:"));
    lcd.print(noteCount);
    lcd.print(F("   "));
  }

  void showMemoryScreen(const char* melodyName, int index, int total) {
    lcd.setCursor(0, 0);
    lcd.print(F("Mode: MEMORY     "));
    lcd.setCursor(0, 1);
    lcd.print(index + 1);
    lcd.print(F("/"));
    lcd.print(total);
    lcd.print(F(" "));
    lcd.print(melodyName);
    lcd.print(F("   "));
  }
};

// ==========================================
// 2. SD MANAGER CLASS (File & CSV Handler)
// ==========================================
class SDManager {
private:
  File currentFile;

public:
  bool begin() {
    bool success = SD.begin(SD_CS_PIN);
    if (success) {
      Serial.println(F("[SD] Card initialized successfully."));
      cleanNonMelodyFiles();
    } else {
      Serial.println(F("[SD] Initialization FAILED!"));
    }
    return success;
  }

  void cleanNonMelodyFiles() {
    Serial.println(F("[SD] Cleaning non-melody files..."));
    File root = SD.open("/");
    while (true) {
      File entry = root.openNextFile();
      if (!entry) break;
      if (!entry.isDirectory()) {
        char filename[13];
        strcpy(filename, entry.name());
        String name = String(filename);
        name.toLowerCase();
        entry.close();

        bool isMelody = false;
        if (name.startsWith("melody_") && name.endsWith(".csv")) {
          int startIdx = 7;
          int endIdx = name.lastIndexOf('.');
          if (endIdx > startIdx) {
            String numStr = name.substring(startIdx, endIdx);
            bool isNumeric = true;
            for (unsigned int i = 0; i < numStr.length(); i++) {
              if (!isDigit(numStr[i])) { isNumeric = false; break; }
            }
            if (isNumeric && numStr.length() > 0) isMelody = true;
          }
        }

        if (!isMelody) {
          SD.remove(filename);
          Serial.print(F("[SD] Removed unauthorized file: "));
          Serial.println(filename);
        }
      } else {
        entry.close();
      }
    }
    root.close();
    Serial.println(F("[SD] Cleanup complete."));
  }

  void getNextMelodyFilename(char* buffer, size_t maxLen) {
    int maxNum = 0;
    File root = SD.open("/");
    while (true) {
      File entry = root.openNextFile();
      if (!entry) break;
      if (!entry.isDirectory()) {
        String name = String(entry.name());
        name.toLowerCase();
        if (name.startsWith("melody_") && name.endsWith(".csv")) {
          int startIdx = 7;
          int endIdx = name.lastIndexOf('.');
          if (endIdx > startIdx) {
            int num = name.substring(startIdx, endIdx).toInt();
            if (num > maxNum) maxNum = num;
          }
        }
      }
      entry.close();
    }
    root.close();

    snprintf(buffer, maxLen, "melody_%d.csv", maxNum + 1);
    Serial.print(F("[SD] Next melody filename generated: "));
    Serial.println(buffer);
  }

  bool saveMelody(const char* filename, uint16_t* notes, uint16_t* durations, int count) {
    if (SD.exists(filename)) {
      SD.remove(filename);
    }
    currentFile = SD.open(filename, FILE_WRITE);
    if (!currentFile) {
      Serial.println(F("[SD] Error opening file for writing!"));
      return false;
    }

    for (int i = 0; i < count; i++) {
      currentFile.print(notes[i]);
      currentFile.print(',');
      currentFile.println(durations[i]);
    }
    currentFile.close();
    Serial.print(F("[SD] Successfully saved melody to: "));
    Serial.println(filename);
    return true;
  }

  int getMelodyCount() {
    File root = SD.open("/");
    int count = 0;
    while (true) {
      File entry = root.openNextFile();
      if (!entry) break;
      if (!entry.isDirectory()) {
        String name = String(entry.name());
        if (name.endsWith(".CSV") || name.endsWith(".csv")) {
          count++;
        }
      }
      entry.close();
    }
    root.close();
    return count;
  }

  void getMelodyFilename(int index, char* buffer, size_t maxLen) {
    File root = SD.open("/");
    int count = 0;
    buffer[0] = '\0';
    while (true) {
      File entry = root.openNextFile();
      if (!entry) break;
      if (!entry.isDirectory()) {
        String name = String(entry.name());
        if (name.endsWith(".CSV") || name.endsWith(".csv")) {
          if (count == index) {
            name.toCharArray(buffer, maxLen);
            entry.close();
            break;
          }
          count++;
        }
      }
      entry.close();
    }
    root.close();
  }
};

DisplayManager displayMgr;
SDManager sdMgr;

// ==========================================
// 3. PIANO SYNTH / RECORDER CLASS
// ==========================================
class PianoSynth {
private:
  int octaveShift;
  uint16_t recordedNotes[50];
  uint16_t recordedDurations[50];
  int recordCount;
  unsigned long recordStartTime;
  unsigned long lastNoteTime;
  bool isRecording;

public:
  PianoSynth() {
    octaveShift = 0;
    recordCount = 0;
    isRecording = false;
  }

  void changeOctave(int delta) {
    octaveShift = constrain(octaveShift + delta, -2, 2);
    Serial.print(F("[Piano] Octave shifted to: "));
    Serial.println(4 + octaveShift);
  }

  int getOctave() {
    return 4 + octaveShift;
  }

  void playNote(int buttonIndex, DisplayManager &disp) {
    int baseFreq = BASE_NOTES[buttonIndex];
    long freq = baseFreq;

    if (octaveShift > 0) {
      freq = baseFreq * (1L << octaveShift);
    } else if (octaveShift < 0) {
      freq = baseFreq >> (-octaveShift);
    }

    tone(BUZZER_PIN, (int)freq);
    
    // LED Control & Debug
    digitalWrite(LED_PIN, HIGH);

    disp.showPlayScreen(NOTE_NAMES[buttonIndex], 4 + octaveShift, currentMode);

    if (isRecording && recordCount < 50) {
      unsigned long now = millis();
      if (recordCount > 0) {
        recordedDurations[recordCount - 1] = (uint16_t)(now - lastNoteTime);
      }
      recordedNotes[recordCount] = (uint16_t)freq;
      lastNoteTime = now;
      recordCount++;
      Serial.print(F("[Record] Note added: "));
      Serial.print(freq);
      Serial.print(F(" Hz, Count: "));
      Serial.println(recordCount);
    }
  }

  void stopNote() {
    noTone(BUZZER_PIN);
    digitalWrite(LED_PIN, LOW);
  }

  void startRecording() {
    recordCount = 0;
    recordStartTime = millis();
    lastNoteTime = recordStartTime;
    isRecording = true;
    Serial.println(F("[Record] Started recording session."));
  }

  void stopRecording() {
    isRecording = false;
    if (recordCount > 0) {
      recordedDurations[recordCount - 1] = 250; 
    }
    char filename[15];
    sdMgr.getNextMelodyFilename(filename, sizeof(filename));
    sdMgr.saveMelody(filename, recordedNotes, recordedDurations, recordCount);
    Serial.println(F("[Record] Stopped and saved recording."));
  }

  bool getIsRecording() { return isRecording; }
  int getRecordCount() { return recordCount; }
  unsigned long getRecordDuration() { return millis() - recordStartTime; }

  void playMelodyFromFile(const char* filename) {
    Serial.print(F("[Player] Playing melody from file: "));
    Serial.println(filename);

    File f = SD.open(filename);
    if (!f) {
      Serial.println(F("[Player] Failed to open file!"));
      return;
    }

    while (f.available()) {
      String line = f.readStringUntil('\n');
      int commaIndex = line.indexOf(',');
      if (commaIndex > 0) {
        int freq = line.substring(0, commaIndex).toInt();
        int duration = line.substring(commaIndex + 1).toInt();
        tone(BUZZER_PIN, freq, duration);
        digitalWrite(LED_PIN, HIGH);
        delay(duration + 30);
        digitalWrite(LED_PIN, LOW);
      }
    }
    f.close();
    Serial.println(F("[Player] Finished playing melody."));
  }
};

PianoSynth piano;

// ==========================================
// SETUP AND MAIN LOOP
// ==========================================
void setup() {
  Serial.begin(9600);
  while (!Serial); 
  Serial.println(F("--- Arduino Piano Booting ---"));

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(JOYSTICK_SW, INPUT_PULLUP);

  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
  }

  displayMgr.begin();
  sdMgr.begin();
  
  Serial.println(F("--- Setup Complete ---"));
}

void loop() {
  int xVal = analogRead(JOYSTICK_X);
  int yVal = analogRead(JOYSTICK_Y);
  bool swPressed = (digitalRead(JOYSTICK_SW) == LOW);

  // --- Circular Mode Switching via X-axis ---
  static unsigned long lastModeChange = 0;
  if (millis() - lastModeChange > 400) {
    if (xVal > 800) { // Joystick Right
      if (currentMode == MODE_PLAY) currentMode = MODE_RECORD;
      else if (currentMode == MODE_RECORD) currentMode = MODE_MEMORY;
      else if (currentMode == MODE_MEMORY) currentMode = MODE_PLAY;
      
      Serial.print(F("[Mode] Switched Right -> Current Mode: "));
      Serial.println(currentMode);
      lastModeChange = millis();
    } 
    else if (xVal < 200) { // Joystick Left
      if (currentMode == MODE_PLAY) currentMode = MODE_MEMORY;
      else if (currentMode == MODE_MEMORY) currentMode = MODE_RECORD;
      else if (currentMode == MODE_RECORD) currentMode = MODE_PLAY;
      
      Serial.print(F("[Mode] Switched Left -> Current Mode: "));
      Serial.println(currentMode);
      lastModeChange = millis();
    }
  }

  // --- Mode Handling ---
  if (currentMode == MODE_PLAY) {
    static unsigned long lastOctaveChange = 0;
    if (millis() - lastOctaveChange > 400) {
      if (yVal > 800) {
        piano.changeOctave(1);
        lastOctaveChange = millis();
      } else if (yVal < 200) {
        piano.changeOctave(-1);
        lastOctaveChange = millis();
      }
    }

    bool buttonPressed = false;
    for (int i = 0; i < NUM_BUTTONS; i++) {
      if (digitalRead(BUTTON_PINS[i]) == LOW) {
        piano.playNote(i, displayMgr);
        buttonPressed = true;
        break;
      }
    }
    if (!buttonPressed) {
      piano.stopNote();
      displayMgr.showPlayScreen("---", piano.getOctave(), currentMode);
    }
  } 
  else if (currentMode == MODE_RECORD) {
    static bool lastSwState = HIGH;
    if (swPressed && lastSwState == HIGH) {
      if (!piano.getIsRecording()) {
        piano.startRecording();
      } else {
        piano.stopRecording(); 
      }
      delay(300);
    }
    lastSwState = swPressed;

    bool buttonPressed = false;
    for (int i = 0; i < NUM_BUTTONS; i++) {
      if (digitalRead(BUTTON_PINS[i]) == LOW) {
        piano.playNote(i, displayMgr);
        buttonPressed = true;
        break;
      }
    }
    if (!buttonPressed) {
      piano.stopNote();
    }

    if (piano.getIsRecording()) {
      displayMgr.showRecordScreen(piano.getRecordDuration(), piano.getRecordCount());
    } else {
      displayMgr.showRecordScreen(0, 0);
    }
  } 
  else if (currentMode == MODE_MEMORY) {
    int totalSongs = sdMgr.getMelodyCount();
    static int selectedSongIndex = 0;

    if (yVal > 800 && totalSongs > 0) {
      selectedSongIndex = (selectedSongIndex + 1) % totalSongs;
      delay(300);
    } else if (yVal < 200 && totalSongs > 0) {
      selectedSongIndex = (selectedSongIndex - 1 + totalSongs) % totalSongs;
      delay(300);
    }

    char filename[13];
    if (totalSongs > 0) {
      sdMgr.getMelodyFilename(selectedSongIndex, filename, sizeof(filename));
    } else {
      strcpy(filename, "No Files");
    }
    
    displayMgr.showMemoryScreen(filename, selectedSongIndex, totalSongs);

    if (swPressed && totalSongs > 0) {
      piano.playMelodyFromFile(filename);
      delay(300);
    }
  }
}