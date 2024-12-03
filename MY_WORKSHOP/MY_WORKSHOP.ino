#include <Servo.h>

#include <LiquidCrystal.h>

int seconds = 0;

int inches = 0;

int cm = 0;

int pos = 0;

#define infra 10

#define led 8

Servo servo_9;

LiquidCrystal lcd_1(12, 11, 5, 4, 3, 2);
  
long readUltrasonicDistance(int triggerPin, int echoPin)
{
  pinMode(triggerPin, OUTPUT);
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  return pulseIn(echoPin, HIGH);
}

void setup()
{
  lcd_1.begin(16, 2);
  servo_9.attach(7, 500, 2500);
  pinMode (led, OUTPUT);
}

void loop()
{
  cm = 0.01723 * readUltrasonicDistance(6, 6);
  inches = (cm / 2.54);
  if (inches <= 100) {
    lcd_1.setCursor (6, 0);
    lcd_1.print (" ");
  }
  if (cm <= 100) {
    lcd_1.setCursor (6, 1);
    lcd_1.print (" ");
  }
  lcd_1.setCursor(0, 0);
  lcd_1.print("in: ");
  lcd_1.setCursor (4, 0);
  lcd_1.print(inches);
  lcd_1.setCursor(0, 1);
  lcd_1.print("cm: ");
  lcd_1.setCursor (4, 1);
  lcd_1.print(cm);
  while (digitalRead(infra)==HIGH||inches<40||inches>50) {
  	digitalWrite (led, HIGH);
  	for (pos = 0; pos <= 180; pos += 3) {
  		cm = 0.01723 * readUltrasonicDistance(6, 6);
  		inches = (cm / 2.54);
  		if (inches <= 100) {
    		lcd_1.setCursor (6, 0);
    		lcd_1.print (" ");
  		}
  		if (cm <= 100) {
    		lcd_1.setCursor (6, 1);
    		lcd_1.print (" ");
  		}
  		lcd_1.setCursor(0, 0);
  		lcd_1.print("in: ");
  		lcd_1.setCursor (4, 0);
  		lcd_1.print(inches);
  		lcd_1.setCursor(0, 1);
  		lcd_1.print("cm: ");
  		lcd_1.setCursor (4, 1);
  		lcd_1.print(cm);
  		servo_9.write(pos);
  		delay(100);
  	}
  	digitalWrite (led, LOW);
  	for (pos = 180; pos >= 0; pos -= 3) {
  		cm = 0.01723 * readUltrasonicDistance(6, 6);
  		inches = (cm / 2.54);
  		if (inches <= 100) {
    		lcd_1.setCursor (6, 0);
    		lcd_1.print (" ");
  		}
  		if (cm <= 100) {
    		lcd_1.setCursor (6, 1);
    		lcd_1.print (" ");
  		}
  		lcd_1.setCursor(0, 0);
 		  lcd_1.print("in: ");
  		lcd_1.setCursor (4, 0);
  		lcd_1.print(inches);
  		lcd_1.setCursor(0, 1);
  		lcd_1.print("cm: ");
  		lcd_1.setCursor (4, 1);
  		lcd_1.print(cm);
  		servo_9.write(pos);
  		delay(100);
  	}
  }
  delay (1000);
}
