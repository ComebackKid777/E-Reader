# E-Reader
E-Reader for a 2.9" Monochrome E-Ink Display connected to a Xiao ePaper Display Board

I used the following from SeeedStudio:
- 2.9" Monochrome eInk / ePaper Display with 296x128 Pixels
- XIAO ePaper Display Board(ESP32-S3) - EE04

NB: Each line of your book.txt is 1 page.

# How to compile and use
- This program uses https://github.com/Seeed-Studio/Seeed_GFX
- Download the latest release ofthe Seeed_GFX library
- In Arduino Studio import the .zip as a library and include it in the project
- This library is based off of TFT_eSPI so if you already have this library you will have to remove it
- Make sure driver.h has the correct definitions for your display and board combo if using different from above mentioned (Ref: https://seeed-studio.github.io/Seeed_GFX/)
- Use the following Arduino plugin for uploading your book.txt to your ESP32: https://github.com/earlephilhower/arduino-littlefs-upload
- Make sure your board in Arduino Studio has the following: PSRAM set to "OPI PSRAM" and Partition Scheme set to "Default with Spiffs
- If you want to use a different bmp image for your sleep screen you must convert your bmp into an byte array and copy it into sleep.h (Ref: https://wiki.seeedstudio.com/XIAO-eInk-Expansion-Board/#image-extraction-software)

# Useful links:
- https://wiki.seeedstudio.com/epaper_ee04/
