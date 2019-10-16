/*
  display SD card file data example(simple version)

 Arduino board manager:
  https://dl.espressif.com/dl/package_esp32_index.json

 Arduino Settings(use Arduino 1.8.5):    
  Board ESP32 Dev Module
  Flash Mode  QIO
  Flash Frequency 40MHz
  CPU Frequency 240MHz
  Flash Size  4M (32Mb)
  Partition Scheme  Default 4MB with spiffs(1.2MB APP / 1.5MB SPIFFS)
  Upload Speed  115200
  Core Debug Level non

 The circuit:
 * SD card attached to SPI bus as follows:
 ** CLK - pin 14
 ** MISO - pin 2
 ** MOSI - pin 15
 ** SS - pin 13 

 */

#include <Update.h>

#include "esp_partition.h"
#include "esp_ota_ops.h"

#include "FS.h"
#include "SD.h"
#include "SPI.h"
SPIClass spiSD(HSPI);
#define SD_CLK 14
#define SD_MISO 2
#define SD_MOSI 15
#define SD_SS 13
#define SDSPEED 27000000

const esp_partition_t *running;

// perform the update from a given stream
void performUpdate(Stream &updateSource, size_t updateSize) {
   if (Update.begin(updateSize)) {      
      size_t written = Update.writeStream(updateSource);
      if (written == updateSize) {
         Serial.println("Written : " + String(written) + " successfully");
      }
      else {
         Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
      }
      if (Update.end()) {
         Serial.println("done!");
         if (Update.isFinished()) {
            Serial.println("Update successfully completed. Rebooting.");
         }
         else {
            Serial.println("Update not finished? Something went wrong!");
         }
      }
      else {
         Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      }
   }
   else
   {
      Serial.println("Not enough space to begin OTA");
   }
}

#define UPDATE_FILE_NAME "/update.bin"

// check update.bin if available
boolean updateFileIsAvailable(fs::FS &fs) {
  boolean ret = false;
  
  File updateBin = fs.open(UPDATE_FILE_NAME);
  if (updateBin) {
    if(updateBin.isDirectory()){
       Serial.println("Error, update.bin is not a file");
    }
    else {
      size_t updateSize = updateBin.size();
      if (updateSize > 0) {
         ret = true;
      }
      else {
         Serial.println("Error, file is empty");
      }
    }
    updateBin.close();
  }
  else {
    Serial.println("Could not load update.bin from sd root");
  }

  return ret;
}

// check given FS for valid update.bin and perform update if available
void updateFromFS(fs::FS &fs) {
  File updateBin = fs.open(UPDATE_FILE_NAME);

  size_t updateSize = updateBin.size();
  performUpdate(updateBin, updateSize);  
  updateBin.close();

  // remove the update file from sd card to indicate end of the process
  fs.remove("/update.bin");

  reboot();
}

void setup() {
  Serial.begin(115200);

  running = esp_ota_get_running_partition();
  Serial.printf("running partition address:0x%08x", running->address);
  Serial.println("");

  Serial.print("Checking new firmware...");
  spiSD.begin(SD_CLK, SD_MISO, SD_MOSI, SD_SS);
  boolean newFirmIsOK;
  if(!SD.begin( SD_SS, spiSD, SDSPEED)){
    newFirmIsOK = false;
  }
  else {
    if (updateFileIsAvailable(SD)) {
      newFirmIsOK = true;
    }
    else {
      newFirmIsOK = false;
    }
  }
  if (newFirmIsOK) {
    Serial.println("OK");
    Serial.println("Start firm update");
    updateFromFS(SD);
  }
  else {
    Serial.println("FAILED");
    reboot();
  }

}

void reboot(){
    Serial.println("Reboot");
    delay(3000);
    ESP.restart();
}

void loop() {
  ; //will not be reached  
}
