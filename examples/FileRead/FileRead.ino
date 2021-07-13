/*
  Portenta - FileRead

  The sketch shows how to mount an usb storage device and how to
  read from an existing file.
  to use this sketch create a .txt file named Arduino.txt,
  in your storage device and write some content inside.
  
  The circuit:
   - Portenta H7
    
  This example code is in the public domain.
*/
#include "USBHostMbed5.h"
#include "USBHostMSD/USBHostMSD.h"
#include "USBHostHub/USBHostHub.h"
#include "USBHostSerial/USBHostSerial.h"
#include "DigitalOut.h"
#include "FATFileSystem.h"

USBHostMSD msd;
mbed::FATFileSystem fs("fs");

mbed::DigitalOut pin5(PC_6, 0);

// If you are using a Portenta Machine Control uncomment the following line
//mbed::DigitalOut otg(PB_8, 1);
 
#define BUFFER_MAX_LEN 64
void setup() {
  Serial.begin(115200);
  while (!Serial);

  // if you are using a Max Carrier uncomment the following line
  //start_hub();

  while (!msd.connect()) {
    //while (!port.connected()) {
    delay(1000);
  }

  Serial.println("Mounting USB device...");
  int err =  fs.mount(&msd);
  if (err) {
    Serial.print("Error mounting USB device ");
    Serial.println(err);
    while (1);
  }
  Serial.print("read done ");
  mbed::fs_file_t file;
  struct dirent *ent;
  int dirIndex = 0;
  int res = 0;
  Serial.println("Open file..");
  FILE *f = fopen("/fs/Arduino.txt", "r+");
  char buf[2];
  Serial.println("File conntet:");

  while (fgets(buf, 2, f) != NULL) {
    Serial.println(buf);
  }

  Serial.println("File closing");
  fflush(stdout);
  err = fclose(f);
  if (err < 0) {
    Serial.print("fclose error:");
    Serial.print(strerror(errno));
    Serial.print(" (");
    Serial.print(-errno);
    Serial.print(")");
  } else {
    Serial.println("File closed");
  }
}

void loop() {

}