// Adafruit SPI Flash FatFs Format Example
// Author: Tony DiCola
//
// This will partition and format the SPI flash to have a new
// blank FAT filesystem.  This is useful for resetting the flash
// filesystem and erasing all the old data.  This is also a good
// example of using the low-level FatFs library functions directly
// instead of wrappers from this library.  See documentation on
// FatFs functions here:
//   http://elm-chan.org/fsw/ff/00index_e.html
//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// !!  NOTE: YOU WILL ERASE ALL DATA BY RUNNING THIS SKETCH!  !!
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//
// Usage:
// - Modify the pins and type of fatfs object in the config
//   section below if necessary (usually not necessary).
// - Upload this sketch to your M0 express board.
// - Open the serial monitor at 115200 baud.  You should see a
//   prompt to confirm formatting.  If you don't see the prompt
//   close the serial monitor, press the board reset buttton,
//   wait a few seconds, then open the serial monitor again.
// - Type OK and enter to confirm the format when prompted.
// - Partitioning and formatting will take about 30-60 seconds.
//   Once formatted a message will be printed to notify you that
//   it is finished.
//
/**
 * @file main.cpp
 * @author Medad Rufus Newman (medad@medadnewman.co.uk)
 * @brief 
 * @version 0.1
 * @date 2021-10-04
 * 
 * @copyright Copyright (c) 2021
 * 
 * Run this program to wipe out the flash chip of the flight computer. WARNING! ALL DATA WILL BE WIPED OUT
 * 
 */
#include <SPI.h>
#include <Adafruit_SPIFlash.h>
#include <Adafruit_SPIFlash_FatFs.h>
#include <Arduino.h>

// Include the FatFs library header to use its low level functions
// directly.  Specifically the f_fdisk and f_mkfs functions are used
// to partition and create the filesystem.
#include "utility/ff.h"

// Configuration of the flash chip pins and flash fatfs object.
// You don't normally need to change these if using a Feather/Metro
// M0 express board.
#define FLASH_TYPE SPIFLASHTYPE_W25Q64 // Flash chip type.         \
                                       // If you change this be    \
                                       // sure to change the fatfs \
                                       // object type below to match.

#if defined(__SAMD51__)
// Alternatively you can define and use non-SPI pins, QSPI isnt on a sercom
Adafruit_SPIFlash flash(PIN_QSPI_SCK, PIN_QSPI_IO1, PIN_QSPI_IO0, PIN_QSPI_CS);
#else
#if (SPI_INTERFACES_COUNT == 1)
#define FLASH_SS SS        // Flash chip SS pin.
#define FLASH_SPI_PORT SPI // What SPI port is Flash on?
#else
#define FLASH_SS SS1        // Flash chip SS pin.
#define FLASH_SPI_PORT SPI1 // What SPI port is Flash on?
#endif

Adafruit_SPIFlash flash(FLASH_SS, &FLASH_SPI_PORT); // Use hardware SPI
#endif

Adafruit_W25Q16BV_FatFs fatfs(flash);

void setup()
{
  // Initialize serial port and wait for it to open before continuing.
  Serial.begin(115200);
  while (!Serial)
  {
    delay(100);
  }
  Serial.println("Adafruit SPI Flash FatFs Format Example");

  // Initialize flash library and check its chip ID.
  if (!flash.begin(FLASH_TYPE))
  {
    Serial.println("Error, failed to initialize flash chip!");
    while (1)
      ;
  }
  Serial.print("Flash chip JEDEC ID: 0x");
  Serial.println(flash.GetJEDECID(), HEX);

  // Wait for user to send OK to continue.
  Serial.setTimeout(30000); // Increase timeout to print message less frequently.
  do
  {
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
    Serial.println("This sketch will ERASE ALL DATA on the flash chip and format it with a new filesystem!");
    Serial.println("Type OK (all caps) and press enter to continue.");
    Serial.println("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!");
  } while (!Serial.find("OK"));

  // Call fatfs activate to make it the active chip that receives low level fatfs
  // callbacks.  This is necessary before making any manual fatfs function calls
  // (like the f_fdisk and f_mkfs functions further below).  Be sure to call
  // activate before you call any fatfs functions yourself!
  fatfs.activate();

  // Partition the flash with 1 partition that takes the entire space.
  Serial.println("Partitioning flash with 1 primary partition...");
  DWORD plist[] = {100, 0, 0, 0}; // 1 primary partition with 100% of space.
  uint8_t buf[512] = {0};         // Working buffer for f_fdisk function.
  FRESULT r = f_fdisk(0, plist, buf);
  if (r != FR_OK)
  {
    Serial.print("Error, f_fdisk failed with error code: ");
    Serial.println(r, DEC);
    while (1)
      ;
  }
  Serial.println("Partitioned flash!");

  // Make filesystem.
  Serial.println("Creating and formatting FAT filesystem (this takes ~60 seconds)...");
  r = f_mkfs("", FM_ANY, 0, buf, sizeof(buf));
  if (r != FR_OK)
  {
    Serial.print("Error, f_mkfs failed with error code: ");
    Serial.println(r, DEC);
    while (1)
      ;
  }
  Serial.println("Formatted flash!");

  // Finally test that the filesystem can be mounted.
  if (!fatfs.begin())
  {
    Serial.println("Error, failed to mount newly formatted filesystem!");
    while (1)
      ;
  }

  // Done!
  Serial.println("Flash chip successfully formatted with new empty filesystem!");
}

void loop()
{
  // Nothing to be done in the main loop.
  delay(100);
}
