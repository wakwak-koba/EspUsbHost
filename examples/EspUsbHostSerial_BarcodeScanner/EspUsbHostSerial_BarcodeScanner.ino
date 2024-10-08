#include "EspUsbHostSerial.h"

class SerialBarcodeScanner : public EspUsbHostSerial {
  String value;
  
  virtual void onNew() override {
    Serial.println(("Manufacturer:" + getManufacturer()).c_str());
    Serial.println(("Product:" + getProduct()).c_str());
  }
  virtual void onReceive(const uint8_t *data, const size_t length) override {
    for(int i = 0; i < length; i++) {
      if(data[i] >= 0x20)
        value += (char)data[i];
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