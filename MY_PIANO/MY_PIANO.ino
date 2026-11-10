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
 * 5. I2C LCD DISPLAY (16x2 with I2C Backpack):
 *    - SDA -> Analog Pin A4 (SDA)
 *    - SCL -> Analog Pin A5 (SCL)
 *    - VCC -> 5V
 *    - GND -> GND
 * ========================================================== */

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>
#include "pitches.h"

// --- Pin Definitions ---
const int BUZZER_PIN = 2;
const int BUTTON_PINS[] = {3, 5, 6, 7, 8, 9, 10};
const int NUM_BUTTONS = 7;
const int SD_CS_PIN = 4;
const int JOYSTICK_SW = A0;
const int JOYSTICK_Y = A1;
const int JOYSTICK_X = A2;

// --- EEPROM Memory Addresses ---
const int EEPROM_ADDR_MODE = 0;
const int EEPROM_ADDR_SONG = 1;

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

public:
  DisplayManager() : lcd(0x27, 16, 2) {}

  void begin() {
    Serial.println(F("[Display] Initializing LCD display..."));
    lcd.init();
    lcd.backlight();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Arduino Piano"));
    lcd.setCursor(0, 1);
    lcd.print(F("Initializing..."));
    delay(1500);
    Serial.println(F("[Display] LCD initialized successfully."));
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
    Serial.println(F("[SD] Initializing SD Card SPI interface..."));
    bool success = SD.begin(SD_CS_PIN);
    if (success) {
      Serial.println(F("[SD] Card initialized successfully."));
      cleanNonMelodyFiles();
    } else {
      Serial.println(F("[SD] ERROR: Initialization FAILED! Check wiring and format."));
    }
    return success;
  }

  void cleanNonMelodyFiles() {
    Serial.println(F("[SD] Scanning root directory to clean non-melody files..."));
    File root = SD.open("/");
    int cleanedCount = 0;
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
          cleanedCount++;
          Serial.print(F("[SD] Removed unauthorized file: "));
          Serial.println(filename);
        }
      } else {
        entry.close();
      }
    }
    root.close();
    Serial.print(F("[SD] Cleanup complete. Removed files count: "));
    Serial.println(cleanedCount);
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
    Serial.print(F("[SD] Saving melody file: "));
    Serial.println(filename);
    
    if (SD.exists(filename)) {
      Serial.println(F("[SD] File already exists, overwriting..."));
      SD.remove(filename);
    }
    
    currentFile = SD.open(filename, FILE_WRITE);
    if (!currentFile) {
      Serial.println(F("[SD] ERROR: Failed to open file for writing!"));
      return false;
    }

    for (int i = 0; i < count; i++) {
      currentFile.print(notes[i]);
      currentFile.print(',');
      currentFile.println(durations[i]);
    }
    currentFile.close();
    Serial.print(F("[SD] Successfully saved "));
    Serial.print(count);
    Serial.println(F(" notes to SD card."));
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
    int oldOctave = 4 + octaveShift;
    octaveShift = constrain(octaveShift + delta, -2, 2);
    Serial.print(F("[Piano Synth] Octave shifted from "));
    Serial.print(oldOctave);
    Serial.print(F(" to: "));
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

    Serial.print(F("[Piano Synth] Playing Note -> Index: "));
    Serial.print(buttonIndex);
    Serial.print(F(", Name: "));
    Serial.print(NOTE_NAMES[buttonIndex]);
    Serial.print(F(", Freq: "));
    Serial.print(freq);
    Serial.print(F(" Hz, Octave: "));
    Serial.println(4 + octaveShift);

    tone(BUZZER_PIN, (int)freq);
    disp.showPlayScreen(NOTE_NAMES[buttonIndex], 4 + octaveShift, currentMode);

    if (isRecording && recordCount < 50) {
      unsigned long now = millis();
      if (recordCount > 0) {
        recordedDurations[recordCount - 1] = (uint16_t)(now - lastNoteTime);
      }
      recordedNotes[recordCount] = (uint16_t)freq;
      lastNoteTime = now;
      recordCount++;
      Serial.print(F("[Record Buffer] Note added to memory. Freq: "));
      Serial.print(freq);
      Serial.print(F(" Hz | Total Recorded Notes: "));
      Serial.println(recordCount);
    } else if (isRecording && recordCount >= 50) {
      Serial.println(F("[Record Buffer] WARNING: Recording buffer full (max 50 notes)!"));
    }
  }

  void stopNote() {
    noTone(BUZZER_PIN);
  }

  void startRecording() {
    recordCount = 0;
    recordStartTime = millis();
    lastNoteTime = recordStartTime;
    isRecording = true;
    Serial.println(F("[Record Session] STARTED new recording session. Timers reset."));
  }

  void stopRecording() {
    isRecording = false;
    Serial.println(F("[Record Session] STOPPED recording session."));
    if (recordCount > 0) {
      recordedDurations[recordCount - 1] = 250; 
    }
    char filename[15];
    sdMgr.getNextMelodyFilename(filename, sizeof(filename));
    sdMgr.saveMelody(filename, recordedNotes, recordedDurations, recordCount);
  }

  bool getIsRecording() { return isRecording; }
  int getRecordCount() { return recordCount; }
  unsigned long getRecordDuration() { return millis() - recordStartTime; }

  void playMelodyFromFile(const char* filename) {
    Serial.print(F("[Playback Engine] Starting melody playback from file: "));
    Serial.println(filename);

    File f = SD.open(filename);
    if (!f) {
      Serial.println(F("[Playback Engine] ERROR: Failed to open melody file for reading!"));
      return;
    }

    int noteIndex = 0;
    while (f.available()) {
      String line = f.readStringUntil('\n');
      int commaIndex = line.indexOf(',');
      if (commaIndex > 0) {
        int freq = line.substring(0, commaIndex).toInt();
        int duration = line.substring(commaIndex + 1).toInt();
        
        Serial.print(F("[Playback Engine] Note #"));
        Serial.print(noteIndex++);
        Serial.print(F(" -> Freq: "));
        Serial.print(freq);
        Serial.print(F(" Hz, Duration: "));
        Serial.print(duration);
        Serial.println(F(" ms"));

        tone(BUZZER_PIN, freq, duration);
        delay(duration + 30);
      }
    }
    f.close();
    Serial.println(F("[Playback Engine] Finished playing melody successfully."));
  }
};

PianoSynth piano;

// ==========================================
// SETUP AND MAIN LOOP
// ==========================================
void setup() {
  Serial.begin(9600);
  while (!Serial); 
  Serial.println(F("=========================================="));
  Serial.println(F("--- Arduino Piano Boot Sequence Started ---"));
  Serial.println(F("=========================================="));

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(JOYSTICK_SW, INPUT_PULLUP);
  Serial.println(F("[Setup] Buzzer pin configured as OUTPUT."));
  Serial.println(F("[Setup] Joystick switch configured as INPUT_PULLUP."));

  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLUP);
    Serial.print(F("[Setup] Piano Button pin configured: "));
    Serial.println(BUTTON_PINS[i]);
  }

  displayMgr.begin();
  sdMgr.begin();

  // --- Restore Mode from EEPROM ---
  byte storedMode = EEPROM.read(EEPROM_ADDR_MODE);
  if (storedMode == MODE_PLAY || storedMode == MODE_RECORD || storedMode == MODE_MEMORY) {
    currentMode = (Mode)storedMode;
    Serial.print(F("[EEPROM] Restored saved mode: "));
    if (currentMode == MODE_PLAY) Serial.println(F("MODE_PLAY"));
    else if (currentMode == MODE_RECORD) Serial.println(F("MODE_RECORD"));
    else if (currentMode == MODE_MEMORY) Serial.println(F("MODE_MEMORY"));
  } else {
    currentMode = MODE_PLAY;
    EEPROM.update(EEPROM_ADDR_MODE, currentMode);
    Serial.println(F("[EEPROM] No valid mode found in EEPROM. Defaulted to MODE_PLAY."));
  }
  
  Serial.println(F("=========================================="));
  Serial.println(F("--- Setup Complete. Entering Main Loop ---"));
  Serial.println(F("=========================================="));
}

void loop() {
  int xVal = analogRead(JOYSTICK_X);
  int yVal = analogRead(JOYSTICK_Y);
  bool swPressed = (digitalRead(JOYSTICK_SW) == LOW);

  // --- Circular Mode Switching via X-axis ---
  static unsigned long lastModeChange = 0;
  if (millis() - lastModeChange > 400) {
    if (xVal > 800) { // Joystick Right
      Mode oldMode = currentMode;
      if (currentMode == MODE_PLAY) currentMode = MODE_RECORD;
      else if (currentMode == MODE_RECORD) currentMode = MODE_MEMORY;
      else if (currentMode == MODE_MEMORY) currentMode = MODE_PLAY;
      
      // Save new mode to EEPROM
      EEPROM.update(EEPROM_ADDR_MODE, (byte)currentMode);

      Serial.print(F("[Mode Switch] Switched Right -> Mode changed from "));
      Serial.print(oldMode);
      Serial.print(F(" to new Mode: "));
      if (currentMode == MODE_PLAY) Serial.println(F("MODE_PLAY"));
      else if (currentMode == MODE_RECORD) Serial.println(F("MODE_RECORD"));
      else if (currentMode == MODE_MEMORY) Serial.println(F("MODE_MEMORY"));

      lastModeChange = millis();
    } 
    else if (xVal < 200) { // Joystick Left
      Mode oldMode = currentMode;
      if (currentMode == MODE_PLAY) currentMode = MODE_MEMORY;
      else if (currentMode == MODE_MEMORY) currentMode = MODE_RECORD;
      else if (currentMode == MODE_RECORD) currentMode = MODE_PLAY;
      
      // Save new mode to EEPROM
      EEPROM.update(EEPROM_ADDR_MODE, (byte)currentMode);

      Serial.print(F("[Mode Switch] Switched Left -> Mode changed from "));
      Serial.print(oldMode);
      Serial.print(F(" to new Mode: "));
      if (currentMode == MODE_PLAY) Serial.println(F("MODE_PLAY"));
      else if (currentMode == MODE_RECORD) Serial.println(F("MODE_RECORD"));
      else if (currentMode == MODE_MEMORY) Serial.println(F("MODE_MEMORY"));

      lastModeChange = millis();
    }
  }

  // --- Mode Handling ---
  if (currentMode == MODE_PLAY) {
    static unsigned long lastOctaveChange = 0;
    if (millis() - lastOctaveChange > 400) {
      if (yVal > 800) {
        Serial.println(F("[Joystick Y] Pushed Up -> Increasing Octave"));
        piano.changeOctave(1);
        lastOctaveChange = millis();
      } else if (yVal < 200) {
        Serial.println(F("[Joystick Y] Pushed Down -> Decreasing Octave"));
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
      Serial.println(F("[Joystick Switch] Button pressed in RECORD mode."));
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
    static int selectedSongIndex = -1;
    static int lastLoadedIndex = -1;
    static char cachedFilename[13] = "";
    static bool initMemory = true;

    // Load/Validate song index from EEPROM on entry or file count change
    if (initMemory) {
      int storedSong = EEPROM.read(EEPROM_ADDR_SONG);
      if (totalSongs > 0) {
        if (storedSong >= totalSongs) {
          // Nearest smaller valid index fallback if file is missing/deleted
          selectedSongIndex = totalSongs - 1;
          Serial.print(F("[EEPROM] Saved song index out of range. Falling back to nearest smaller index: "));
          Serial.println(selectedSongIndex);
        } else {
          selectedSongIndex = storedSong;
          Serial.print(F("[EEPROM] Loaded saved song index: "));
          Serial.println(selectedSongIndex);
        }
      } else {
        selectedSongIndex = 0;
      }
      initMemory = false;
    }

    if (totalSongs > 0 && selectedSongIndex >= totalSongs) {
      selectedSongIndex = totalSongs - 1;
      Serial.print(F("[Memory] Song index adjusted to nearest smaller valid index: "));
      Serial.println(selectedSongIndex);
    }

    bool indexChanged = false;
    if (yVal > 800 && totalSongs > 0) {
      selectedSongIndex = (selectedSongIndex + 1) % totalSongs;
      indexChanged = true;
      Serial.print(F("[Joystick Y] Pushed Up in MEMORY mode -> Selected song index: "));
      Serial.println(selectedSongIndex);
      delay(300);
    } else if (yVal < 200 && totalSongs > 0) {
      selectedSongIndex = (selectedSongIndex - 1 + totalSongs) % totalSongs;
      indexChanged = true;
      Serial.print(F("[Joystick Y] Pushed Down in MEMORY mode -> Selected song index: "));
      Serial.println(selectedSongIndex);
      delay(300);
    }

    // Update filename and display ONLY once when index changes (prevents infinite printing spam)
    if (indexChanged || selectedSongIndex != lastLoadedIndex) {
      if (totalSongs > 0) {
        sdMgr.getMelodyFilename(selectedSongIndex, cachedFilename, sizeof(cachedFilename));
        EEPROM.update(EEPROM_ADDR_SONG, selectedSongIndex);
        Serial.print(F("[EEPROM] Saved song index "));
        Serial.print(selectedSongIndex);
        Serial.print(F(" (File: "));
        Serial.print(cachedFilename);
        Serial.println(F(") to EEPROM."));
      } else {
        strcpy(cachedFilename, "No Files");
      }
      displayMgr.showMemoryScreen(cachedFilename, selectedSongIndex, totalSongs);
      lastLoadedIndex = selectedSongIndex;
    } else if (totalSongs == 0) {
      strcpy(cachedFilename, "No Files");
      displayMgr.showMemoryScreen(cachedFilename, 0, 0);
    }

    if (swPressed && totalSongs > 0) {
      Serial.print(F("[Joystick Switch] Playing selected melody file: "));
      Serial.println(cachedFilename);
      piano.playMelodyFromFile(cachedFilename);
      displayMgr.showMemoryScreen(cachedFilename, selectedSongIndex, totalSongs);
      delay(300);
    }
  } else {
    static bool initMemoryReset = false;
  }
}