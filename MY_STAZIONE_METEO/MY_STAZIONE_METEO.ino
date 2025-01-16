#include <Adafruit_MPL3115A2.h>
#include <DFRobot_CCS811.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>

Adafruit_MPL3115A2 Baro;
DFRobot_CCS811 Aria;
RTC_DS3231 Orologio;
File Data;
DateTime ultimoRilevamento;

#define pin_data 10

void setup() {
    Serial.begin(9600);

    if (!Baro.begin()) {
        Serial.println("Errore sensore MPL, riavvia e controlla.");
        while (1);
    }

    Baro.setSeaPressure(1014);

    while (Aria.begin() != 0) {
        Serial.println("Caricamento sensore CSS, attendere.");
        delay(1000);
    }

    if (!SD.begin(pin_data)) {
        Serial.println("Errore con SD reader, rivvia e controlla.");
        while (1);
    }

    Data = SD.open("Data.csv", FILE_WRITE);
    if (!Data) {
      Serial.println("Errore nell'apertura del file."); 
      while (1); 
    }

    if (Data.size() == 0) {
      Data.println("Data, Ora, Temperatura (°C), Pressione (hPa), Altezza (m), CO2 (ppm), TVOC (ppb)"); 
    } 
    Data.close();

    if (!Orologio.begin()) {
        Serial.println("Errore con l'RTC, rivvia e controlla.");
        while (1);
    }

    if (Orologio.lostPower()) {
        Orologio.adjust(DateTime(F(__DATE__), F(__TIME__)));
        Serial.println("Il RTC si era decalibrato, ora è stato aggiustato ai tempi corretti");
    }

    ultimoRilevamento = Orologio.now();
}

void loop() {
  DateTime oraCorrente = Orologio.now();

    if (oraCorrente.minute() == 0 && oraCorrente.second() == 0) {
        if (oraCorrente != ultimoRilevamento) {
            ultimoRilevamento = oraCorrente;
            float Temp = Baro.getTemperature();
            float Pres = Baro.getPressure();
            float Alte = Baro.getAltitude();
            float CO2_Somma = 0, TVOC_Somma = 0;
            float CO2, TVOC;
            Aria.writeBaseLine(0x447B);

            if (Aria.checkDataReady() == true) {
                for (int i = 0; i < 10; i++) {
                    CO2_Somma += Aria.getCO2PPM();
                    TVOC_Somma += Aria.getTVOCPPB();
                    delay(1000);
                }
                CO2 = CO2_Somma / 10;
                TVOC = TVOC_Somma / 10;
                Serial.println("------------------------------");
                Serial.print("Temperatura = "); Serial.print(Temp); Serial.println(" °C");
                Serial.print("Pressione = "); Serial.print(Pres); Serial.println(" hPa");
                Serial.print("Altezza = "); Serial.print(Alte); Serial.println(" m");
                Serial.print("Livello di CO2 = "); Serial.print(CO2); Serial.println(" ppm");
                Serial.print("Livello di TVOC = "); Serial.print(TVOC); Serial.println(" ppb");

                Data = SD.open("Data.csv", FILE_WRITE);
                if (Data) {
                    Data.print(oraCorrente.year(), DEC); Data.print("/");
                    Data.print(oraCorrente.month(), DEC); Data.print("/");
                    Data.print(oraCorrente.day(), DEC); Data.print(" ");
                    Data.print(oraCorrente.hour(), DEC); Data.print(", ");

                    Data.print(Temp); Data.print(", ");
                    Data.print(Pres); Data.print(", ");
                    Data.print(Alte); Data.print(", ");
                    Data.print(CO2); Data.print(", ");
                    Data.println(TVOC);
                    Data.close();
                } else {
                    Serial.println("Errore nella scrittura del file.");
                }
            } else {
                Serial.println("Il data non è ancora pronto");
            }
        }
    }

  delay(500);
}
