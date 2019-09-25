/*
  display SD card file data example

 This example shows how to create and destroy an SD card file
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

byte updateCount;

// definition for uart
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

String SD_read() {

   rcvCnt = 0;

   String str;
   File file = SD.open(TEXT_FILE_NAME, FILE_READ);

   if(file){
       //unit character(1byte)
       while (file.available()) {
           char rcvData = char(file.read());
           str += rcvData;
           if (rcvData > 31 && rcvData < 127) {
             if( rcvCnt < (RCV_BUF_SIZE-1) ) {
               rcvBuff[rcvCnt] = rcvData;
               rcvCnt++;
             }
           }
       }
   } else{
       Serial.println(" error...");
   }
   file.close();

   if (rcvCnt > 0) {
     rcv1Line = 1;
     rcvBuff[rcvCnt] = '\0';
   }
   
   return str;
}
 
void setup() {

 WiFi.disconnect(true); //disable wifi

 delay(500); // wait for current consumption to down

 Serial.begin(115200);
 
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

 spiSD.begin(SD_CLK, SD_MISO, SD_MOSI, SD_SS);
 if(!SD.begin( SD_SS, spiSD, SDSPEED)){
   Serial.println("Card Mount Failed");
   return;
 }
 Serial.println("Card Mount Succeeded");
 updateCount = 0;
}

void loop() {

 // draw string(scrolling strings)
 if(rcv1Line) {
   spriteContents.setTextColor(TFT_BLUE); // color for dummy write
   int16_t widthX = spriteContents.drawString(rcvBuff, 260, (FILE_CONTENTS_HIGHT-1), 2); // dummy write for string width
   spriteContents.setTextColor(TFT_WHITE); // white text
   spriteContents.drawString(rcvBuff, widthX, (FILE_CONTENTS_HIGHT-1), 2); // plot string in font 2
 }

 sprintf(sbuf, "file: %s", TEXT_FILE_NAME);
 spriteFileName.drawString(sbuf, 127, 10, 1); // draw AREA6

 if(rcv1Line) {
   spriteContents.pushSprite(0, (BOTTOM_FILE_CONTENTS - FILE_CONTENTS_HIGHT));
 }
 spriteFileName.pushSprite(135, 230);

 delay(50); // wait so things do not scroll too fast

 if(rcv1Line) {
   rcv1Line = 0;
   spriteContents.scroll(0,-16); // scroll contents pixels 16 up
 }
 spriteFileName.scroll(0,-16);

 updateCount++;
 if(updateCount >= 20) {
   updateCount = 0;
   Serial.println("read result:");
   Serial.println(SD_read()); 
 }

}

