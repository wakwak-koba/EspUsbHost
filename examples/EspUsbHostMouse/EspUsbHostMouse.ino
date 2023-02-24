#include "EspUsbHostMouse.h"

class MyEspUsbHostMouse : public EspUsbHostMouse {
public:
  void onMouse(const uint8_t *data, const size_t length) override {
    Serial.printf("length=%d", length);
    for(int i = 0; i < length; i++)
      Serial.printf(" %02x", data[i]);
    Serial.println();
  }
  virtual void onNew() override {
    Serial.println("connected");
    Serial.println(("Manufacturer:" + getManufacturer()).c_str());
    Serial.println(("Product:" + getProduct()).c_str());
  }
  virtual void onGone() override {
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
