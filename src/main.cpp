#include <Arduino.h>
#include <MFRC522.h>

#include <Wire.h>

#define SAD 5   // SPI chip select pin (CS/SS/SSEL)
#define RST 27  // Not reset and power-down pin.

MFRC522 nfc(SAD, RST);

byte allowedCards[2][5] = {
    {0xC0, 0x51, 0xAC, 0x22, 0x1F},
    {0xC0, 0x51, 0xAC, 0x22, 0x1E}
};

void setup() {
  SPI.begin();
  // Read a fast as possible. There is a limit for how long we are
  // allowed to read from the tags.
  Serial.begin(115200);

  Serial.println("Looking for MFRC522.");
  nfc.begin();

  // Get the firmware version of the RFID chip
  byte version = nfc.getFirmwareVersion();
  if (! version) {
    Serial.print("Didn't find MFRC522 board.");
    while(1); //halt
  }

  Serial.print("Found chip MFRC522 ");
  Serial.print("Firmware ver. 0x");
  Serial.print(version, HEX);
  Serial.println(".");

}

void loop() {
  byte status;
  byte data[MAX_LEN];
  byte serial[5];
  int i, j, pos;

  // Send a general request out into the aether. If there is a tag in
  // the area it will respond and the status will be MI_OK.
  status = nfc.requestTag(MF1_REQIDL, data);

  if (status == MI_OK) {

    // calculate the anti-collision value for the currently detected
    // tag and write the serial into the data array.
    status = nfc.antiCollision(data);
    memcpy(serial, data, 5);

    Serial.println("The serial nb of the tag is:");
    for (i = 0; i < 4; i++) {
      Serial.print(serial[i], HEX);
      Serial.print(", ");
    }
    Serial.println(serial[4], HEX);

    int rows = sizeof allowedCards / sizeof allowedCards[0];
    for (int allowedNumber = 0; allowedNumber < rows; allowedNumber++) {
      Serial.print(allowedNumber, HEX);

      Serial.print(" - The serial tag is allowed : ");
      boolean allow = true;
      for (i = 0; i < 4; i++) {
        Serial.print(allowedCards[allowedNumber][i], HEX);
        Serial.print(", ");
        if (allowedCards[allowedNumber][i] != serial[i]) allow = false;
      }
      Serial.print(allowedCards[allowedNumber][4], HEX);
      if (allowedCards[allowedNumber][4] != serial[4]) allow = false;

      if (allow) {
        Serial.println(" - allowed");
      } else {
        Serial.println(" - not allowed");
      }
    }

    // Stop the tag and get ready for reading a new tag.
    nfc.haltTag();
  }
  delay(2000);
}
