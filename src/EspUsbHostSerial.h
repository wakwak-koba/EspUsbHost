#ifndef __EspUsbHostSerial_H__
#define __EspUsbHostSerial_H__

#include "EspUsbHost.h"

class EspUsbHostSerial : public EspUsbHost {
public:
  EspUsbHostSerial() {};
  EspUsbHostSerial(uint16_t Vendor, uint16_t Product) : Vendor(Vendor), Product(Product) {;}
  EspUsbHostSerial(uint8_t InterfaceClass, uint8_t InterfaceSubClass, uint8_t InterfaceProtocol) : InterfaceClass(InterfaceClass), InterfaceSubClass(InterfaceSubClass), InterfaceProtocol(InterfaceProtocol) {;}
  
  virtual void task(void) override {
    EspUsbHost::task();
    if (this->isReady()) {
      unsigned long now = millis();
      if ((now - this->lastCheck) > this->interval) {
        this->lastCheck = now;
        esp_err_t err = usb_host_transfer_submit(this->usbTransfer_recv);
        if (err != ESP_OK && err != ESP_ERR_NOT_FINISHED && err != ESP_ERR_INVALID_STATE) {
          ESP_LOGI("EspUsbHostSerial", "usb_host_transfer_submit() err=%x", err);
        }
      }
    }
  }
  
  virtual void submit(const uint8_t *data, const uint8_t length) {
    if(length > 64)
      return;
    
    this->usbTransfer_send->num_bytes = length;
    memcpy(this->usbTransfer_send->data_buffer, data, this->usbTransfer_send->num_bytes);
    esp_err_t err = usb_host_transfer_submit(this->usbTransfer_send);
      if (err != ESP_OK) {
        ESP_LOGI("EspUsbHostSerial", "usb_host_transfer_submit() err=%x", err);
      }
    
  }
  
  virtual void setCallback_onReceive(void (*callback)(const uint8_t *data, const size_t length)) {onReceiveCB = callback;}

protected:
  uint8_t interval;
  unsigned long lastCheck;
  usb_transfer_t *usbTransfer_recv = nullptr;
  usb_transfer_t *usbTransfer_send = nullptr;
  bool device = false;
  int16_t InterfaceNumber = -1;
  
  uint16_t Vendor = 0x0000;
  uint16_t Product = 0x0000;
  uint8_t InterfaceClass = USB_CLASS_CDC_DATA;
  uint8_t InterfaceSubClass = 0x00;
  uint8_t InterfaceProtocol = 0x00;
  
  void onConfig(const usb_device_desc_t *dev_desc) override {
    ESP_LOGI("EspUsbHostSerial", "idVendor=%04x idProduct=%04x", dev_desc->idVendor, dev_desc->idProduct);
    if (Vendor && Vendor == dev_desc->idVendor && Product && Product == dev_desc->idProduct) {
      device = true;
    } else {
      if(dev_desc->idVendor == 0x27dd && dev_desc->idProduct == 0x0201) {
        // MINDEO Virtual COM Port
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
  
  void onConfig(const uint8_t bDescriptorType, const uint8_t *p) override {
    if(!device)
      return;

    switch (bDescriptorType) {
      case USB_B_DESCRIPTOR_TYPE_INTERFACE:
        {
          const usb_intf_desc_t *intf = (const usb_intf_desc_t *)p;
          ESP_LOGI("EspUsbHostSerial", "bDescriptorType=%d bInterfaceClass=%02x bInterfaceSubClass=%02x bInterfaceProtocol=%02x", bDescriptorType, intf->bInterfaceClass, intf->bInterfaceSubClass, intf->bInterfaceProtocol);
          
          if (intf->bInterfaceClass == USB_CLASS_COMM && intf->bInterfaceSubClass == 0x02 && intf->bInterfaceProtocol == 0x01) {
            esp_err_t err = usb_host_interface_claim(this->clientHandle, this->deviceHandle, intf->bInterfaceNumber, intf->bAlternateSetting);
            if (err != ESP_OK) {
              ESP_LOGI("EspUsbHostSerial", "usb_host_interface_claim() err=%x", err);
            }
          }

          if (intf->bInterfaceClass == InterfaceClass && intf->bInterfaceSubClass == InterfaceSubClass && intf->bInterfaceProtocol == InterfaceProtocol) {
            esp_err_t err = usb_host_interface_claim(this->clientHandle, this->deviceHandle, intf->bInterfaceNumber, intf->bAlternateSetting);
            if (err != ESP_OK) {
              ESP_LOGI("EspUsbHostSerial", "usb_host_interface_claim() err=%x", err);
            } else {
              this->InterfaceNumber = intf->bInterfaceNumber;
              ESP_LOGI("EspUsbHostSerial", "bInterfaceNumber=%d", intf->bInterfaceNumber);
            }
          }
        }
        break;

      case USB_B_DESCRIPTOR_TYPE_ENDPOINT:
        {

          const usb_ep_desc_t *endpoint = (const usb_ep_desc_t *)p;
          ESP_LOGI("EspUsbHostSerial", "bDescriptorType=%d bEndpointAddress=%02x bmAttributes=%02x wMaxPacketSize=%d", bDescriptorType, endpoint->bEndpointAddress, endpoint->bmAttributes, endpoint->wMaxPacketSize);

          esp_err_t err;
          if (this->InterfaceNumber >= 0 && !this->usbTransfer_recv && ((endpoint->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) == USB_BM_ATTRIBUTES_XFER_BULK) && (endpoint->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK)) {
            err = usb_host_transfer_alloc(endpoint->wMaxPacketSize, 0, &this->usbTransfer_recv);
            if (err != ESP_OK) {
              this->usbTransfer_recv = NULL;
              ESP_LOGI("EspUsbHostSerial", "usb_host_transfer_alloc() err=%x", err);
              return;
            }

            this->usbTransfer_recv->device_handle = this->deviceHandle;
            this->usbTransfer_recv->bEndpointAddress = endpoint->bEndpointAddress;
            this->usbTransfer_recv->callback = this->_onReceive;
            this->usbTransfer_recv->context = this;
            this->usbTransfer_recv->num_bytes = endpoint->wMaxPacketSize;
            ESP_LOGI("EspUsbHostSerial", "usbTransfer_recv");
          }

          if (this->InterfaceNumber >= 0 && !this->usbTransfer_send && ((endpoint->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) == USB_BM_ATTRIBUTES_XFER_BULK) && !(endpoint->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK)) {
            err = usb_host_transfer_alloc(endpoint->wMaxPacketSize, 0, &this->usbTransfer_send);
            if (err != ESP_OK) {
              this->usbTransfer_send = NULL;
              ESP_LOGI("EspUsbHostSerial", "usb_host_transfer_alloc() err=%x", err);
              return;
            }

            this->usbTransfer_send->device_handle = this->deviceHandle;
            this->usbTransfer_send->bEndpointAddress = endpoint->bEndpointAddress;
            this->usbTransfer_send->callback = this->_onSend;
            this->usbTransfer_send->context = this;
             ESP_LOGI("EspUsbHostSerial", "usbTransfer_send");
         }
        }
        break;

      default:
        ESP_LOGI("EspUsbHostSerial", "bDescriptorType=%d", bDescriptorType);
        break;
    }
  }
  
  void onGone(const usb_device_handle_t *dev_hdl) override {
    if(this->usbTransfer_send) {
      usb_host_endpoint_clear(*dev_hdl, usbTransfer_send->bEndpointAddress);
      usb_host_transfer_free(this->usbTransfer_send);
      this->usbTransfer_send = nullptr;
    }
    if(this->usbTransfer_recv) {
      usb_host_endpoint_clear(*dev_hdl, usbTransfer_recv->bEndpointAddress);
      usb_host_transfer_free(this->usbTransfer_recv);
      this->usbTransfer_recv = nullptr;
    }
    usb_host_interface_release(this->clientHandle, this->deviceHandle, this->InterfaceNumber);
    this->InterfaceNumber = -1;
  }
  
  virtual void onReceive(const uint8_t *data, const size_t length) { if(onReceiveCB) onReceiveCB(data, length); }
  virtual void onReceive(usb_transfer_t *transfer) {
    onReceive((uint8_t *)transfer->data_buffer, transfer->actual_num_bytes);
  }
  virtual void onSend(usb_transfer_t *transfer) {};
  virtual bool isReady() override { return usbTransfer_recv && usbTransfer_send; }
  
private:
  static void _onReceive(usb_transfer_t *transfer) {
    if (transfer->status == 0 && transfer->actual_num_bytes > 0) {
      ((EspUsbHostSerial*)transfer->context)->onReceive(transfer);
    }
  }
  
  static void _onSend(usb_transfer_t *transfer) {
    if (transfer->status == 0 && transfer->actual_num_bytes > 0) {
      EspUsbHostSerial *usbHost = (EspUsbHostSerial*)transfer->context;
      usbHost->onSend(transfer);
    }
  }
  
  void (*onReceiveCB)(const uint8_t *data, const size_t length) = nullptr;
};

#endif
