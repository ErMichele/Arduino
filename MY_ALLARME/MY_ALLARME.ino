const int microphonePin = 2;   // ingresso digitale dal sensore knock
const int redPin = 9;
const int greenPin = 10;
const int bluePin = 11;

const int knockSequence[] = {1, 1, 0, 1, 0, 1};  // sequenza knock
const unsigned long knockInterval = 1000;               // intervallo tra knock
const unsigned long tolerancePercent = 40;             // tolleranza %
const int knockSequenceLength = sizeof(knockSequence) / sizeof(knockSequence[0]);
const unsigned long tolerance = knockInterval / 100 * tolerancePercent;

unsigned int knockIndex = 0;
unsigned long beginKnockTime = 0;
bool knocking = false;
bool error = false;
bool knocked = false;

void setup() {
  Serial.begin(9600);
  pinMode(microphonePin, INPUT);
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  setColor(0,0,255); // blu standby
  Serial.println("Sistema pronto: LED blu standby, attendendo primo knock...");
}

void loop() {
  if (knocking) {
    unsigned long knockDelta = millis() - beginKnockTime;

    bool isWithinToleranceWindow = knockDelta >= knockInterval - tolerance && knockDelta <= knockInterval + tolerance;

    // Controllo se knock avvenuto nella finestra
    if (isWithinToleranceWindow) {
      setColor(255,255,0); // giallo = finestra input
      if (digitalRead(microphonePin) == HIGH) {
        knocked = true;
        Serial.print("Knock rilevato al bit ");
        Serial.println(knockIndex);
      } else {
        Serial.print("Nessun knock al bit ");
        Serial.println(knockIndex);
      }
    }

    // Se scaduto tempo finestra → passo al prossimo bit
    if (knockDelta > knockInterval + tolerance) {
      // confronto knock con sequenza
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
        // sequenza completata
        if (error) {
          wrongSequence();
        } else {
          correctSequence();
        }
        // reset
        knocking = false;
        error = false;
        knockIndex = 0;
        setColor(0,0,255); // blu standby
        Serial.println("Sequenza terminata, ritorno in standby");
      }
    }
  } else {
    // attesa primo knock
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
  setColor(255,0,0); // rosso
  delay(2000);
}

void correctSequence() {
  Serial.println("Sequenza CORRETTA → LED verde");
  setColor(0,255,0); // verde
  delay(2000);
}

void setColor(int r, int g, int b) {
  analogWrite(redPin, r);
  analogWrite(greenPin, g);
  analogWrite(bluePin, b);
}
