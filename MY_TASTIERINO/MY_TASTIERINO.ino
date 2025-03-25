#include <Keypad.h>
#include <Servo.h>

const byte ROWS = 4;
const byte COLS = 4;
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {9, 8, 7, 6};
byte colPins[COLS] = {5, 4, 3, 2};
Keypad Numeri = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

Servo Griletto;

void setup() {
  Serial.begin(9600);
  Griletto.attach(9);
  Griletto.write(0);
}
  
void loop() {
  int Spari;
  char Tasto = Numeri.getKey();
  if (Tasto == '#') {
    Spari = 0;
  } else if (Tasto == '1') {
    Spari = (Spari * 10) + 1;
  } else if (Tasto == '2') {
    Spari = (Spari * 10) + 2;
  } else if (Tasto == '3') {
    Spari = (Spari * 10) + 3;
  } else if (Tasto == '4') {
    Spari = (Spari * 10) + 4;
  } else if (Tasto == '5') {
    Spari = (Spari * 10) + 5;
  } else if (Tasto == '6') {
    Spari = (Spari * 10) + 6;
  } else if (Tasto == '7') {
    Spari = (Spari * 10) + 7;
  } else if (Tasto == '8') {
    Spari = (Spari * 10) + 8;
  } else if (Tasto == '9') {
    Spari = (Spari * 10) + 9;
  } else if (Tasto == '0') {
    Spari = Spari * 10;
  } else if (Tasto == '*' && Spari != 0) {
    for (int i = 1; i <= Spari; i++) {
      Griletto.write(180);
      delay(1000);
      Griletto.write(0);
      delay(9000);
    }
    Spari = 0;
  }
}
