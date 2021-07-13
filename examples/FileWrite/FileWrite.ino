#include "USBHostMbed5.h"
#include "USBHostMSD/USBHostMSD.h"
#include "USBHostHub/USBHostHub.h"
#include "USBHostSerial/USBHostSerial.h"
#include "DigitalOut.h"
#include "FATFileSystem.h"

USBHostMSD msd;
mbed::FATFileSystem fs("fs");

mbed::DigitalOut pin5(PC_6, 0);
mbed::DigitalOut otg(PB_8, 1);

void setup() {
  Serial.begin(115200);
  while (!Serial);
  msd.connect();

  while (!msd.connected()) {
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
  Serial.println("Open /fs/numbers.txt");
  FILE *f = fopen("/fs/numbers.txt", "w+");
  for (int i = 0; i < 10; i++) {
    Serial.print("Writing numbers (");
    Serial.print(i);
    Serial.println("/10)");
    fflush(stdout);
    err = fprintf(f, "%d\n", i);
    if (err < 0) {
      Serial.println("Fail :(");
      error("error: %s (%d)\n", strerror(errno), -errno);
    }
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
