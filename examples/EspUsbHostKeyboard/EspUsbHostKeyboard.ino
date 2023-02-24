#include "EspUsbHostKeyboard.h"

class MyEspUsbHostKeyboard : public EspUsbHostKeyboard {
public:
  void onNew() override {
    Serial.println("connected");
    Serial.println(("Manufacturer:" + getManufacturer()).c_str());
    Serial.println(("Product:" + getProduct()).c_str());
    Serial.print("ReportDescriptor:");
    for(int i = 0; i < this->reportDescriptorSize; i++) {
      if(!(i % 16))
        Serial.println();
      Serial.printf(" %02x", this->reportDescriptor[i]);
    }
    Serial.println();
  }
  void onKey(const uint8_t *data, const size_t length) override {
    for(int i = 0; i < length; i++)
      Serial.printf(" %02x", data[i]);
    Serial.println();
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
