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

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  delay(2000);

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
  Serial.print("Measurements started\n");
  delay(2000);
  String dataString;
  dataString += ("lat");
  dataString += ",";
  dataString += ("lng");
  dataString += ",";
  dataString += ("date");
  dataString += ",";
  dataString += ("time");
  dataString += ",";
  dataString += ("PM2.5");
  dataString += (",");
  dataString += ("PM10");
  dataString += "\r\n";
  Serial.println(dataString);
  writeFile(SD, "/data.txt", dataString.c_str());
  delay(2000);
}

void loop() {
  while (Serial2.available() > 0)
    if (gps.encode(Serial2.read()))
      //displayInfo();

      if (millis() > 5000 && gps.charsProcessed() < 10) {
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
      displayInfo();
    }

    if (gps.time.second() % 30 == 0) {
        String dataString;
        dataString += String(gps.location.lat(), 6);
        dataString += ",";
        dataString += String(gps.location.lng(), 6);
        dataString += ",";
        dataString += gps.date.day();
        dataString += "/";
        dataString += gps.date.month();
        dataString += "/";
        dataString += gps.date.year();
        dataString += ",";
        dataString += gps.time.hour();
        dataString += ":";
        dataString += gps.time.minute();
        dataString += ",";
        dataString += m.mc_2p5;
        dataString += ",";
        dataString += m.mc_10p0;
        dataString += "\r\n";
        appendFile(SD, "/data.txt", dataString.c_str());
        Serial.println();
        Serial.println(dataString);
        Serial.println();
      
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

void writeFile(fs::FS & fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (file.print(message)) {
    Serial.println("File written");
  } else {
    Serial.println("Write failed");
  }
  file.close();
}

void appendFile(fs::FS & fs, const char * path, const char * message) {
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Failed to open file for appending");
    return;
  }
  if (file.print(message)) {
    Serial.println("Data appended");
    Serial.println();
  } else {
    Serial.println("Append failed");
  }
  file.close();
}
