#include "EspUsbHostSerial.h"

class SerialBarcodeScanner : public EspUsbHostSerial {
  String value;
  
  virtual void onNew(const usb_device_info_t *dev_info, const usb_device_desc_t *dev_desc) {
    Serial.println("connected");
  }
  virtual void onReceive(usb_transfer_t *transfer) {
    for(int i = 0; i < transfer->actual_num_bytes; i++) {
      if(transfer->data_buffer[i] >= 0x20)
        value += (char)transfer->data_buffer[i];
      else if (value.length()) {
        Serial.println(value);
        value.clear();
      }
    }
  }
  virtual void onGone() {
    Serial.println("disconnected");
  }
};

SerialBarcodeScanner usbDev;

void setup() {
  Serial.begin(115200);
  usbDev.begin();
}

void loop()
{
  usbDev.task();
}