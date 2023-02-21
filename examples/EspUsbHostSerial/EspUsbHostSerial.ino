#include "EspUsbHostSerial.h"

EspUsbHostSerial usbDev;

void setup() {
  Serial.begin(115200);

  usbDev.setCallback_onReceive([](usb_transfer_t *transfer) {
    Serial.printf("actual_num_bytes=%d", transfer->actual_num_bytes);
    for(int i = 0; i < transfer->actual_num_bytes; i++)
      Serial.printf(" %02x", transfer->data_buffer[i]);
    Serial.println();
  });
  usbDev.begin();
}

void loop()
{
  usbDev.task();
}