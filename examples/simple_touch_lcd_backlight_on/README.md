# simple_touch_lcd_backlight_on

Example for lcd back light on/off with touch detection

## Requirement

- Arduino IDE Version: 1.8.5
- [ESPr® Developer 32](https://www.switch-science.com/catalog/3210/)
- [HiLetgo® 240X320 Resolution 2,8" SPI TFT LCD Display Touch Panel](https://www.amazon.co.jp/HiLetgo%C2%AE-LCD%E3%83%87%E3%82%A3%E3%82%B9%E3%83%97%E3%83%AC%E3%82%A4-%E3%82%BF%E3%83%83%E3%83%81%E3%83%91%E3%83%8D%E3%83%AB-SPI%E3%82%B7%E3%83%AA%E3%82%A2%E3%83%AB240-ILI9341/dp/B072N551V3)

## Pin connections

|ESP32  |LCD  |
|---|---|
|VOUT  |VCC  |
|GND  |GND  |
|IO13  |CS  |
|IO12  |RES  |
|IO14  |DC  |
|IO27  |MOSI  |
|IO26  |SCK  |
|IO17  |LED  |
|IO25  |MISO  |
|IO23  |T_IRQ  |


## Installation

1. Install Arduino libraries:
   - Adafruit_ILI9341(ver.1.5.0)　※
   - Adafruit-GFX-Library(ver.1.6.1)　 ※

1. Upload the sketch(*.ino) to ESP32 with Arduino IDE
　  

※Download as zip from github and import to Arduino IDE (open in toolbar Sketch -> Include Library -> Add .ZIP Library...)  
　  
## Use

1. Power on ESP32, "Start" will be shown .

1. A few seconds later, the LCD backlight will turn it off. 

1. After a touching display, the LCD backlight turn it on.

1. A few seconds later, the LCD backlight will turn it off. 
