#include <Adafruit_MPL3115A2.h>

Adafruit_MPL3115A2 Baro;

void setup() {
  Serial.begin(9600);
  Serial.print("Test barometro!\n");

  if (!Baro.begin()) {
    Serial.print("Errore sensore, riavvia e controlla.");
    while(1);
  }
  Baro.setSeaPressure(1014);
}

void loop() {
  float Temp = Baro.getTemperature();
  float Pres = Baro.getPressure();
  float Alte = Baro.getAltitude() + 15.5;

  Serial.println("------------------------------");
  Serial.print("Temperatura = "); Serial.print(Temp); Serial.println(" °C");
  Serial.print("Pressione = "); Serial.print(Pres); Serial.println(" hPa");
  Serial.print("Altezza = "); Serial.print(Alte); Serial.println(" m");

  delay(750);
}
