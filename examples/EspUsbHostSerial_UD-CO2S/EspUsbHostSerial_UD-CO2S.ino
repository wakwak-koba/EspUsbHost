#include "EspUsbHostSerial.h"

class UDCO2S : public EspUsbHostSerial {
public:
  String value;
  uint32_t moment = 0;
  int co2 = 0;
  float hum = 0.0F;
  float tmp = 0.0F;

  UDCO2S() : EspUsbHostSerial(0x04d8, 0xa95a) {};

  String toString() {
    char bufs[30];
    sprintf(bufs, "CO2=%d,HUM=%.1f,TMP=%.1f", co2, hum, tmp);
    return String(bufs);
  }

  virtual void onNew() override {
    Serial.println(("Manufacturer:" + getManufacturer()).c_str());
    Serial.println(("Product:" + getProduct()).c_str());
    submit((uint8_t *)"STA\r", 4);
  }
  virtual void onReceive(const uint8_t *data, const size_t length) override {
    for(int i = 0; i < length; i++) {
      if(data[i] >= 0x20)
        value += (char)data[i];
      else if (value.length()) {
        if(sscanf(value.c_str(), "CO2=%d,HUM=%f,TMP=%f", &co2, &hum, &tmp) == 3) {
          moment = millis();
          // Need correction ?
          // tmp -= 4.5F;
          // hum = (int)(10.0F * hum * 4.0F / 3.0F + 0.5F) / 10.0F;
          Serial.println(toString());
        }
        value.clear();
      }
    }
  }
  virtual void onGone() {
    Serial.println("disconnected");
  }
};

UDCO2S usbDev;

void setup() {
  Serial.begin(115200);
  usbDev.begin();
}

void loop()
{
  usbDev.task();
}