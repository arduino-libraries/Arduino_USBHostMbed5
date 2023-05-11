/*
  Opta - DirList

  The sketch shows how to mount an usb storage device and how to
  get a list of the existing folders and files.
  It will print debugging output to the RS485 port.

  The circuit:
   - Arduino Opta RS485 or WiFi

  This example code is in the public domain.
*/

#include <Arduino_USBHostMbed5.h>
#include <DigitalOut.h>
#include <FATFileSystem.h>

USBHostMSD msd;
mbed::FATFileSystem usb("usb");

#include <ArduinoRS485.h>
constexpr auto baudrate { 115200 };

void setup()
{
    configureRS485(baudrate);
    printlnRS485("Starting USB Dir List example...");


    printRS485("Please, connect the USB device...");
    while (!msd.connect()) {
        printRS485(".");
        delay(1000);
    }

    char buf[256] {};

    printlnRS485("");
    printRS485("Mounting USB device... ");
    int err = usb.mount(&msd);

    if (err) {
        printRS485("Error mounting USB device ");
        snprintf(buf, sizeof(buf), "error: %s (%d)\r\n", -err == EINVAL ? "Invalid Filesystem" : "Unknown Error", -err);
        printlnRS485(buf);
        while (1)
            ;
    }
    printlnRS485("done.");

    // Display the root directory
    printRS485("Opening the root directory... ");
    DIR* d = opendir("/usb/");
    if (!d) {
        snprintf(buf, sizeof(buf), "error: %s (%d)", strerror(errno), -errno);
        printRS485(buf);
    }
    printlnRS485("done.");

    printlnRS485("Root directory:");
    unsigned int count { 0 };
    while (true) {
        struct dirent* e = readdir(d);
        if (!e) {
            break;
        }
        count++;
        snprintf(buf, sizeof(buf), "    %s\r\n", e->d_name);
        printRS485(buf);
    }
    printRS485(count);
    printlnRS485(" files found!");

    snprintf(buf, sizeof(buf), "Closing the root directory... ");
    printRS485(buf);
    fflush(stdout);
    err = closedir(d);
    snprintf(buf, sizeof(buf), "%s\r\n", (err < 0 ? "Fail :(" : "OK"));
    printRS485(buf);
    if (err < 0) {
        snprintf(buf, sizeof(buf), "error: %s (%d)\r\n", strerror(errno), -errno);
        printRS485(buf);
    }
}

void loop()
{
    delay(1000);
    // handle disconnection and reconnection
    if (!msd.connected()) {
        msd.connect();
    }
}

size_t printRS485(char* message)
{
    RS485.beginTransmission();
    auto len = strlen(message);
    RS485.write(message, len);
    RS485.endTransmission();
}

size_t printRS485(int num)
{
    char message[256] {};
    sprintf(message, "%d", num);
    RS485.beginTransmission();
    auto len = strlen(message);
    RS485.write(message, len);
    RS485.endTransmission();
}

size_t printlnRS485(char* message)
{
    printRS485(message);
    RS485.beginTransmission();
    RS485.write('\r');
    RS485.write('\n');
    RS485.endTransmission();
}

void configureRS485(const int baudrate)
{
    const auto bitduration { 1.f / baudrate };
    const auto wordlen { 9.6f }; // OR 10.0f depending on the channel configuration
    const auto preDelayBR { bitduration * wordlen * 3.5f * 1e6 };
    const auto postDelayBR { bitduration * wordlen * 3.5f * 1e6 };

    RS485.begin(baudrate);
    RS485.setDelays(preDelayBR, postDelayBR);
    RS485.noReceive();
}
