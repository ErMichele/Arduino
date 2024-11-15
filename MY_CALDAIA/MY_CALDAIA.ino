#include <LiquidCrystal.h>

int umi;
int temp=10;
#define on 6
#define off 8
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup()
{
  lcd.begin(16, 2);
  lcd.print("Umidita':");
  lcd.setCursor(0, 1);
  lcd.print("Temperatura:");
  pinMode (on, OUTPUT);
  pinMode (off, OUTPUT);
}
//nota bene: la temperatura non sarà usata dato che non abbiamo la componente qui, ma il funzionamento è lo stesso
void loop()
{
  umi=analogRead(A0);
  if (umi<10) {
    lcd.setCursor(10, 0);
    lcd.print ("  ");
  }
  else if (umi<100) {
    lcd.setCursor(10, 0);
    lcd.print (" ");
  }
  lcd.setCursor(9, 0);
  lcd.print(umi);
  lcd.setCursor (12, 0);
  lcd.print("g/m3");
  lcd.setCursor (12, 1);
  lcd.print(temp);
  lcd.setCursor (15, 1);
  lcd.print("C");
  if (umi<438/*||temp<16*/) {
    digitalWrite(on, HIGH);
    digitalWrite(off, LOW);
  }
  else {
	digitalWrite(off, HIGH);
    digitalWrite(on, LOW);
  }
  delay(1000);
}