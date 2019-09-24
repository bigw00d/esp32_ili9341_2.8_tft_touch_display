# display_sd_data

display SD card data application with HSPI.

## Requirement

- Arduino Version: 1.8.5

## Note

- In SD mode, 10k pull up resistor may be required at GPIO14(SCK), GPIO15(MOSI), GPIO2(MISO), GPIO13(CS) : https://github.com/espressif/esp-idf/tree/master/examples/storage/sd_card#hardware 

- Failure to upload -> Disconnect the SD card D0/MISO line from GPIO2 : https://github.com/espressif/esp-idf/tree/master/examples/storage/sd_card#hardware 
