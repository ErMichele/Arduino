#include "LedControl.h"

const int microphonePin = 2;
const int redPin = 11;
const int greenPin = 10;
const int bluePin = 9;

const int knockSequence[] = {1, 1, 0, 1, 0, 1};
const int knockSequenceLength = sizeof(knockSequence) / sizeof(knockSequence[0]);

LedControl lc = LedControl(4, 6, 5, 1);

unsigned int knockIndex = 0;
unsigned long beginKnockTime = 0;
bool knocking = false;
bool error = false;
bool knocked = false;

unsigned long delaytime = 100;

byte lockedLock[8] = {
  B00011111,
  B00101111,
  B01001111,
  B10001111,
  B10001111,
  B01001111,
  B00101111,
  B00011111
};

byte unlockedLock[8] = {
  B00001111,
  B00001111,
  B01001111,
  B10001111,
  B10001111,
  B01001111,
  B00101111,
  B00011111
};

byte soundIcon[8] = {
  B00000000,
  B00000000,
  B11111111,
  B01111110,
  B00111100,
  B00011000,
  B00000000,
  B00000000
};

void setup() {
  Serial.begin(9600);
  pinMode(microphonePin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  lc.shutdown(0,false);
  lc.setIntensity(0,1);
  lc.clearDisplay(0);

  setColor(0,0,255);
  showIcon(lockedLock);
  Serial.println("Sistema pronto: LED blu standby, attendendo primo knock...");
}

void loop() {
  if (knocking) {
    unsigned long knockDelta = millis() - beginKnockTime;

    if (knockDelta <= 1000) {
      setColor(255,123,0);

      if ((millis() / 300) % 2 == 0) {
        showIcon(soundIcon);
      } else {
        lc.clearDisplay(0);
      }
      if (digitalRead(microphonePin) == HIGH && !knocked) {
        knocked = true;
        Serial.print("Knock rilevato al bit ");
        Serial.println(knockIndex);
      }
    }
    else if (knockDelta > 1000 && knockDelta <= 1500) {
      setColor(128,0,128);
      if ((millis() / 300) % 2 == 0) {
        showIcon(soundIcon);
      } else {
        lc.clearDisplay(0);
      }
    }

    if (knockDelta > 1500) {
      if (knocked != knockSequence[knockIndex]) {
        error = true;
        Serial.print("Errore alla posizione ");
        Serial.println(knockIndex);
      } else {
        Serial.print("Bit ");
        Serial.print(knockIndex);
        Serial.println(" corretto");
      }

      knocked = false;
      knockIndex++;

      if (knockIndex < knockSequenceLength) {
        beginKnockTime = millis();
      } else {
        if (error) {
          wrongSequence();
        } else {
          correctSequence();
        }
        knocking = false;
        error = false;
        knockIndex = 0;
        setColor(0,0,255);
        showIcon(lockedLock);
        Serial.println("Sequenza terminata, ritorno in standby");
      }
    }
  } else {
    if (digitalRead(microphonePin) == HIGH) {
      beginKnockTime = millis();
      knockIndex = 0;
      knocking = true;
      Serial.println("Primo knock rilevato → inizio sequenza!");
    }
  }
}

void wrongSequence() {
  Serial.println("Sequenza ERRATA → LED rosso");
  setColor(255,0,0);
  for (int i=0; i<3; i++) {
    showIcon(lockedLock);
    delay(500);
    lc.clearDisplay(0);
    delay(500);
  }
}

void correctSequence() {
  Serial.println("Sequenza CORRETTA → LED verde");
  setColor(0,255,0);
  for (int i=0; i<3; i++) {
    showIcon(unlockedLock);
    delay(500);
    lc.clearDisplay(0);
    delay(500);
  }
}

void setColor(int r, int g, int b) {
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}

void showIcon(byte icon[8]) {
  for (int row=0; row<8; row++) {
    lc.setRow(0,row,icon[row]);
  }
}
