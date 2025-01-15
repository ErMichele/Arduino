#include <Adafruit_MPL3115A2.h>
#include <DFRobot_CCS811.h>

Adafruit_MPL3115A2 Baro;
DFRobot_CCS811 Aria;

void setup() {
  Serial.begin(9600);
  Serial.print("Test barometro!\n");

  if (!Baro.begin()) {
    Serial.print("Errore sensore MPL, riavvia e controlla.");
    while(1);
  }

  Baro.setSeaPressure(1014);

  while (Aria.begin() != 0) {
    Serial.println("Caricamento sensore CSS, attendere.");
    delay(1000);
  }
}

void loop() {
  float Temp = Baro.getTemperature();
  float Pres = Baro.getPressure();
  float Alte = Baro.getAltitude() + 15.5;
  Aria.writeBaseLine(0x447B);
  
  if (Aria.checkDataReady () == true) {
    Serial.println("------------------------------");
    Serial.print("Temperatura = "); Serial.print(Temp); Serial.println(" °C");
    Serial.print("Pressione = "); Serial.print(Pres); Serial.println(" hPa");
    Serial.print("Altezza = "); Serial.print(Alte); Serial.println(" m");
    Serial.print("Livello di CO2 = "); Serial.print(Aria.getCO2PPM()); Serial.println(" ppm");
    Serial.print("Livello di TVOC = "); Serial.print(Aria.getTVOCPPB()); Serial.println(" ppb");
  } else {
    Serial.println("Il data non è ancora pronto");
  }
  
  delay(1000);
}
