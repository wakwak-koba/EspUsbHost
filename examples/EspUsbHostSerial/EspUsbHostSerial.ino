#include "EspUsbHostSerial.h"

EspUsbHostSerial usbDev;

void setup() {
  Serial.begin(115200);

  usbDev.setCallback_onReceive([](const uint8_t *data, const size_t length) {
    Serial.printf("length=%d", length);
    for(int i = 0; i < length; i++)
      Serial.printf(" %02x", data[i]);
    Serial.println();
  });
  usbDev.begin();
}

void loop()
{
  usbDev.task();
}