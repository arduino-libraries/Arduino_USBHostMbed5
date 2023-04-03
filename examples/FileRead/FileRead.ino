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

#include <Arduino_USBHostMbed5.h>
#include <DigitalOut.h>
#include <FATFileSystem.h>

USBHostMSD msd;
mbed::FATFileSystem usb("usb");

// If you are using a Portenta Machine Control uncomment the following line
mbed::DigitalOut otg(PB_14, 0);
 
void setup() {
  Serial.begin(115200);
  while (!Serial);

  delay(2500);
  Serial.println("Starting USB File Read example...");

  // if you are using a Max Carrier uncomment the following line
  //start_hub();

  while (!msd.connect()) {
    delay(1000);
  }

  Serial.println("Mounting USB device...");
  int err =  usb.mount(&msd);
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
  FILE *f = fopen("/usb/Arduino.txt", "r+");
  char buf[256];
  Serial.println("File content:");

  while (fgets(buf, 256, f) != NULL) {
    Serial.print(buf);
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
    delay(1000);
    // handle disconnection and reconnection
    if (!msd.connected()) {
        msd.connect();
    }
}
