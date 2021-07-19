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
#include <DigitalOut.h>
#include <FATFileSystem.h>
#include <USBHostMbed5.h>

USBHostMSD msd;
mbed::FATFileSystem usb("usb");

mbed::DigitalOut pin5(PC_6, 0);

// If you are using a Portenta Machine Control uncomment the following line
mbed::DigitalOut otg(PB_14, 0);

#define BUFFER_MAX_LEN 64
void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;

    delay(2500);
    Serial.println("Starting USB Dir List example...");

    // if you are using a Max Carrier uncomment the following line
    //start_hub();

    while (!msd.connect()) {
        //while (!port.connected()) {
        delay(1000);
    }

    Serial.print("Mounting USB device... ");
    int err = usb.mount(&msd);
    if (err) {
        Serial.print("Error mounting USB device ");
        Serial.println(err);
        while (1)
            ;
    }
    Serial.println("done.");

    char buf[256] {};
    // Display the root directory
    Serial.print("Opening the root directory... ");
    DIR* d = opendir("/usb/");
    Serial.println(!d ? "Fail :(" : "Done");
    if (!d) {
        snprintf(buf, sizeof(buf), "error: %s (%d)\r\n", strerror(errno), -errno);
        Serial.print(buf);
    }
    Serial.println("done.");

    Serial.println("Root directory:");
    unsigned int count { 0 };
    while (true) {
        struct dirent* e = readdir(d);
        if (!e) {
            break;
        }
        count++;
        snprintf(buf, sizeof(buf), "    %s\r\n", e->d_name);
        Serial.print(buf);
    }
    Serial.print(count);
    Serial.println(" files found!");

    snprintf(buf, sizeof(buf), "Closing the root directory... ");
    Serial.print(buf);
    fflush(stdout);
    err = closedir(d);
    snprintf(buf, sizeof(buf), "%s\r\n", (err < 0 ? "Fail :(" : "OK"));
    Serial.print(buf);
    if (err < 0) {
        snprintf(buf, sizeof(buf), "error: %s (%d)\r\n", strerror(errno), -errno);
        Serial.print(buf);
    }
}

void loop()
{
}
