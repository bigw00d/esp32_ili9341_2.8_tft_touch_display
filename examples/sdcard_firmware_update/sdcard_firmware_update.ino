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
#include <TFT_eSPI.h>

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

#define UPDATE_FILE_NAME "/update.bin"
#define MAX_SD_MOUNT_RETRY_COUNT 2

TFT_eSPI tft = TFT_eSPI();

// perform the actual update from a given stream
void performUpdate(Stream &updateSource, size_t updateSize) {
  tft.print("updating...");
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
      tft.println("done!");
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
         Serial.println("update.bin is available");
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

  Serial.println("Try to start update");
  size_t updateSize = updateBin.size();
  performUpdate(updateBin, updateSize);  
  updateBin.close();
  // whe finished remove the binary from sd card to indicate end of the process
  fs.remove("/update.bin");

  rebootEspWithReason("Reboot after Update"); 
}

void tft_init() {
  tft.init();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set the font color and the background color
  tft.setTextSize(2);
  tft.setCursor(0, 0); // Set cursor at top left of screen
}

void setup() {
  uint8_t cardType;
  Serial.begin(115200);
  Serial.println("Welcome to the SD-Update example!");

  tft_init();

  tft.print("SD card mount...");
  boolean sdMoundIsOK;
  int count;
  for(count = 0; count < MAX_SD_MOUNT_RETRY_COUNT; count++) {
    spiSD.begin(SD_CLK, SD_MISO, SD_MOSI, SD_SS);
    if(!SD.begin( SD_SS, spiSD, SDSPEED)){
      sdMoundIsOK = false;
    }
    else {
      sdMoundIsOK = true;
      break;
    }
  }
  
  if(sdMoundIsOK) {
    tft.println("OK");
    Serial.println("Card Mount Succeeded");
    tft.print("Check new firm");
    tft.print("...");
    if (updateFileIsAvailable(SD)) {
      tft.println("DETECT");
      countdownEventTimer("Start firm update", 10);
      updateFromFS(SD);
    }
    else {
      tft.println("NO FILE");
    }
  }
  else {
    tft.println("FAILED"); // sd mount failed
  }

}

void rebootEspWithReason(String reason){
    Serial.println(reason);
    delay(1000);
    ESP.restart();
}

void countdownEventTimer(String event, int sec) {
  tft.printf("%s in", event.c_str());
  int16_t curX = tft.getCursorX();
  int16_t curY = tft.getCursorY();
  int count;
  for(count = sec; count >= 0; count--) {
    Serial.printf("%d ", count);
    tft.setCursor(curX, curY);
    tft.printf(" %d sec  ", count);
    delay(1000);
  }
  tft.println("");
}

void startRestartTimer(String reason, int sec) {
  countdownEventTimer("Restart", 10);
  rebootEspWithReason(reason);
}

#define STRING_BUF_SIZE 100
const esp_partition_t *running;
char buf[STRING_BUF_SIZE];

//will not be reached
void loop() {
  running = esp_ota_get_running_partition();
  Serial.printf("Running partition type %d subtype %d (offset 0x%08x)\r\n",
             running->type, running->subtype, running->address);
  tft.printf("Partition type %d\n", running->type);
  tft.printf("Partition subtype %d\n", running->subtype);
  tft.printf("Partition address 0x%06x\n", running->address);

  startRestartTimer("Finish Normal Routine", 10);
}

