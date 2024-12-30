#define pompa 2
float umidita;
void setup() {
  Serial.begin(9600);
  pinMode(pompa, OUTPUT);
  digitalWrite(pompa, LOW);
  delay(500);
}
void loop() {
  Serial.print("Umidità:");
  umidita = analogRead(A0);
  Serial.print(umidita);
  Serial.println(";");
  if(umidita<450)
  {
    digitalWrite(pompa, HIGH);
  }
  else
  {
    digitalWrite(pompa, LOW);
  }
  delay(1000);
}
