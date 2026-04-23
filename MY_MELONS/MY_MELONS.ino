#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h> 
#include <DHT.h> 

/**
 * PROGETTO: Termostato per Coltivazione Meloni (Versione I2C)
 * SCOPO: Stress Test termico con lampade/luci rosse
 * HARDWARE: Arduino Uno/Nano, LCD 16x2 I2C, Joystick, Relè, Sensore DHT11
 */

// --- CONFIGURAZIONE PIN ---
#define PIN_JOY_X     A3    // Asse X: Regolazione fine temperatura
#define PIN_JOY_Y     A2    // Asse Y: Selezione Preset
#define PIN_JOY_BTN   2     // Pulsante: Conferma e Salva
#define PIN_RELE      12    // Pin comando Relè (Luce Rossa/Riscaldatore)
#define PIN_DHT       A0    // Ingresso Sensore DHT11
#define DHTTYPE       DHT11 

// --- COSTANTI DI SISTEMA ---
const int ADDR_TARGET_TEMP = 0; 

DHT dht(PIN_DHT, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2); 

class Termostato {
  private:
    float currentTemp;        
    float currentHum;         
    float activeDesiredTemp;  
    float editingTemp;        
    float hysteresis;         
    bool relayState;          
    int relayPin;             
    bool isEditing;           
    
    // --- SICUREZZA ---
    const float MAX_SAFE_TEMP = 50.0; // Limite critico per stress test
    const float MIN_SAFE_TEMP = 5.0;
    bool sensorError = false;

    // --- PROTEZIONE RELÈ ---
    unsigned long lastSwitchTime;
    const unsigned long MIN_CYCLE_TIME = 5000; // 5 secondi minimi tra ON/OFF

    // --- PRESET VELOCI (Temperature tipiche per Stress Test) ---
    float savedPresets[5] = {25.0, 30.0, 35.0, 40.0, 45.0};
    int currentPresetIndex = 0;

  public:
    Termostato(int pRelay, float startTemp, float hys) {
      relayPin = pRelay;
      activeDesiredTemp = startTemp;
      editingTemp = startTemp;
      hysteresis = hys;
      relayState = false;
      currentTemp = 0.0;
      currentHum = 0.0;
      lastSwitchTime = 0;
      isEditing = false;
    }

    void init() {
      pinMode(relayPin, OUTPUT);
      digitalWrite(relayPin, LOW);
      pinMode(PIN_JOY_BTN, INPUT_PULLUP);
      
      dht.begin();

      float storedTemp;
      EEPROM.get(ADDR_TARGET_TEMP, storedTemp);
      
      if (!isnan(storedTemp) && storedTemp >= 10.0 && storedTemp <= 48.0) { 
        activeDesiredTemp = storedTemp;
        editingTemp = storedTemp;
      }

      Serial.begin(9600);
      Serial.println("--- LOG STRESS TEST MELONI ---");
      Serial.println("Tempo(ms), Temp(C), Umidita(%), Target(C), StatoRele");
    }

    void updateSensor() {
      float t = dht.readTemperature();
      float h = dht.readHumidity();

      // Verifica errori del sensore
      if (isnan(t) || isnan(h) || t > MAX_SAFE_TEMP || t < MIN_SAFE_TEMP) {
        sensorError = true;
      } else {
        sensorError = false;
        currentTemp = t;
        currentHum = h;
      }
    }

    void updateControl() {
      unsigned long currentTime = millis();
      
      // FAILSAFE: Se c'è un errore, spegniamo la luce rossa per sicurezza
      if (sensorError) {
        relayState = false;
        digitalWrite(relayPin, LOW);
        return; 
      }

      // Protezione cicli brevi
      if (currentTime - lastSwitchTime < MIN_CYCLE_TIME) return;

      // Logica On/Off con Isteresi
      if (!relayState && currentTemp < (activeDesiredTemp - hysteresis)) {
        relayState = true;
        digitalWrite(relayPin, HIGH);
        lastSwitchTime = currentTime;
        logData();
      } 
      else if (relayState && currentTemp > (activeDesiredTemp + hysteresis)) {
        relayState = false;
        digitalWrite(relayPin, LOW);
        lastSwitchTime = currentTime;
        logData();
      }
    }

    void logData() {
      Serial.print(millis()); Serial.print(",");
      Serial.print(currentTemp); Serial.print(",");
      Serial.print(currentHum); Serial.print(",");
      Serial.print(activeDesiredTemp); Serial.print(",");
      Serial.println(relayState ? "ON" : "OFF");
    }

    void handleJoystick() {
      int xVal = analogRead(PIN_JOY_X);
      int yVal = analogRead(PIN_JOY_Y);
      bool btnPressed = (digitalRead(PIN_JOY_BTN) == LOW);

      // Regolazione fine (Asse X)
      if (xVal < 200) { editingTemp -= 0.1; isEditing = true; delay(100); }
      else if (xVal > 800) { editingTemp += 0.1; isEditing = true; delay(100); }

      // Navigazione Preset (Asse Y)
      if (yVal < 200) {
        currentPresetIndex = (currentPresetIndex + 1) % 5;
        editingTemp = savedPresets[currentPresetIndex];
        isEditing = true;
        delay(250);
      } else if (yVal > 800) {
        currentPresetIndex = (currentPresetIndex <= 0) ? 4 : currentPresetIndex - 1;
        editingTemp = savedPresets[currentPresetIndex];
        isEditing = true;
        delay(250);
      }

      // Salvataggio
      if (btnPressed) {
        activeDesiredTemp = editingTemp;
        EEPROM.put(ADDR_TARGET_TEMP, activeDesiredTemp);
        isEditing = false;
        
        lcd.setCursor(0, 1);
        lcd.print("  TARGET SET!   ");
        delay(1000);
      }
    }

    float getCurrentTemp() { return currentTemp; }
    float getCurrentHum() { return currentHum; }
    float getTargetTemp() { return isEditing ? editingTemp : activeDesiredTemp; }
    bool isHeating() { return relayState; }
    bool getEditingState() { return isEditing; }
    bool hasError() { return sensorError; }
};

Termostato stressTester(PIN_RELE, 30.0, 0.5); // Isteresi ridotta a 0.5 per maggiore precisione nel test

void setup() {
  lcd.init();
  lcd.backlight();
  stressTester.init();
  
  lcd.setCursor(0, 0);
  lcd.print("STRESS TEST");
  lcd.setCursor(0, 1);
  lcd.print("MELON SYSTEM");
  delay(2000);
  lcd.clear();
}

void loop() {
  static unsigned long lastSensorRead = 0;
  if (millis() - lastSensorRead > 2000) {
    stressTester.updateSensor();
    stressTester.logData(); // Log ogni 2 secondi
    lastSensorRead = millis();
  }
  
  stressTester.handleJoystick();
  stressTester.updateControl();

  static unsigned long lastRefresh = 0;
  if (millis() - lastRefresh > 300) {
    updateDisplay();
    lastRefresh = millis();
  }
}

void updateDisplay() {
  lcd.setCursor(0, 0);
  if (stressTester.hasError()) {
    lcd.print("! SENSOR ERR !  ");
  } else {
    lcd.print("T:");
    lcd.print(stressTester.getCurrentTemp(), 1);
    lcd.print("C U:");
    lcd.print(stressTester.getCurrentHum(), 0);
    lcd.print("%");
    
    lcd.setCursor(13, 0);
    lcd.print(stressTester.isHeating() ? "ON " : "OFF");
  }

  lcd.setCursor(0, 1);
  lcd.print(stressTester.getEditingState() ? "EDIT: " : "SET : ");
  lcd.print(stressTester.getTargetTemp(), 1);
  lcd.print("\xDF"); 
  lcd.print("C   ");
}