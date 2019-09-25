/*
  display SD card file data example

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

#define FILE_CONTENTS_LINE 13
#define CONTENTS_LINE_HIGHT_PIXEL 16
#define FILE_CONTENTS_HIGHT (FILE_CONTENTS_LINE * CONTENTS_LINE_HIGHT_PIXEL)
#define BOTTOM_FILE_CONTENTS 223

#define DISP_UPDATE_DELAY_MS 50
#define SD_READ_INTERVAL_MS 1000
#define SD_READ_INTERVAL_COUNT (SD_READ_INTERVAL_MS/DISP_UPDATE_DELAY_MS)

byte updateCount;

// definition for file data
#define RCV_BUF_SIZE 100
unsigned long baudRate = 115200;
char sbuf[RCV_BUF_SIZE];
char rcvBuff[RCV_BUF_SIZE];
byte rcvCnt;
boolean rcv1Line = 0;
byte rcvData;

TFT_eSPI tft = TFT_eSPI();

TFT_eSprite spriteContents = TFT_eSprite(&tft); // Sprite object File Contents
TFT_eSprite spriteFileName = TFT_eSprite(&tft); // Sprite object File Name

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

String read_sd_file_line() {

  rcvCnt = 0;
  String str;
  
  if(openFile){
    int readableSize = openFile.available();
    while (readableSize) {
      char rcvData = char(openFile.read());
      readableSize = openFile.available();
      str += rcvData;
      if (rcvData > 31 && rcvData < 127) {
        if( rcvCnt < (RCV_BUF_SIZE-1) ) {
          rcvBuff[rcvCnt] = rcvData;
          rcvCnt++;
        }
      }
      else if (rcvData == '\r') {
        readableSize = 0;
      }
    }
    if (rcvCnt == 0) {
      close_sd_file();
    }
  }

  if (rcvCnt > 0) {
   rcv1Line = 1;
   rcvBuff[rcvCnt] = '\0';
  }
  
  return str;
}

void close_sd_file() {
  if(sdFileIsOpened){
    Serial.println("Close SD File");
    openFile.close();
    sdFileIsOpened = false;
  }
}

void sd_task() {
  updateCount++;
  if(updateCount >= SD_READ_INTERVAL_COUNT) {
    updateCount = 0;
    if (sdHasError) {
      //retry establishing SD connection
      spiSD.begin(SD_CLK, SD_MISO, SD_MOSI, SD_SS);
      if(!SD.begin( SD_SS, spiSD, SDSPEED)){
       Serial.println("Card Mount Failed");
      }
      Serial.println("Card Mount Succeeded");
      sdHasError = false;
      open_sd_file();
    }
    else {
      if (sdFileIsOpened) {
        Serial.println("read result:");
        Serial.println(read_sd_file_line());
      }
      else {
        Serial.println("SD File is not opened");
        Serial.println("open SD File");
        open_sd_file();
      }
    }
  }
}
 
void setup() {

 WiFi.disconnect(true); //disable wifi

 Serial.begin(115200);

 Serial.println("start");

 tft.init();
 tft.setRotation(3);
 tft.fillScreen(TFT_BLACK);

 // Create a sprite for scrolling strings(all received uart data)
 spriteContents.setColorDepth(8);
 spriteContents.createSprite(261, FILE_CONTENTS_HIGHT);
 spriteContents.fillSprite(TFT_BLUE); // Fill sprite with blue
 spriteContents.setScrollRect(0, 0, 261, FILE_CONTENTS_HIGHT, TFT_BLUE);     // here we set scroll gap fill color to blue
 spriteContents.setTextColor(TFT_WHITE); // White text, no background
 spriteContents.setTextDatum(BR_DATUM);  // Bottom right coordinate datum

 // Create a sprite for baudRate rate
 spriteFileName.setColorDepth(8);
 spriteFileName.createSprite(128, 16);
 spriteFileName.fillSprite(TFT_BLACK); // Fill sprite with black
 spriteFileName.setScrollRect(0, 0, 128, 16, TFT_BLACK);     // here we set scroll gap fill color to blue
 spriteFileName.setTextColor(TFT_WHITE); // White text, no background
 spriteFileName.setTextDatum(BR_DATUM);  // Bottom right coordinate datum

 rcv1Line = 1;
 rcvBuff[0] = ' ';
 rcvBuff[1] = '\0';

 sdHasError = false;
 spiSD.begin(SD_CLK, SD_MISO, SD_MOSI, SD_SS);
 if(!SD.begin( SD_SS, spiSD, SDSPEED)){
   Serial.println("Card Mount Failed");
   sdHasError = true;
 }
 else {
   Serial.println("Card Mount Succeeded");
 }
 updateCount = 0;

 open_sd_file();
}

void loop() {
  
  if(rcv1Line) {
   spriteContents.setTextColor(TFT_BLUE); // color for dummy write
   int16_t widthX = spriteContents.drawString(rcvBuff, 260, (FILE_CONTENTS_HIGHT-1), 2); // dummy write for string width
   spriteContents.setTextColor(TFT_WHITE); // white text
   spriteContents.drawString(rcvBuff, widthX, (FILE_CONTENTS_HIGHT-1), 2); // plot string in font 2
  }
  
  sprintf(sbuf, "file: %s", TEXT_FILE_NAME);
  spriteFileName.drawString(sbuf, 127, 10, 1); // draw File Name
  
  if(rcv1Line) {
   spriteContents.pushSprite(0, (BOTTOM_FILE_CONTENTS - FILE_CONTENTS_HIGHT));
  }
  spriteFileName.pushSprite(135, 230);
  
  delay(DISP_UPDATE_DELAY_MS); // wait so things do not scroll too fast
  
  if(rcv1Line) {
   rcv1Line = 0;
   spriteContents.scroll(0,(-1)*CONTENTS_LINE_HIGHT_PIXEL); // scroll contents pixels 16 up
  }
  spriteFileName.scroll(0,(-1)*CONTENTS_LINE_HIGHT_PIXEL);

  sd_task();
}

