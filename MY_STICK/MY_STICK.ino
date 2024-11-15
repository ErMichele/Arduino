#include <pitches.h>
const int SW_pin = 2; 
const int X_pin = 0; 
const int Y_pin = 1; 
#define blu 8
#define ver 9
#define ros 10
#define gia 11
#define buzzer 12
int melodia [] = { NOTE_C5, NOTE_D5, NOTE_C5, NOTE_D5, NOTE_B5, NOTE_F5, NOTE_C6, NOTE_C6 };
void setup() {
  pinMode(SW_pin, INPUT);
  pinMode(blu, OUTPUT);
  pinMode(ver, OUTPUT);
  pinMode(ros, OUTPUT);
  pinMode(gia, OUTPUT);
  digitalWrite(SW_pin, HIGH);

}
int tocco;
int x;
int y;

void loop() {
  tocco=digitalRead(SW_pin);
  x=analogRead(X_pin);
  y=analogRead(Y_pin);
  if (x>1000) {
    digitalWrite (gia, HIGH);
    digitalWrite (ros, LOW);
    digitalWrite (blu, LOW);
    digitalWrite (ver, LOW);
  }
  else if (x<100) {
    digitalWrite (ros, HIGH);
    digitalWrite (gia, LOW);
    digitalWrite (blu, LOW);
    digitalWrite (ver, LOW);
  }
  else if (y<100) {
    digitalWrite (blu, HIGH);
    digitalWrite (gia, LOW);
    digitalWrite (ros, LOW);
    digitalWrite (ver, LOW);
  }
  else if (y>1000) {
    digitalWrite (ver, HIGH);
    digitalWrite (gia, LOW);
    digitalWrite (blu, LOW);
    digitalWrite (ros, LOW);
  }
  else if (tocco==0) {
    for (int i=0; i<8; i++) {
      
        tone (buzzer, melodia[i], 3000);
        delay (1000);}
  }
}
