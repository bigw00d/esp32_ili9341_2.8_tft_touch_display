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

#include "Update.h"
 
#include <WiFiMulti.h>
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

#define TEXT_FILE_NAME "/hello.txt"
#define CONFIG_FILE_NAME "/config.txt"

#define SW_VERSION "VERSION001"

typedef struct {
    char *strMD5;
}DlInfo;

typedef enum {
    DownloadingAndSaving_e = 0x01,
    Flashing_e
}DlState;


boolean sdHasError;
boolean sdFileIsOpened;

File openFile;

File file;

TFT_eSPI tft = TFT_eSPI();

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

void progressFunc(DlState state, int percent){
  Serial.printf("state = %d - percent = %d\n", state, percent);
}

void error(char *message){
  // printf("%s\n", message);
  Serial.printf("%s\n", message);
}

int readData(uint8_t *buffer, int bytes){
  return file.read(buffer, bytes);
}

void startFl(void){
  //write bin file to sdcard
  file = SD.open("fw.bin", FILE_READ);
}

void endFl(void){
  file.close();
}

int sd_firmupdate_start(DlInfo &info) {
    int downloaded = 0;
    int written = 0;
    int total = 1;
    uint8_t buff[1024] = { 0 };
    size_t size = sizeof(buff);
    int ret = 0;

    error("Downloading ... done -> Flashing ..."); 
    if (Update.begin(total, U_FLASH)){
//        Update.setMD5(info.strMD5);
        Update.setMD5("5EAD17D0BF2DA78FF0FBB41B0D5F53C7"); // sample
        downloaded = 0;
        int r;
        startFl();
        while (!Update.isFinished()) {
            //read sdcard
            r = readData(buff, size);
            written = Update.write(buff, r);
            if (written > 0) {
                if(written != r){
                    error("Flashing chunk not full ... warning!");
                }
                downloaded += written;
                progressFunc(Flashing_e, (100*downloaded)/total);
            } else {
                error("Flashing ... failed!");
                ret = -1;
                break;
            }
        }
        endFl();
        if(downloaded == total && Update.end()){
            error("Flashing ... done!");
            delay(100);
            ESP.restart();                
        } else {
            error("Flashing or md5 ... failed!"); 
            ret = -1;
        }
    } else {
        error("Flashing init ... failed!");
        ret = -1;
      } 

    return ret;
}


int readln(File* df, char* buf, int len)
{
    *buf = '\0';
    int pos = 0;    
     
    while (df->available()) {
        char ch = df->read();
        *(buf + pos) = ch;
        if (ch == 0x0a) {
            *(buf + pos + 1) = '\0';
            return pos;
        }
        pos++;
        if (pos >= len) {
            Serial.println("Overflowed!!");
            *buf = '\0';
            return 0;
        }
    }
}

#define CONFIG_BUF_SIZE 31
File configFile;
char configBuff[CONFIG_BUF_SIZE];

void get_sd_config() {
  Serial.print("Open SD CONFIG");
  configFile = SD.open(CONFIG_FILE_NAME, FILE_READ);
  if(configFile){
    Serial.println("config:");
    memset(configBuff, 0, CONFIG_BUF_SIZE);
    if (readln(&configFile, configBuff, (CONFIG_BUF_SIZE-1)) > 0) {
        Serial.print(configBuff);
    }
    configFile.close();
  } else{
    configFile.close();
    Serial.println("...failed->create new file");
    create_sd_config();
  }
}

void create_sd_config() {
  Serial.println("Create SD CONFIG");
  configFile = SD.open(CONFIG_FILE_NAME, FILE_WRITE);
  configFile.print(SW_VERSION);
  configFile.close();
}

const esp_partition_t *running;

typedef enum {
  Runnning_e = 0x01,
  Fota_e  
}SysState;
SysState state = Runnning_e;
char strMD5[50];

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

 memset(strMD5, 0, 50);

}

char buf[100];

void loop() {

  get_sd_config();


  Serial.println("display file name");
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK); // Set the font color and the background color
  tft.setTextSize(2);
  tft.setCursor(0, 0); // Set cursor at top left of screen

  tft.println(); 
  tft.print("File:");
  tft.println(TEXT_FILE_NAME);

  // const esp_partition_t *running = esp_ota_get_running_partition();
  running = esp_ota_get_running_partition();
  Serial.printf("Running partition type %d subtype %d (offset 0x%08x)\r\n",
             running->type, running->subtype, running->address);

  switch(state)
  {
    case Runnning_e:
      Serial.println("state: Runnning_e");
      break;
    case Fota_e:
      Serial.println("state: Fota_e");
//      DlInfo info;
//      info.strMD5 = strMD5;
//      sd_firmupdate_start(info);
      break;
    default:
      Serial.println("state: default");
      break;
  }

  memset(buf, 0, 100);
  sprintf(buf, "Running partition type %d", running->type);
  tft.print(buf);
  tft.println(); 
  memset(buf, 0, 100);
  sprintf(buf, "Running partition subtype %d", running->subtype);
  tft.print(buf);
  tft.println(); 
  memset(buf, 0, 100);
  sprintf(buf, "(offset 0x%08x)",
             running->address);
  tft.print(buf);
  tft.println(); 

  memset(buf, 0, 100);
  sprintf(buf, "soft version: %s", SW_VERSION);
  tft.print(buf);
  tft.println(); 

  memset(buf, 0, 100);
  sprintf(buf, "sd soft version: %s", configBuff);
  tft.print(buf);
  tft.println(); 

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

