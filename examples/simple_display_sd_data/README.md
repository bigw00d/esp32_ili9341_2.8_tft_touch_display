# simple_display_sd_data

Display SD file contents application with HSPI.(simple version)

## Requirement

- Arduino Version: 1.8.5

## Hardware Connections

### LCD

|ESP32  |LCD  |
|---|---|
|3V3  |3V3  |
|GND  |GND  |
|IO5  |CS  |
|IO16  |RES  |
|IO17  |DC  |
|IO23  |MOSI  |
|IO18  |SCK  |
|3V3  |LED  |
|IO19  |MISO  |

### SD

|ESP32  |SD  |
|---|---|
|IO14  |SCK  |
|IO2  |MISO  |
|IO15  |MOSI  |
|IO13  |SS  |

## Note

- In SD mode, 10k pull up resistor may be required at GPIO14(SCK), GPIO15(MOSI), GPIO2(MISO), GPIO13(CS) : https://github.com/espressif/esp-idf/tree/master/examples/storage/sd_card#hardware 

- Failure to upload -> Disconnect the SD card D0/MISO line from GPIO2 : https://github.com/espressif/esp-idf/tree/master/examples/storage/sd_card#hardware 

- In battery use, flash frequency 40Mhz is better : http://wakwak-koba.hatenadiary.jp/entry/20170228/p1
