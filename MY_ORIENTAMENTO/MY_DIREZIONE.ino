void setup () {
pinMode (LED_BUILTIN, OUTPUT);
pinMode (2, INPUT);
digitalWrite (2, HIGH);
}
void loop (){
if (digitalRead(2)==HIGH) {
  digitalWrite(LED_BUILTIN, HIGH);
}
else {
  digitalWrite(LED_BUILTIN, LOW);
}
}
