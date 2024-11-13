#include <Servo.h>


Servo MyServo;
int TEMPO = 500;
void setup () {
MyServo.attach (9);
MyServo.write (0);
}
void loop () {
delay (TEMPO);
MyServo.write (15);
delay (TEMPO);
MyServo.write (30);
delay (TEMPO);
MyServo.write (45);
delay (TEMPO);
MyServo.write (60);
delay (TEMPO);
MyServo.write (75);
delay (TEMPO);
MyServo.write (90);
delay (TEMPO);
MyServo.write (105);
delay (TEMPO);
MyServo.write (120);
delay (TEMPO);
MyServo.write (135);
delay (TEMPO);
MyServo.write (150);
delay (TEMPO);
MyServo.write (165);
delay (TEMPO);
MyServo.write (180);
delay (TEMPO);
MyServo.write (165);
delay (TEMPO);
MyServo.write (150);
delay (TEMPO);
MyServo.write (135);
delay (TEMPO);
MyServo.write (120);
delay (TEMPO);
MyServo.write (105);
delay (TEMPO);
MyServo.write (90);
delay (TEMPO);
MyServo.write (75);
delay (TEMPO);
MyServo.write (60);
delay (TEMPO);
MyServo.write (45);
delay (TEMPO);
MyServo.write (30);
delay (TEMPO);
MyServo.write (15);
delay (TEMPO);
MyServo.write (0);
}
