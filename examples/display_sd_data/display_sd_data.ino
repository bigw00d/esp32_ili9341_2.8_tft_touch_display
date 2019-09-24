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

byte updateCount;

// definition for graph
#define GRAPH_MAX_VALUE 60
#define GRAPH_MIN_VALUE 0
int graphVal = 1;
int grid = 0;
int tcount = 0;

// definition for uart
#define RCV_BUF_SIZE 100
unsigned long baudRate = 115200;
char sbuf[RCV_BUF_SIZE];
char rcvBuff[RCV_BUF_SIZE];
byte rcvCnt;
boolean rcv1Line = 0;
byte rcvData;

// definition for display items(graph, title, ....)
#define MAX_VALUE_STRLEN 10
enum DispValue {
 TITLE = 0,
 NUMBERS1,
 NUMBERS2,
 NUMBERS3,
 NUMBERS4,
 GRAPH_VALUE,
 NUM_DISP_VALUES
};
String values[NUM_DISP_VALUES] = {"\0"};
String labels[NUM_DISP_VALUES] = {"\0"};

TFT_eSPI tft = TFT_eSPI();

TFT_eSprite graph1 = TFT_eSprite(&tft); // Sprite object graph1
TFT_eSprite stext1 = TFT_eSprite(&tft); // Sprite object stext1 // title
TFT_eSprite stext2 = TFT_eSprite(&tft); // Sprite object stext2 // monitor values(4 numbers)
TFT_eSprite stext3 = TFT_eSprite(&tft); // Sprite object stext3 // graph value
TFT_eSprite stext4 = TFT_eSprite(&tft); // Sprite object stext4 // all received uart data(only a part of ascii)
TFT_eSprite stext5 = TFT_eSprite(&tft); // Sprite object stext5 // display baudRate rate

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
 Serial2.begin(baudRate, SERIAL_8N1, 26, 27); //rx:G26, tx:G27
 
 tft.init();
 tft.setRotation(3);
 tft.fillScreen(TFT_BLACK);

 // Create a sprite for the graph
 graph1.setColorDepth(8);
 graph1.createSprite(128, 61);
 graph1.fillSprite(TFT_BLACK); // Note: Sprite is filled with black when created

 // The scroll area is set to the full sprite size upon creation of the sprite
 // but we can change that by defining a smaller area using "setScrollRect()"if needed
 // parameters are x,y,w,h,color as in drawRect(), the color fills the gap left by scrolling
 //graph1.setScrollRect(64, 0, 64, 61, TFT_DARKGREY);  // Try this line to change the graph scroll area

 // Create a sprite for monitor values(4 numbers)
 stext1.setColorDepth(8);
 stext1.createSprite(128, 64);
 stext1.fillSprite(TFT_BLUE); // Fill sprite with blue
 stext1.setScrollRect(50, 0, 128, 64, TFT_BLUE); // label width is 50
 stext1.setTextColor(TFT_WHITE); // White text, no background
 stext1.setTextDatum(BR_DATUM);  // Bottom right coordinate datum

 // Create a sprite for title
 stext2.setColorDepth(8);
 stext2.createSprite(128, 16);
 stext2.fillSprite(TFT_DARKGREY);
 stext2.setScrollRect(0, 0, 128, 16, TFT_DARKGREY);
 stext2.setTextColor(TFT_WHITE); // White text, no background
 stext2.setTextDatum(BR_DATUM);  // Bottom right coordinate datum

 // Create a sprite for graph value
 stext3.setColorDepth(8);
 stext3.createSprite(128, 16);
 stext3.fillSprite(TFT_BLUE); // Fill sprite with blue
 stext3.setScrollRect(0, 0, 128, 16, TFT_BLUE);     // here we set scroll gap fill color to blue
 stext3.setTextColor(TFT_WHITE); // White text, no background
 stext3.setTextDatum(BR_DATUM);  // Bottom right coordinate datum

 // Create a sprite for scrolling strings(all received uart data)
 stext4.setColorDepth(8);
 stext4.createSprite(261, 128);
 stext4.fillSprite(TFT_BLUE); // Fill sprite with blue
 stext4.setScrollRect(0, 0, 261, 128, TFT_BLUE);     // here we set scroll gap fill color to blue
 stext4.setTextColor(TFT_WHITE); // White text, no background
 stext4.setTextDatum(BR_DATUM);  // Bottom right coordinate datum

 // Create a sprite for baudRate rate
 stext5.setColorDepth(8);
 stext5.createSprite(128, 16);
 stext5.fillSprite(TFT_BLACK); // Fill sprite with black
 stext5.setScrollRect(0, 0, 128, 16, TFT_BLACK);     // here we set scroll gap fill color to blue
 stext5.setTextColor(TFT_WHITE); // White text, no background
 stext5.setTextDatum(BR_DATUM);  // Bottom right coordinate datum

 rcv1Line = 1;
 rcvBuff[0] = ' ';
 rcvBuff[1] = '\0';

 for(int i=0; i<NUM_DISP_VALUES; ++i) {
   values[i] = " ";
   labels[i] = "LABEL" + String(i);
 }
 values[GRAPH_VALUE] = "0"; //graph initial value

 spiSD.begin(SD_CLK, SD_MISO, SD_MOSI, SD_SS);
 if(!SD.begin( SD_SS, spiSD, SDSPEED)){
   Serial.println("Card Mount Failed");
   return;
 }
 Serial.println("Card Mount Succeeded");
 updateCount = 0;
}


void loop() {
 String pieces[10] = {"\0"};

 // update graph value
 int newGraphVal = values[GRAPH_VALUE].toInt();
 if ( (GRAPH_MIN_VALUE <= newGraphVal) && (newGraphVal <= GRAPH_MAX_VALUE) ) {
   graphVal = newGraphVal;  // update value
 }

 // draw point in graph1 sprite at far right edge (this will scroll left later)
 graph1.drawFastVLine(127,GRAPH_MAX_VALUE-graphVal,2,TFT_YELLOW); // draw 2 pixel point on graph

 // draw numbers
 stext1.fillSprite(TFT_BLUE);
 stext1.drawString(labels[NUMBERS1], 6, 16, 2); // draw stext1 LABEL1
 stext1.drawString(labels[NUMBERS2], 6, 32, 2); // draw stext1 LABEL2
 stext1.drawString(labels[NUMBERS3], 6, 48, 2); // draw stext1 LABEL3
 stext1.drawString(labels[NUMBERS4], 6, 64, 2); // draw stext1 LABEL4
 stext1.drawString(values[NUMBERS1], 127, 16, 2); // draw stext1 VALUE1
 stext1.drawString(values[NUMBERS2], 127, 32, 2); // draw stext1 VALUE2
 stext1.drawString(values[NUMBERS3], 127, 48, 2); // draw stext1 VALUE3
 stext1.drawString(values[NUMBERS4], 127, 64, 2); // draw stext1 VALUE4

 // draw graph value
 stext3.fillSprite(TFT_BLUE); // Fill sprite with blue
 stext3.drawString(labels[GRAPH_VALUE], 6, 16, 2); // draw AREA4_LABEL
 stext3.drawString(values[GRAPH_VALUE], 127, 16, 2); // draw AREA4_VALUE

 // draw string(scrolling strings)
 if(rcv1Line) {
   stext4.setTextColor(TFT_BLUE); // color for dummy write
   int16_t widthX = stext4.drawString(rcvBuff, 260, 127, 2); // dummy write for string width
   stext4.setTextColor(TFT_WHITE); // white text
   stext4.drawString(rcvBuff, widthX, 127, 2); // plot string in font 2
 }

 // draw string(baudRate rate)
 sprintf(sbuf, "baudRate: %d", baudRate);
 stext5.drawString(sbuf, 127, 10, 1); // draw AREA6

 // draw string(title)
 stext2.fillSprite(TFT_DARKGREY);
 stext2.drawString(labels[TITLE], 6, 0, 2); // draw AREA4_LABEL
 stext2.drawString(values[TITLE], 127, 0, 2); // draw AREA4_VALUE

 // Push the sprites onto the TFT at specied coordinates
 graph1.pushSprite(135, 0);
 stext1.pushSprite(0, 20);
 stext2.pushSprite(0, 0);
 stext3.pushSprite(135, 65);
 if(rcv1Line) {
   stext4.pushSprite(0, 95);
 }
 stext5.pushSprite(135, 230);

 delay(50); // wait so things do not scroll too fast

 // Now scroll the sprites scroll(dt, dy) where:
 // dx is pixels to scroll, left = negative value, right = positive value
 // dy is pixels to scroll, up = negative value, down = positive value
 graph1.scroll(-1, 0); // scroll graph 1 pixel left, 0 up/down
 stext1.scroll(0,-64); // scroll stext 0 pixels left/right, 16 up
 stext3.scroll(0,-16); // scroll stext 3 pixels left/right, 16 up
 if(rcv1Line) {
   rcv1Line = 0;
   stext4.scroll(0,-16); // scroll stext 4 pixels left/right, 16 up
 }
 stext5.scroll(0,-16);

 // draw the grid on far right edge of graph
 grid++;
 if (grid >= 10)
 { // draw a vertical line if we have scrolled 10 times (10 pixels)
   grid = 0;
   graph1.drawFastVLine(127, 0, 61, TFT_NAVY); // draw line on graph
 }
 else
 { // Otherwise draw points spaced 10 pixels for the horizontal grid lines
   for (int p = 0; p <= GRAPH_MAX_VALUE; p += 10) graph1.drawPixel(127, p, TFT_NAVY);
 }

 updateCount++;
 if(updateCount >= 20) {
   updateCount = 0;
   Serial.println("read result:");
   Serial.println(SD_read()); 
 }

}

// function to split string for dividing CSV data
int split(String rcvData, char delimiter, String *dst){
 int index = 0;
 int arraySize = (sizeof(rcvData)/sizeof((rcvData)[0]));  
 int rcvDatalength = rcvData.length();
 for (int i = 0; i < rcvDatalength; i++) {
   char tmp = rcvData.charAt(i);
   if ( tmp == '\0' ) {
     dst[index] += tmp;
     break;
   }
   else if ( tmp == delimiter ) {
     index++;
     if ( index > (arraySize - 1)) return -1;
   }
   else dst[index] += tmp;
 }
 return (index + 1);
}

