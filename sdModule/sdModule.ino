/*

   SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS - pin 4 (for MKRZero SD: SDCARD_SS_PIN)

  created   Nov 2010
  by David A. Mellis
  modified 9 Apr 2012
  by Tom Igoe

*/

#include <stdio.h>
#include <SPI.h>
#include <SD.h>

File bajaData;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect
  }



  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1);
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  bajaData = SD.open("test.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (bajaData) {
    bajaData.println("Data package test: Package contents will appear here");
    // close the file:
    bajaData.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("Error opening the test text file");
  }

  // re-open the file for reading:
  bajaData = SD.open("test.txt");
  if (bajaData) {
    Serial.println("test.txt:");

    // read from the file until there's nothing else in it:
    while (bajaData.available()) {
      Serial.write(bajaData.read());
    }
    // close the file:
    bajaData.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("Error opening the test text file");
  }
}

void loop() {
  // nothing happens after setup
}


