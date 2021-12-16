/***************************************************
 * M5Stack firmware to log the data from two ENV units
 * 
 * Hague Nusseck @ electricidea
 * v1.0 | 16.December.2021
 * https://github.com/electricidea/M5Stack_ENV-logger
 * 
 * 
 * Check the complete project at Hackster.io:
 * https://www.hackster.io/hague/sensor-comparison-temperature-humidity-and-air-pressure-554592
 * 
 * 
 * Distributed as-is; no warranty is given.
****************************************************/

#include <Arduino.h>

#include <M5Stack.h>
// install the library:
// pio lib install "M5Stack"

// Free Fonts for nice looking fonts on the screen
#include "Free_Fonts.h"

// logo with 150x150 pixel size in XBM format
// check the file header for more information
#include "electric-idea_logo.h"

#include "UNIT_ENV.h"

#include <Wire.h> 
#include "Adafruit_Sensor.h"
// pio lib install "adafruit/Adafruit Unified Sensor"
#include <Adafruit_BMP280.h>
// install the library:
// pio lib install "adafruit/Adafruit BMP280 Library"

#define SEALEVELPRESSURE_HPA (1013.25)

// logfile and addressfile definition
File log_data;

float start_height;

// M5Stack Unit ENV-I
// DHT12:   temperature and humidity sensor  I2C: 0x5C
// BMP280:  absolute air pressure sensor     I2C: 0x76
//
// M5Stack Unit ENVIII:
// SHT30:   temperature and humidity sensor  I2C: 0x44
// QMP6988: absolute air pressure sensor     I2C: 0x70

DHT12 dht12; //Preset scale CELSIUS and ID 0x5c.
Adafruit_BMP280 bme;
SHT3X sht30;
QMP6988 qmp6988;

#define MINUTE 60*1000
#define HALFMINUTE 30*1000
unsigned long nextMillis = 0;

// Buffer for snprintf calls
char String_buffer[256];

// forward declarations
void I2Cscan();
void clear_screen(){
    M5.Lcd.clear();   
    M5.Lcd.setCursor(0,0);
    M5.Lcd.println(""); 
}


void setup() {
  // start M5Stack without I2C
  M5.begin(true, true, true, false);
  M5.Power.begin();  
  // bool TwoWire::begin(int sda = -1, int scl = -1, uint32_t frequency = 0U)
  Wire.begin(17, 16);

  M5.Lcd.setBrightness(100); //Brightness (0: Off - 255: Full)
  // electric-idea logo
  M5.Lcd.fillScreen(TFT_BLACK);
  M5.Lcd.drawXBitmap((int)(320-logoWidth)/2, (int)(240-logoHeight)/2, logo, logoWidth, logoHeight, TFT_WHITE);
  delay(1500);

  M5.Lcd.setTextSize(1);
  // configure Top-Left oriented String output
  M5.Lcd.setTextDatum(TL_DATUM);
  M5.Lcd.setFreeFont(FF2);
  M5.Lcd.setBrightness(100); //Brightness (0: Off - 255: Full)
  
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE , BLACK);
  M5.Lcd.println("\n\n  ENV I & III Test\n");
  delay(1500);
  clear_screen(); 

  M5.Lcd.setFreeFont(FF1);
  // scan for I2C devices
  I2Cscan();
  delay(2500);
  clear_screen(); 

  
  char filename1[15];
  // creat new filename that does not exist at SD
  // Format:
  // log_000.txt = first file
  // log_001.txt = second file
  // ...
  // log_999.txt = last file!
  strcpy(filename1, "/log_000.txt");
  for (int i = 0; i < 1000; i++) {
    filename1[5] = '0' + i/100;
    filename1[6] = '0' + (i/10)%10;
    filename1[7] = '0' + i%10;
    // exit if filename does not exist
    // !do not open existing, write, sync after write!
    if (!SD.exists(filename1)) {
      break;
    }
  }
  // try to open the files for writing
  log_data = SD.open(filename1, FILE_WRITE);
  if(!log_data) {
    M5.Lcd.setTextColor(TFT_RED,TFT_BLACK);  
    M5.Lcd.println("\nCould not create\n");
    M5.Lcd.println(filename1);
    while(true){
      delay(100);
    }
  }
  M5.Lcd.println("\nWriting data to ");
  M5.Lcd.println(filename1);

  M5.Lcd.println("\n\nLogger Ready!");

  // header for data output
  log_data.print("ms\tdht12_Temperature\tdht12_Humidity\tbme_Pressure\tbme_Temperature\tbme_altitute_1\tbme_altitute_2\tsht30_Temperature\tsht30_Humidity\tqmp_Pressure\tqmp_altitute\n");
  // flush files to save data onto SD
  log_data.flush();


  unsigned status;
  
  // default settings
  status = bme.begin(0x76);  
  // You can also pass in a Wire library object like &Wire2
  // status = bme.begin(0x76, &Wire2)
  if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
      Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
      Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
      Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
      Serial.print("        ID of 0x60 represents a BME 280.\n");
      Serial.print("        ID of 0x61 represents a BME 680.\n");
  } else {
    Serial.println("[OK] BMP280 ready");
  }

  
  if(qmp6988.init()==1){
    Serial.println("[OK] QMP6988 ready");
  }

  // high precission mode
  qmp6988.setFilter(QMP6988_FILTERCOEFF_32);
  qmp6988.setOversamplingP(QMP6988_OVERSAMPLING_32X);
  qmp6988.setOversamplingT(QMP6988_OVERSAMPLING_4X);
  
  M5.Lcd.setTextColor(TFT_WHITE);

  
  start_height = bme.readAltitude(SEALEVELPRESSURE_HPA);

  nextMillis = millis() + 1000;
}


void loop() {
  M5.update();

  if(M5.BtnA.wasPressed()){
    delay(100); 
  } 

  if(M5.BtnB.wasPressed()){
    delay(100); 
  } 

  if(M5.BtnC.wasPressed()){
    delay(100); 
  } 
  
  // get actual time in miliseconds
  unsigned long currentMillis = millis();
  // next interval
  if(currentMillis > nextMillis) {
    float bme_Pressure = 0.0;
    float bme_Temperature = 0.0;
    float dht12_Temperature = 0.0;
    float dht12_Humidity = 0.0;
    float qmp_Pressure = 0.0;
    float sht30_Temperature = 0.0;
    float sht30_Humidity = 0.0;
    int n_average = 10;
    M5.Lcd.fillRect(0,0, M5.Lcd.width(), M5.Lcd.height(), TFT_BLACK);
    M5.Lcd.setFreeFont(FF3);
    M5.Lcd.setTextDatum(TC_DATUM);
    for(int i=0; i<n_average; i++){
      if (sht30.get() != 0) {
        M5.Lcd.drawString("-ERROR-", (int)(M5.Lcd.width()/2), 20, 1);
      } else {
        bme_Pressure += bme.readPressure();
        bme_Temperature += bme.readTemperature();
        qmp_Pressure += qmp6988.calcPressure();
        sht30_Temperature += sht30.cTemp;
        sht30_Humidity += sht30.humidity;
        dht12_Temperature += dht12.readTemperature();
        dht12_Humidity += dht12.readHumidity();
      }
    }
    bme_Pressure = bme_Pressure / n_average;
    bme_Temperature = bme_Temperature / n_average;
    qmp_Pressure = qmp_Pressure / n_average;
    sht30_Temperature = sht30_Temperature / n_average;
    sht30_Humidity = sht30_Humidity / n_average;
    dht12_Temperature = dht12_Temperature / n_average;
    dht12_Humidity = dht12_Humidity / n_average;
    char TextBuffer[100];
    sprintf(TextBuffer, "%3.2f*C", dht12_Temperature);
    M5.Lcd.drawString(TextBuffer, (int)(M5.Lcd.width()/2), 20, 1);
    sprintf(TextBuffer, "%3.2f%%", dht12_Humidity);
    M5.Lcd.drawString(TextBuffer, (int)(M5.Lcd.width()/2), 50, 1);

    sprintf(TextBuffer, "%3.2f*C", sht30_Temperature);
    M5.Lcd.drawString(TextBuffer, (int)(M5.Lcd.width()/2), 100, 1);
    sprintf(TextBuffer, "%3.2f%%", sht30_Humidity);
    M5.Lcd.drawString(TextBuffer, (int)(M5.Lcd.width()/2), 130, 1);

    sprintf(TextBuffer, "%3.2fhPa", qmp_Pressure/100.0F);
    M5.Lcd.drawString(TextBuffer, (int)(M5.Lcd.width()/2), 180, 1);
  
    
    //    ms
    //    dht12_Temperature
    //    dht12_Humidity
    //    bme_Pressure
    //    bme_Temperature
    //    bme_altitute_1
    //    bme_altitute_2
    //    sht30_Temperature
    //    sht30_Humidity
    //    qmp_Pressure
    //    qmp_altitute
    snprintf(String_buffer, sizeof(String_buffer), "%lu\t%7.3f\t%7.3f\t%7.3f\t%7.3f\t%7.3f\t%7.3f\t%7.3f\t%7.3f\t%7.3f\t%7.3f\n", 
              millis(), 
              dht12_Temperature, dht12_Humidity,
              bme_Pressure,bme_Temperature, qmp6988.calcAltitude(bme_Pressure, bme_Temperature), qmp6988.calcAltitude(bme_Pressure, dht12_Temperature),
              sht30_Temperature, sht30_Humidity,
              qmp_Pressure, qmp6988.calcAltitude(qmp_Pressure, sht30_Temperature));
    log_data.print(String_buffer);
    // flush files to save data onto SD
    log_data.flush();

    // every 30 seconds
    nextMillis = currentMillis + HALFMINUTE; 
  }
  delay(100);
}


//==============================================================
void I2Cscan(){
  // scan for i2c devices
  byte error, address;
  int nDevices;

  M5.Lcd.println("Scanning I2C bus...\n");
  Serial.println("Scanning I2C bus...\n");

  nDevices = 0;
  for(address = 1; address < 127; address++ ){
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    // Errors:
    //  0 : Success
    //  1 : Data too long
    //  2 : NACK on transmit of address
    //  3 : NACK on transmit of data
    //  4 : Other error
    if (error == 0){
      nDevices++;
      M5.Lcd.printf("[OK] %i 0x%.2X\n", nDevices, address);
      Serial.printf("[OK] %i 0x%.2X\n", nDevices, address);
    } else{
      if(error == 4)
        M5.Lcd.printf("[ERR] %i 0x%.2X\n", nDevices, address);
    }
  }
  M5.Lcd.printf("\n%i devices found\n\n", nDevices);
  Serial.printf("\n%i devices found\n\n", nDevices);
 }
