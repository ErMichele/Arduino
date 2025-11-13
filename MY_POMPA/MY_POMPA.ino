#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define POMPA 9
#define START 4
#define STOP 10
#define DATI A0
#define MODALITA 3
#define LED LED_BUILTIN

LiquidCrystal_I2C lcd(0x27,16, 2);

int umidita;
bool modalitaAutomatica = true;
const int sogliaUmidita = 400;

void Mostra (String Modalita);

void setup() {
  Serial.begin(9600);
  pinMode(POMPA, OUTPUT);
  digitalWrite(POMPA, HIGH);  // relè disattivo (pompa spenta)
  pinMode(START, INPUT_PULLUP);
  pinMode(STOP, INPUT_PULLUP);
  pinMode(MODALITA, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  lcd.init();
  lcd.backlight();
  Mostra("Automatica!");
}

void loop() {
  // Cambio modalità con pulsante MODALITA
  static int lastStatoModalita = HIGH;
  int statoModalita = digitalRead(MODALITA);
  if (statoModalita == LOW && lastStatoModalita == HIGH) {
    modalitaAutomatica = !modalitaAutomatica;
    Serial.print("Modalità cambiata: ");
    String Modalita = modalitaAutomatica ? "Automatica!" : "Manuale!";
    Serial.println(Modalita);
    Mostra(Modalita);

    // Spegni pompa al cambio modalità
    digitalWrite(POMPA, HIGH);  // relè disattivo
    Serial.println("Pompa forzata spenta al cambio modalità");
    delay(300);
  }
  lastStatoModalita = statoModalita;

  // LED indica modalità
  digitalWrite(LED, modalitaAutomatica ? HIGH : LOW);

  // Modalità automatica
  if (modalitaAutomatica) {
    umidita = analogRead(DATI);
    Serial.print("Umidità: ");
    Serial.println(umidita);
    
    lcd.setCursor(9, 1);
    lcd.print("   ");
    lcd.setCursor(9, 1);
    lcd.print(umidita);
    lcd.setCursor(13, 0);
    if (umidita < sogliaUmidita) {
      if (digitalRead(STOP) == LOW) {
        digitalWrite(POMPA, HIGH);  // relè disattivo
        Serial.println("Pompa spenta (umidità sufficiente ma stoppata)");
        lcd.print("OFF");
      } else {
        digitalWrite(POMPA, LOW);  // relè attivo
        Serial.println("Pompa accesa (umidità bassa)");
        lcd.print("ON ");
      }
    } else {
      digitalWrite(POMPA, HIGH);  // relè disattivo
      Serial.println("Pompa spenta (umidità sufficiente)");
      lcd.print("OFF");
    }
  } else {                             // Modalità manuale
    lcd.setCursor(0, 1);
    if (digitalRead(START) == LOW) {  // pulsante premuto
      digitalWrite(POMPA, LOW);
      Serial.println("Manuale: pompa accesa");
      lcd.print("Pompa ON ");
    } else {
      digitalWrite(POMPA, HIGH);
      Serial.println("Manuale: pompa spenta");
      lcd.print("Pompa OFF");
    }
  }
  delay(500);
}

void Mostra (String Modalita) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(Modalita);
  if (Modalita == "Automatica!") {
    lcd.setCursor(0, 1);
    lcd.print("Umidita: ");
  }
}
