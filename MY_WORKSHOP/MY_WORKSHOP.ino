#include <Servo.h>

#include <LiquidCrystal.h>

#include <SR04.h>

long distanza;

int inches = 0;

int cm = 0;

int pos = 0;

int cmt = 1;

#define infra 10

#define led 8

#define TRIG_PIN 6

#define ECHO_PIN 9 

SR04 sr04 = SR04(ECHO_PIN,TRIG_PIN);

Servo servo_9;

LiquidCrystal lcd_1(12, 11, 5, 4, 3, 2);

void print () {
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
}

void misure(int *m, int *in) {
  distanza=sr04.Distance();
  *m = 0.01723 * distanza;
  *in = (*m / 2.54);
}

void setup()
{
  lcd_1.begin(16, 2);
  servo_9.attach(7, 500, 2500);
  pinMode (led, OUTPUT);
  servo_9.write(0);
}

void loop()
{ 
  misure (&cm, &inches);
  if (cmt!=cm) {
    print ();
    cmt = cm;
  }
  while (inches < 40 || inches > 50) {
  	digitalWrite (led, HIGH);
  	for (pos = 0; pos <= 180; pos += 10) {
  		misure (&cm, &inches);
      if (cmt!=cm) {
        print ();
        cmt = cm;
      }
  		servo_9.write(pos);
  		delay(100);
  	}
  	digitalWrite (led, LOW);
  	for (pos = 180; pos >= 0; pos -= 10) {
  		misure (&cm, &inches);
      if (cmt!=cm) {
        print ();
        cmt = cm;
      }
  		servo_9.write(pos);
  		delay(100);
  	}
  }
  delay (1000);
}
