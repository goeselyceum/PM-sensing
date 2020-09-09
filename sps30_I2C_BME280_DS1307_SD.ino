// Sketch, based on https://github.com/Sensirion/embedded-sps/blob/master/sps30-i2c/sps30_example_usage.c
#include <Wire.h>
#include <sps30.h>            //I2C adress = 0x69
#include <Adafruit_Sensor.h>  //I2C adress = 
#include <Adafruit_BME280.h>
#include "RTClib.h"           //I2C adress = 0x68
#include <SPI.h>
#include <SD.h>               


#define DEBUG //comment this line to turn off Serial.print


RTC_DS1307 rtc;
Adafruit_BME280 bme; // I2C

bool logFlag = true;
const int chipSelect = 10;
int logSeconds = 0;
int seconds = 0;

void setup() {
  s16 ret;
  u8 auto_clean_days = 4;
  u32 auto_clean;

  Serial.begin(9600);
  rtc.begin();

  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  bme.begin();
  delay(2000);

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

  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // if card failed don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");
  delay(100);  //wait to finish initialisation


  File dataFile = SD.open("pmlog.txt", FILE_WRITE);
  if (dataFile) {
    dataFile.print("datum");// Writing column headers voor data-export Excel
    dataFile.print(";");
    dataFile.print("tijd");
    dataFile.print(";");
    dataFile.print("temp");
    dataFile.print(";");
    dataFile.print("hum");
    dataFile.print(';');
    dataFile.print("PM2.5");
    dataFile.print(";");
    dataFile.println("PM10");
    dataFile.close();
    Serial.println("Wrote headers");
  }
  else {
    Serial.println("error opening pmlog.txt");
    Serial.println(" ");
  }
  delay(2000);
}

void loop() {
  struct sps30_measurement m;
  char serial[SPS_MAX_SERIAL_LEN];
  u16 data_ready;
  s16 ret;

  DateTime now = rtc.now(); //take a readig from the RTC
  seconds = now.second(); //store seconds in a variable for timing datalogging

  do {
    ret = sps30_read_data_ready(&data_ready); //check SPS30 for readyness
    if (ret < 0) {
      Serial.print("error reading data-ready flag: ");
      Serial.println(ret);
    } else if (!data_ready)
      Serial.print("data not ready, no new measurement available\n");
    else
      break;
    delay(100); /* retry in 100ms */
  } while (1);

  ret = sps30_read_measurement(&m); // take a reading from SPS30
  if (ret < 0) {
    Serial.print("error reading measurement\n");
  } else {
#ifdef DEBUG
    Serial.print("PM  2.5: "); // print values from SPS30,  BME280
    Serial.println(m.mc_2p5);
    Serial.print("PM 10.0: ");
    Serial.println(m.mc_10p0);

    Serial.print("Temperature = ");
    Serial.print(bme.readTemperature());
    Serial.println(" *C");
    Serial.print("Humidity = ");
    Serial.print(bme.readHumidity());
    Serial.println(" %");
    Serial.print("Seconds: ");
    Serial.println(now.second(), DEC);

    Serial.println();
#endif
  }

  //check if a second has elapsed, otherwise data is logged during a complete second
  if (seconds == logSeconds + 1)  {
    logFlag = true;
  }

  // logging happens here
  if (seconds == 0 && logFlag == true) {
    File dataFile = SD.open("pmlog.txt", FILE_WRITE);

    // if the file is available, write to it:
    if (dataFile) {
      dataFile.print(now.day(), DEC);
      dataFile.print('/');
      dataFile.print(now.month(), DEC);
      dataFile.print('/');
      dataFile.print(now.year(), DEC);
      dataFile.print(";");
      dataFile.print(now.hour(), DEC);
      dataFile.print(':');
      dataFile.print(now.minute(), DEC);
      dataFile.print(";");
      dataFile.print(bme.readTemperature());
      dataFile.print(";");
      dataFile.print(bme.readHumidity());
      dataFile.print(";");
      dataFile.print(m.mc_2p5);
      dataFile.print(";");
      dataFile.println(m.mc_10p0);
      dataFile.close();
      logFlag = false;
      logSeconds = now.second();
      #ifdef DEBUG
      Serial.println("Logged");
      #endif
    }
    // if the file isn't open, pop up an error:
    else {
      Serial.println("error opening PMlog.txt");
      Serial.println(" ");
    }
  }
  delay(1000);
}
