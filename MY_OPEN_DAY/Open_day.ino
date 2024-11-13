int verde = 0;
int blu = 1;
int giallo = 2;
int rosso = 3;
int bianco = 4;
int t = 1000;

void setup() {
  pinMode (blu, OUTPUT);
  pinMode (giallo, OUTPUT);
  pinMode (rosso, OUTPUT);
  pinMode (bianco, OUTPUT);
  pinMode (verde, OUTPUT);
}

void loop() {
  digitalWrite (verde, HIGH);
  delay (t);
  digitalWrite (verde, LOW);
  digitalWrite (blu, HIGH);
  delay (t);
  digitalWrite (blu, LOW);
  digitalWrite (giallo, HIGH);
  delay (t);
  digitalWrite (giallo, LOW);
  digitalWrite (rosso, HIGH);
  delay (t);
  digitalWrite (rosso, LOW);
  digitalWrite (bianco, HIGH);
  delay (t);
  digitalWrite (bianco, LOW);
}
