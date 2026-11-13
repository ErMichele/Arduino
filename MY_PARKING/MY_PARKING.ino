#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

// ==========================================
// Class 1: Ultrasonic Sensor Management
// ==========================================
class UltrasonicSensor {
private:
  uint8_t trigPin;
  uint8_t echoPin;

public:
  UltrasonicSensor(uint8_t trig, uint8_t echo) : trigPin(trig), echoPin(echo) {}

  void begin() {
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    digitalWrite(trigPin, LOW);
  }

  float readDistance() {
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);

    long duration = pulseIn(echoPin, HIGH, 30000); 
    if (duration == 0) return 999.0; 

    return duration * 0.0343 / 2.0;
  }
};

// ==========================================
// Class 2: RGB LED Management
// ==========================================
class RGBLed {
private:
  uint8_t redPin;
  uint8_t greenPin;
  uint8_t bluePin;

public:
  RGBLed(uint8_t r, uint8_t g, uint8_t b) : redPin(r), greenPin(g), bluePin(b) {}

  void begin() {
    pinMode(redPin, OUTPUT);
    pinMode(greenPin, OUTPUT);
    pinMode(bluePin, OUTPUT);
    off();
  }

  void setColor(bool redState, bool greenState, bool blueState) {
    digitalWrite(redPin, redState);
    digitalWrite(greenPin, greenState);
    digitalWrite(bluePin, blueState);
  }

  void red()    { setColor(HIGH, LOW, LOW); }
  void green()  { setColor(LOW, HIGH, LOW); }
  void yellow() { setColor(HIGH, HIGH, LOW); } // Red + Green = Yellow
  void off()    { setColor(LOW, LOW, LOW); }
};

// ==========================================
// Class 3: Smart Parking Manager
// ==========================================
class ParkingManager {
private:
  UltrasonicSensor entrySensor;
  UltrasonicSensor exitSensor;
  RGBLed statusLed;
  Servo barrierServo;
  LiquidCrystal_I2C lcd;

  const uint8_t servoPin;
  const float detectionDistance; 
  
  int totalSpaces;
  int availableSpaces;

  const int GATE_CLOSED = 0;   
  const int GATE_OPEN = 90;    

  enum GateState {
    IDLE,
    WAITING_FOR_CAR_TO_ENTER,
    WAITING_FOR_CAR_TO_EXIT
  };
  
  GateState currentState;

  void openGate() {
    barrierServo.write(GATE_OPEN);
    statusLed.yellow(); // Bar is lifted -> turns YELLOW
  }

  void closeGate() {
    barrierServo.write(GATE_CLOSED);
    currentState = IDLE; // Mark as closed/idle first
    updateStatusLED();   // Switch LED back to Green or Red based on capacity
    delay(1500);         // 1.5 seconds cooldown after barrier lowers before next operation
  }

public:
  ParkingManager(uint8_t entryTrig, uint8_t entryEcho,
                 uint8_t exitTrig, uint8_t exitEcho,
                 uint8_t redPin, uint8_t greenPin, uint8_t bluePin,
                 uint8_t sPin, uint8_t lcdAddr,
                 int totalCap, float detectDist = 12.0)
    : entrySensor(entryTrig, entryEcho),
      exitSensor(exitTrig, exitEcho),
      statusLed(redPin, greenPin, bluePin),
      lcd(lcdAddr, 16, 2),
      servoPin(sPin),
      totalSpaces(totalCap),
      availableSpaces(totalCap),
      detectionDistance(detectDist),
      currentState(IDLE) {}

  void begin() {
    entrySensor.begin();
    exitSensor.begin();
    statusLed.begin();

    barrierServo.attach(servoPin);
    closeGate(); 

    lcd.init();
    lcd.backlight();
    updateDisplay();
  }

  void updateStatusLED() {
    // If the gate is open and waiting for a car to pass, force YELLOW
    if (currentState != IDLE) {
      statusLed.yellow();
    } else {
      // If gate is closed, use Green (spaces available) or Red (full)
      if (availableSpaces == 0) {
        statusLed.red();
      } else {
        statusLed.green();
      }
    }
  }

  void updateDisplay() {
    lcd.clear();
    if (availableSpaces == 0) {
      lcd.setCursor(0, 0);
      lcd.print("  PARKING FULL  ");
      lcd.setCursor(0, 1);
      lcd.print(" No Slots Avail ");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("  PARKING SYSTEM ");
      lcd.setCursor(0, 1);
      lcd.print("Open Slots: ");
      lcd.print(availableSpaces);
    }
    updateStatusLED();
  }

  void update() {
    float entryDist = entrySensor.readDistance();
    float exitDist = exitSensor.readDistance();

    switch (currentState) {
      case IDLE:
        // Check incoming car at entry
        if (entryDist < detectionDistance) {
          if (availableSpaces > 0) {
            availableSpaces--;
            currentState = WAITING_FOR_CAR_TO_ENTER; // Set state FIRST so yellow locks in
            openGate();       
            updateDisplay();  
            delay(500); 
          } else {
            statusLed.red();  
            lcd.setCursor(0, 1);
            lcd.print("FULL! Wait...  ");
            delay(1500);
            updateDisplay();
          }
        }
        // Check car wanting to exit
        else if (exitDist < detectionDistance) {
          if (availableSpaces < totalSpaces) {
            availableSpaces++;
            currentState = WAITING_FOR_CAR_TO_EXIT; // Set state FIRST so yellow locks in
            openGate();       
            updateDisplay();  
            delay(500); 
          }
        }
        break;

      case WAITING_FOR_CAR_TO_ENTER:
        // Close gate once car clears the exit/inner sensor
        if (exitDist < detectionDistance) {
          closeGate();        // Closes gate, applies 1.5s cooldown & switches LED to Green/Red
          updateDisplay(); 
          delay(500); 
        }
        break;

      case WAITING_FOR_CAR_TO_EXIT:
        // Close gate once car clears the entry/outer sensor
        if (entryDist < detectionDistance) {
          closeGate();        // Closes gate, applies 1.5s cooldown & switches LED to Green/Red
          updateDisplay(); 
          delay(500); 
        }
        break;
    }

    delay(100); 
  }
};

// ==========================================
// Setup and Instantiation
// ==========================================

ParkingManager parkingSystem(2, 3, 4, 5, 10, 6, 11, 9, 0x27, 3, 12.0);

void setup() {
  parkingSystem.begin();
}

void loop() {
  parkingSystem.update();
}