#include <LiquidCrystal.h>
#include <EEPROM.h> // New: For saving data during power loss

// Pin Definitions
#define PIN_JOY_X     A1 
#define PIN_JOY_Y     A2 
#define PIN_JOY_BTN   2  
#define PIN_RELE      12
#define PIN_TEMP      A0

// EEPROM Address for the target temperature
const int ADDR_TARGET_TEMP = 0;

class Termostato {
  private:
    float currentTemp;
    float activeDesiredTemp;
    float editingTemp;
    float hysteresis; 
    bool relayState;
    int relayPin;
    bool isEditing;
    
    // --- Calibration & Failsafe Variables ---
    const float calibrationOffset = 0.0; // Change this based on your ice-bath test
    const float MAX_SAFE_TEMP = 50.0;    // Upper limit (Melons start cooking)
    const float MIN_SAFE_TEMP = 5.0;     // Lower limit (Sensor likely disconnected)
    bool sensorError = false;

    // Sensor Smoothing (EMA)
    float smoothedTemp;
    const float alpha = 0.15; 

    unsigned long lastSwitchTime;
    const unsigned long MIN_CYCLE_TIME = 10000; 

    float savedPresets[5] = {25.0, 28.0, 30.0, 35.0, 40.0};
    int currentPresetIndex = 0;

  public:
    Termostato(int pRelay, float startTemp, float hys) {
      relayPin = pRelay;
      activeDesiredTemp = startTemp;
      editingTemp = startTemp;
      hysteresis = hys;
      relayState = false;
      currentTemp = 0.0;
      smoothedTemp = 0.0;
      lastSwitchTime = 0;
      isEditing = false;
    }

    void init() {
      pinMode(relayPin, OUTPUT);
      digitalWrite(relayPin, LOW);
      pinMode(PIN_JOY_BTN, INPUT_PULLUP);
      
      // LOAD SAVED TEMP: If Arduino resets, it remembers where it was
      float storedTemp;
      EEPROM.get(ADDR_TARGET_TEMP, storedTemp);
      if (storedTemp > 0 && storedTemp < 100) { // Check if valid data exists
        activeDesiredTemp = storedTemp;
        editingTemp = storedTemp;
      }

      Serial.begin(9600);
      Serial.println("Time(ms),Target(C),Current(C),HeatingStatus,Error");
    }

    void updateSensor(float rawTemp) {
      // 1. APPLY CALIBRATION
      float calibratedTemp = rawTemp + calibrationOffset;

      // 2. FAILSAFE CHECK: Is the reading realistic?
      if (calibratedTemp > MAX_SAFE_TEMP || calibratedTemp < MIN_SAFE_TEMP) {
        sensorError = true;
      } else {
        sensorError = false;
      }

      // 3. SMOOTHING
      if (smoothedTemp == 0) smoothedTemp = calibratedTemp;
      smoothedTemp = (alpha * calibratedTemp) + ((1.0 - alpha) * smoothedTemp);
      currentTemp = smoothedTemp;
    }

    void updateControl() {
      unsigned long currentTime = millis();
      
      static unsigned long lastLog = 0;
      if (currentTime - lastLog > 2000) {
        logData();
        lastLog = currentTime;
      }

      // EMERGENCY SHUTOFF: Stop heating if sensor is broken/disconnected
      if (sensorError) {
        relayState = false;
        digitalWrite(relayPin, LOW);
        return; 
      }

      if (currentTime - lastSwitchTime < MIN_CYCLE_TIME) return;

      if (!relayState && currentTemp < (activeDesiredTemp - hysteresis)) {
        relayState = true;
        digitalWrite(relayPin, HIGH);
        lastSwitchTime = currentTime;
      } 
      else if (relayState && currentTemp > (activeDesiredTemp + hysteresis)) {
        relayState = false;
        digitalWrite(relayPin, LOW);
        lastSwitchTime = currentTime;
      }
    }

    void logData() {
      Serial.print(millis());
      Serial.print(","); Serial.print(activeDesiredTemp);
      Serial.print(","); Serial.print(currentTemp);
      Serial.print(","); Serial.print(relayState ? "1" : "0");
      Serial.print(","); Serial.println(sensorError ? "1" : "0");
    }

    void handleJoystick() {
      int xVal = analogRead(PIN_JOY_X);
      int yVal = analogRead(PIN_JOY_Y);
      bool btnPressed = (digitalRead(PIN_JOY_BTN) == LOW);

      if (xVal < 200) { editingTemp -= 0.5; isEditing = true; delay(200); }
      else if (xVal > 800) { editingTemp += 0.5; isEditing = true; delay(200); }

      if (yVal < 200) {
        currentPresetIndex = (currentPresetIndex + 1) % 5;
        editingTemp = savedPresets[currentPresetIndex];
        isEditing = true;
        delay(200);
      } else if (yVal > 800) {
        currentPresetIndex = (currentPresetIndex <= 0) ? 4 : currentPresetIndex - 1;
        editingTemp = savedPresets[currentPresetIndex];
        isEditing = true;
        delay(200);
      }

      if (btnPressed) {
        activeDesiredTemp = editingTemp;
        savedPresets[currentPresetIndex] = editingTemp;
        
        // SAVE TO EEPROM: Permanently store the new goal
        EEPROM.put(ADDR_TARGET_TEMP, activeDesiredTemp);
        
        isEditing = false;
        delay(500);
      }
    }

    float getCurrentTemp() { return currentTemp; }
    float getTargetTemp() { return isEditing ? editingTemp : activeDesiredTemp; }
    bool isHeating() { return relayState; }
    bool getEditingState() { return isEditing; }
    bool hasError() { return sensorError; }
};

LiquidCrystal lcd(7, 8, 9, 3, 4, 5);
Termostato stressTester(PIN_RELE, 28.0, 1.0);

void setup() {
  lcd.begin(16, 2);
  stressTester.init();
  
  lcd.setCursor(0, 0);
  lcd.print("MELON STRESS v2");
  lcd.setCursor(0, 1);
  lcd.print("LOG & SAFE LOAD");
  delay(2000);
  lcd.clear();
}

void loop() {
  // --- TEMPERATURE CALCULATION ---
  int raw = analogRead(PIN_TEMP);
  float voltage = raw * (5.0 / 1023.0);
  
  // CURRENT FORMULA (TMP36):
  float tempC = (voltage - 0.5) * 100.0; 
  
  stressTester.updateSensor(tempC);
  stressTester.handleJoystick();
  stressTester.updateControl();

  static unsigned long lastRefresh = 0;
  if (millis() - lastRefresh > 400) {
    updateDisplay();
    lastRefresh = millis();
  }
}

void updateDisplay() {
  lcd.setCursor(0, 0);
  if (stressTester.hasError()) {
    lcd.print("! SENSOR ERROR !");
  } else {
    lcd.print("CURR: ");
    lcd.print(stressTester.getCurrentTemp(), 1);
    lcd.write(0xDF);
    lcd.print("C      ");
  }

  lcd.setCursor(0, 1);
  lcd.print(stressTester.getEditingState() ? "EDIT: " : "GOAL: ");
  lcd.print(stressTester.getTargetTemp(), 1);
  lcd.write(0xDF);
  lcd.print("C ");
  
  if (stressTester.hasError()) {
    lcd.print("[OFF]");
  } else {
    lcd.print(stressTester.isHeating() ? "[ON]" : "    ");
  }
}