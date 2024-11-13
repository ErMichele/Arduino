#include <pitches.h>

int tempo = 500;
int NOTE [] = {NOTE_C5, NOTE_A5, NOTE_G5};
void setup () {}
void loop (){

  for (int i = 0; i < 3; i++) {
    tone (8, NOTE[i], tempo);
    delay (1000);
  }
  delay (2000);
}
