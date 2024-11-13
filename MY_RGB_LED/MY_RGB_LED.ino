#define BLU 3 
#define VERDE 5
#define ROSSO 6

void setup () {
  pinMode (BLU, OUTPUT);
  pinMode (VERDE, OUTPUT);
  pinMode (ROSSO, OUTPUT);
  digitalWrite (BLU, HIGH);
  digitalWrite (VERDE, LOW);
  digitalWrite (ROSSO, LOW);
}

int bluValue;
int verdeValue;
int rossoValue;

void loop () {
  #define TEMPO 10

  bluValue = 255;
  verdeValue = 0;
  rossoValue = 0;
  
for (int i = 0; i < 255; i += 1) {

bluValue -= 1;
verdeValue += 1;
analogWrite (BLU, bluValue);
analogWrite (VERDE, verdeValue);
delay (TEMPO);
}

for (int i = 0; i < 255; i += 1) {

verdeValue -= 1;
rossoValue += 1;
analogWrite (VERDE, verdeValue);
analogWrite (ROSSO, rossoValue);
delay (TEMPO);
}

for (int i = 0; i < 255; i += 1) {

rossoValue -= 1;
bluValue += 1;
analogWrite (ROSSO, rossoValue);
analogWrite (BLU, bluValue);
delay (TEMPO);
}
}
