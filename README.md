This is a library for using USB Host with ESP32.

## Target board
- ESP32-S3-DevKitC
- M5Stack ATOMS3
- M5Stack StampS3

## function
- USB Keyboard
- USB Mouse
- USB Serial

## Usage
```c
#include "EspUsbHostKeyboard.h"

class MyEspUsbHostKeyboard : public EspUsbHostKeyboard {
public:
  void onReceive(usb_transfer_t *transfer) {
    uint8_t *const p = transfer->data_buffer;
    Serial.printf("onKey %02x %02x %02x %02x %02x %02x %02x %02x\n", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7]);
  }
};

MyEspUsbHostKeyboard usbDev;

void setup() {
  Serial.begin(115200);
  usbDev.begin();
}

void loop() {
  usbDev.task();
}
```
