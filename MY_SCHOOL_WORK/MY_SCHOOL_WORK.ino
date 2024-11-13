#include <pitches.h>
/*Programma di prova per il lavoro del WorkShop
 Utilizzo di bottoni, led, buzzer, filli, Poisson*/
 int fuz = 0;
 int led = 5;
 int stat = 8;
 int on = 6;
 int off = 7;
 int buzzer = 10;
 int tempo = 1000;
 int melodia [] = { NOTE_C5, NOTE_D5, NOTE_C5, NOTE_D5, NOTE_B5, NOTE_F5, NOTE_C6 };
  void setup () {
  pinMode (led, OUTPUT);
  pinMode (stat, INPUT_PULLUP);
  pinMode (on, INPUT_PULLUP);
  pinMode (off, INPUT_PULLUP);
  pinMode (buzzer, OUTPUT);
  digitalWrite (led, LOW);
 }

 /*Il programma si avvia con premere del bottone stat, da lì ci sono due scelte:
 1)premere on accendendo il led e facendo andare la melodia;
 2)premere off per spegnere il led.
 Ripremendo stat si spegne completamente tutto*/
 
 void loop () {
  if ((digitalRead (stat)== LOW) && (fuz=0) ) {
    fuz=1;
  }
  while (fuz=1) {
    if (digitalRead (on) == LOW) {
        do {
        digitalWrite (led, HIGH);
        for (int i=0; i<8; i++) {
        tone (buzzer, melodia, 3000);
        delay (tempo);}
        } while (digitalRead (off) == HIGH);  
    }
     else if (digitalRead (off) == LOW) {
        digitalWrite (led, LOW);
    }
     else if (digitalRead (stat)== LOW) {
      fuz=0;
      digitalWrite (led, LOW);
    }
   }
 }
