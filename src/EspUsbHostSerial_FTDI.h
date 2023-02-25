#ifndef __EspUsbHostSerial_FTDI_FTDI_H__
#define __EspUsbHostSerial_FTDI_FTDI_H__

#include "EspUsbHostSerial.h"

class EspUsbHostSerial_FTDI : public EspUsbHostSerial {
public:
  void begin(uint32_t baudSpeed) {
    this->baudSpeed = baudSpeed;
    EspUsbHostSerial::begin();
  }

protected:
  uint32_t baudSpeed;
  
  void onConfig(const usb_device_desc_t *dev_desc) override {
    ESP_LOGI("EspUsbHostSerial_FTDI", "idVendor=%04x idProduct=%04x", dev_desc->idVendor, dev_desc->idProduct);
    if (Vendor && Vendor == dev_desc->idVendor && Product && Product == dev_desc->idProduct) {
      device = true;
    } else {
      if(dev_desc->idVendor == 0x27dd && dev_desc->idProduct == 0x0201) {
        // MINDEO Virtual COM Port
        InterfaceClass = 0xff;
        InterfaceSubClass = 0xff;
        InterfaceProtocol = 0xff;
      } else if(dev_desc->idVendor == 0x0403 && dev_desc->idProduct == 0x6001) {
        // FTDI USB Serial Port
        InterfaceClass = 0xff;
        InterfaceSubClass = 0xff;
        InterfaceProtocol = 0xff;
      } else if(dev_desc->idVendor == 0x10c4 && dev_desc->idProduct == 0xea60) {
        // Silicon Labs CP210x USB to UART Bridge
        InterfaceClass = 0xff;
        InterfaceSubClass = 0x00;
        InterfaceProtocol = 0x00;
      }
      device = true;
    }
  }
  
  virtual void onConfig(const usb_config_desc_t *config_desc) override {
    if(this->InterfaceNumber < 0)
      return;
    
    esp_err_t err;

    err = submit_control(0x40, 0x00, 0x0000);   // reset
    err = submit_control(0x40, 0x00, 0x0001);   // clear rx
    err = submit_control(0x40, 0x00, 0x0002);   // clear tx
    err = submit_control(0x40, 0x02, 0x0000);   // no-flow

    uint16_t baud;
    switch(this->baudSpeed)
    {
      case 300:
        baud = 0x2710;
        break;
      case 600:
        baud = 0x1388;
        break;
      case 1200:
        baud = 0x09c4;
        break;
      case 2400:
        baud = 0x04e2;
        break;
      case 4800:
        baud = 0x0271;
        break;
      case 9600:
        baud = 0x4138;
        break;
      case 19200:
        baud = 0x809c;
        break;
      case 38400:
        baud = 0xc04e;
        break;
      case 57600:
        baud = 0xc034;
        break;
      case 115200:
        baud = 0x001a;
        break;
      case 230400:
        baud = 0x000d;
        break;
      case 460800:
        baud = 0x4006;
        break;
      case 921600:
        baud = 0x8003;
        break;
      case 1000000:
        baud = 0x0003;
        break;
      case 1500000:
        baud = 0x0002;
        break;
      case 2000000:
        baud = 0x0001;
        break;
      case 3000000:
        baud = 0x0000;
        break;
      default:
        baud = 0x4138;    // 9600bps
    }
    err = submit_control(0x40, 0x03, baud);
    err = submit_control(0x40, 0x04, 0x0008);   // 8bit nonparity 1bit
   
    initialized = true;
  }
  
  void onGone(const usb_device_handle_t *dev_hdl) override {
    EspUsbHostSerial::onGone(dev_hdl);
    initialized = false;
  }

  virtual void onReceive(const uint8_t *data, const size_t length) {};
  virtual void onReceive(usb_transfer_t *transfer) {
    if(transfer->actual_num_bytes > 2)
      onReceive((uint8_t *)transfer->data_buffer + 2, transfer->actual_num_bytes - 2);
  }
  virtual bool isReady() override { return EspUsbHostSerial::isReady() && initialized; }    
private:
  bool initialized = false;
};

#endif
