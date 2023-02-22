#include "EspUsbHostMouse.h"

class MyEspUsbHostMouse : public EspUsbHostMouse {
public:
  void onReceive(usb_transfer_t *transfer) {
    Serial.printf("actual_num_bytes=%d", transfer->actual_num_bytes);
    for(int i = 0; i < transfer->actual_num_bytes; i++)
      Serial.printf(" %02x", transfer->data_buffer[i]);
    Serial.println();
  }
  virtual void onNew(const usb_device_info_t *dev_info, const usb_device_desc_t *dev_desc) {
    Serial.println("connected");
  }
  virtual void onGone() {
    Serial.println("disconnected");
  }
};

MyEspUsbHostMouse usbDev;

void setup() {
  Serial.begin(115200);
  usbDev.begin();
}

void loop() {
  usbDev.task();
}
