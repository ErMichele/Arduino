#define BUZZER 12
int TEMPO;
void setup () {
  pinMode (BUZZER, OUTPUT);
}

void loop () {
  for (int i = 0; i<40; i++){
    if (i<10)
    {
      TEMPO = 3000;
    }
    else if  (i<20){
      TEMPO = 2000;
    } 
    else if   (i<30)
    {
      TEMPO = 1000;
  }
  else if  (i<40)
  {
    TEMPO = 500;
      }
      digitalWrite (BUZZER, HIGH);
      delay (TEMPO);
      digitalWrite (BUZZER, LOW);
      delay (TEMPO);
  }
  digitalWrite (BUZZER, HIGH);
  delay (5000);
  digitalWrite (BUZZER, LOW);
  delay (5000);
}
