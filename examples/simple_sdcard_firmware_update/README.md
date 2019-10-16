# simple_sdcard_firmware_update

Firmware update with SD card.

## Requirement

- Arduino Version: 1.8.5

## Hardware Connections

### SD

|ESP32  |SD  |
|---|---|
|IO14  |SCK  |
|IO2  |MISO  |
|IO15  |MOSI  |
|IO13  |SS  |
|ESP32  |SD  |備考  |

※insert 10k pull-up（[article](https://github.com/espressif/esp-idf/tree/master/examples/storage/sd_card)）



## Usage

1. Upload & Create firmware

1. Rename firmware(***.bin -> update.bin)

1. Input update.bin to SD card & insert to SD slot

1. Disconnect USB cable & disconnect IO2 pull-up

1. Connect USB Cable & upload scketch  

1. Disconnect USB cable & 'connect' IO2 pull-up

1. Connect USB Cable  

1. Start firm update & reboot(& remove firmware in SD card)  

1. Start new firmware
