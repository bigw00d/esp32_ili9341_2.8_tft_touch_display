# simple_ble_echo_back

Simple SD card read application with HSPI.

## Requirement

- Arduino Version: 1.8.5


## Note

- 1k pull up resistor may be required at SD_MISO ( https://www.amazon.co.jp/HiLetgo%C2%AE-LCD%E3%83%87%E3%82%A3%E3%82%B9%E3%83%97%E3%83%AC%E3%82%A4-%E3%82%BF%E3%83%83%E3%83%81%E3%83%91%E3%83%8D%E3%83%AB-SPI%E3%82%B7%E3%83%AA%E3%82%A2%E3%83%AB240-ILI9341/product-reviews/B072N551V3 ) 

- In SD mode, 10k pull up resistor may be required at GPIO14(SCK), GPIO15(MOSI), GPIO2(MISO), GPIO13(CS).( https://github.com/espressif/esp-idf/tree/master/examples/storage/sd_card#hardware )

- Failure to upload -> Disconnect the SD card D0/MISO line from GPIO2. ( https://github.com/espressif/esp-idf/tree/master/examples/storage/sd_card#hardware )
