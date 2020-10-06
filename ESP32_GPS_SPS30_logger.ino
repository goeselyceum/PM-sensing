#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <TinyGPS++.h>
#include <sps30.h>

TinyGPSPlus gps;

unsigned long previousMillis = 0;

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);
  if (!SD.begin()) {
    Serial.println("Card Mount Failed");
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));

  // SPS30
  int16_t ret;
  uint8_t auto_clean_days = 4;
  uint32_t auto_clean;
  sensirion_i2c_init();

  while (sps30_probe() != 0) {
    Serial.print("SPS sensor probing failed\n");
    delay(500);
  }
  Serial.print("SPS sensor probing successful\n");
  ret = sps30_set_fan_auto_cleaning_interval_days(auto_clean_days);
  if (ret) {
    Serial.print("error setting the auto-clean interval: ");
    Serial.println(ret);
  }

  ret = sps30_start_measurement();
  if (ret < 0) {
    Serial.print("error starting measurement\n");
  }
  Serial.print("measurements started\n");
  delay(2000);
}

void loop() {
  while (Serial2.available() > 0)
    if (gps.encode(Serial2.read()))
      displayInfo();

  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    Serial.println(F("No GPS detected: check wiring."));
    while (true);
  }


  if (millis() - previousMillis >= 1000) {
    previousMillis = millis();
    struct sps30_measurement m;
    char serial[SPS30_MAX_SERIAL_LEN];
    uint16_t data_ready;
    int16_t ret;

    do {
      ret = sps30_read_data_ready(&data_ready);
      if (ret < 0) {
        Serial.print("error reading data-ready flag: ");
        Serial.println(ret);
      } else if (!data_ready)
        Serial.print("data not ready, no new measurement available\n");
      else
        break;
      delay(100); /* retry in 100ms */
    } while (1);

    ret = sps30_read_measurement(&m);
    if (ret < 0) {
      Serial.print("error reading measurement\n");
    } else {
      Serial.println();
      Serial.print("PM  2.5: ");
      Serial.println(m.mc_2p5);
      Serial.print("PM 10.0: ");
      Serial.println(m.mc_10p0);
      Serial.println();
    }  

    if (gps.time.second() % 30 == 0) {
      Serial.println("Logging");
      Serial.println();
      File dataFile = SD.open("gpslog.txt", FILE_WRITE);
      dataFile.print(gps.location.lat(), 6);
      dataFile.print(F(","));
      dataFile.print(gps.location.lng(), 6);
      dataFile.print(gps.date.day());
      dataFile.print(F("/"));
      dataFile.print(gps.date.month());
      dataFile.print(F("/"));
      dataFile.print(gps.date.year());
      dataFile.print(F(","));
      dataFile.print(m.mc_2p5);
      dataFile.print(F(","));
      dataFile.println(m.mc_10p0);
      dataFile.close();
    }
  }
}

void displayInfo()
{
  Serial.print(F("Location: "));
  if (gps.location.isValid())
  {
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(","));
    Serial.print(gps.location.lng(), 6);

  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F("  Date/Time: "));
  if (gps.date.isValid())
  {
    Serial.print(gps.date.day());
    Serial.print(F("/"));
    Serial.print(gps.date.month());
    Serial.print(F("/"));
    Serial.print(gps.date.year());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.print(F(" "));
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(F(":"));
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(F(":"));
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
  }
  else
  {
    Serial.print(F("INVALID"));
  }

  Serial.println();
}
