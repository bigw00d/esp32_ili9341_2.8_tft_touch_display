/*
  SD card basic file example

 This example shows how to create and destroy an SD card file
 The circuit:
 * SD card attached to SPI bus as follows:
 ** CLK - pin 14
 ** MISO - pin 2
 ** MOSI - pin 15
 ** SS - pin 13 

 */
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

String SD_read() {

    String str;
    File file = SD.open(TEXT_FILE_NAME, FILE_READ);

    if(file){
        //unit character(1byte)
        while (file.available()) {
            str += char(file.read());
        }
    } else{
        Serial.println(" error...");
    }
    file.close();
    
    return str;
}

void setup(){
  Serial.begin(115200);
  spiSD.begin(SD_CLK, SD_MISO, SD_MOSI, SD_SS);
  if(!SD.begin( SD_SS, spiSD, SDSPEED)){
    Serial.println("Card Mount Failed");
    return;
  }
  Serial.println("Card Mount Succeeded");
  Serial.println("read result:");
  Serial.println(SD_read());
}

void loop() {
  ;
}


