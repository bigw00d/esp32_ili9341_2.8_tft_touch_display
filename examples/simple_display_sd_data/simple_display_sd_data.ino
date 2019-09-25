/*
  display SD card file data example(simple version)

 Arduino board manager:
  https://dl.espressif.com/dl/package_esp32_index.json

 Arduino Settings(use Arduino 1.8.5):    
  ボード ESP32 Dev Module
  Flash Mode  QIO
  Flash Frequency 40MHz
  CPU Frequency 240MHz
  Flash Size  4M (32Mb)
  Partition Scheme  Huge App (3MB No OTA/1MB SPIFFS)
  Upload Speed  115200
  Core Debug Level なし

 The circuit:
 * SD card attached to SPI bus as follows:
 ** CLK - pin 14
 ** MISO - pin 2
 ** MOSI - pin 15
 ** SS - pin 13 

 */
#include <WiFiMulti.h>
#include <TFT_eSPI.h>

#include "FS.h"
#include "SD.h"
#include "SPI.h"
SPIClass spiSD(HSPI);
#define SD_CLK 14
#define SD_MISO 2
#define SD_MOSI 15
#define SD_SS 13
#define SDSPEED 27000000

#define TEXT_FILE_NAME "/hello.txt"

TFT_eSPI tft = TFT_eSPI();

boolean sdHasError;
boolean sdFileIsOpened;

File openFile;

void open_sd_file() {
  Serial.print("Open SD File");
  boolean ret = false;
  openFile = SD.open(TEXT_FILE_NAME, FILE_READ);
  if(openFile){
    Serial.println("...succeeded");
    sdFileIsOpened = true;
  } else{
    Serial.println("...failed");
    openFile.close();
  }
}

void close_sd_file() {
  if(sdFileIsOpened){
    Serial.println("Close SD File");
    openFile.close();
    sdFileIsOpened = false;
  }
}

void setup() {

 WiFi.disconnect(true); //disable wifi

 Serial.begin(115200);

 Serial.println("start");

 tft.init();
 tft.setRotation(3);
 tft.fillScreen(TFT_BLACK);

 sdHasError = false;
 sdFileIsOpened = false;
 spiSD.begin(SD_CLK, SD_MISO, SD_MOSI, SD_SS);
 if(!SD.begin( SD_SS, spiSD, SDSPEED)){
   Serial.println("Card Mount Failed");
   sdHasError = true;
 }
 else {
   Serial.println("Card Mount Succeeded");
 }

}

void loop() {

  Serial.println("display file name");
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set the font color and the background color
  tft.setTextSize(3);
  tft.setCursor(0, 0); // Set cursor at top left of screen

  tft.println(); 
  tft.print("File:");
  tft.println(TEXT_FILE_NAME);

  delay(5000);

  //retry establishing SD connection
  if (sdHasError) {
    //retry establishing SD connection
    spiSD.begin(SD_CLK, SD_MISO, SD_MOSI, SD_SS);
    if(!SD.begin( SD_SS, spiSD, SDSPEED)){
     Serial.println("Card Mount Failed");
    }
    Serial.println("Card Mount Succeeded");
    sdHasError = false;
  }

  Serial.println("display file contents");
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set the font color and the background color
  tft.setCursor(0, 0); // Set cursor at top left of screen

  if (sdHasError) {
    tft.setTextSize(3);
    tft.println("");
    tft.println("cannot access SD File"); // println moves cursor down for a new line
    Serial.println("cannot access SD File");
  }
  else {
    open_sd_file();
    
    if (sdFileIsOpened) {
      String str;
      int readableSize = openFile.available();
      tft.setTextSize(2);
      while (readableSize) {
        char readData = char(openFile.read());
        if (readData > 31 && readData < 127) { // filter(displayable data)
          str += readData;
        }
        else if (readData == '\r') {
          tft.println(str);
          Serial.println(str);
          str = "";
        }
        readableSize = openFile.available();
      }
  
      if (str.length() > 0) {
        tft.println(str);
        Serial.println(str);
      }

      close_sd_file();
    }
    else {
      tft.setTextSize(3);
      tft.println("");
      tft.println("cannot open File"); // println moves cursor down for a new line
      Serial.println("cannot open File");
    }
  }

  delay(5000);

}

