/*
  Portenta - FileWrite

  The sketch shows how to mount an usb storage device and how to
  write a file, eventually overwriting the original content.

  The circuit:
   - Portenta H7

  This example code is in the public domain.
*/

#include <USBHostMbed5.h>
#include <Wire.h>
#include <usb_phy_api.h>
#include <DigitalOut.h>
#include <FATFileSystem.h>

USBHostMSD msd;
mbed::FATFileSystem usb("usb");

#define Serial Serial1

void turnOnLed() {
  digitalWrite(LED_BUILTIN, HIGH);
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

#if 0
  Wire1.begin();

  Wire1.beginTransmission(0x21);
  Wire1.write(0x5);
  Wire1.write(0x1);
  Wire1.endTransmission();

  Wire1.beginTransmission(0x21);
  Wire1.write(0x5);
  Wire1.write(0x0);
  Wire1.endTransmission();

  delay(100);

  Wire1.beginTransmission(0x21);
  Wire1.write(0x2);
  Wire1.write(0x3);
  Wire1.endTransmission();

  // enable interrupts
  Wire1.beginTransmission(0x21);
  Wire1.write(0x3);
  Wire1.write(1 << 1);
  Wire1.endTransmission();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  attachInterrupt(PD_8, turnOnLed, FALLING);
#endif

  while (!msd.connect()) {
    //while (!port.connected()) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
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
  Serial.println("Open /usb/numbers.txt");
  FILE *f = fopen("/usb/numbers.txt", "w+");
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
  delay(1000);
  if (!msd.connected()) {
    msd.connect();
  }
}