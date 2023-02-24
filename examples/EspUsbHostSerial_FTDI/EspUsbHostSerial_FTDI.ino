#include "EspUsbHostSerial_FTDI.h"

class SerialFTDI : public EspUsbHostSerial_FTDI {
  String value;
  
  void onNew() override {
    Serial.println(("Manufacturer:" + getManufacturer()).c_str());
    Serial.println(("Product:" + getProduct()).c_str());
  }
  void onReceive(const uint8_t *data, const size_t length) override {
    for(int i = 0; i < length; i++) {
      if(data[i] >= 0x20)
        value += (char)data[i];
      else if (value.length()) {
        Serial.println(value);
        value.clear();
      }
    }
  }
  void onGone() override {
    Serial.println("disconnected");
  }
};

SerialFTDI usbDev;

void setup() {
  Serial.begin(115200);
  usbDev.begin(115200);
}

void loop()
{
  usbDev.task();
}