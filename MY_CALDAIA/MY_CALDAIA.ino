#include <LiquidCrystal.h>
#include <DHT.h>

int umi, temp;
#define Sens 13
#define on 6
#define off 8

#define DHTTYPE DHT11
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
DHT dht (Sens, DHTTYPE);

void setup()
{
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("Umidita':");
  lcd.setCursor(0, 1);
  lcd.print("Temperatura:");
  pinMode (on, OUTPUT);
  pinMode (off, OUTPUT);
  dht.begin();
}

void loop()
{
  umi=dht.readHumidity();
  temp=dht.readTemperature();
  Serial.print(temp);
  if (umi<10) {
    lcd.setCursor(10, 0);
    lcd.print ("  ");
  }
  else if (umi<100) {
    lcd.setCursor(10, 0);
    lcd.print (" ");
  }
  if (temp<10) {
    lcd.setCursor(10, 1);
    lcd.print ("  ");
  }
  else if (temp<100) {
    lcd.setCursor(10, 1);
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
  if (temp<25) {
    digitalWrite(on, HIGH);
    digitalWrite(off, LOW);
  }
  else {
    digitalWrite(off, HIGH);
    digitalWrite(on, LOW);
  }
  delay(1000);
}
