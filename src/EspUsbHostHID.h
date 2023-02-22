#ifndef __EspUsbHostHID_H__
#define __EspUsbHostHID_H__

#include "EspUsbHost.h"
#include <class/hid/hid.h>

class EspUsbHostHID : public EspUsbHost {
public:
  void task(void) {
    EspUsbHost::task();
    if (this->isReady()) {
      unsigned long now = millis();
      if ((now - this->lastCheck) > this->interval) {
        this->lastCheck = now;
        esp_err_t err = usb_host_transfer_submit(this->usbTransfer);
        if (err != ESP_OK && err != ESP_ERR_NOT_FINISHED && err != ESP_ERR_INVALID_STATE) {
          ESP_LOGI("EspUsbHostHID", "usb_host_transfer_submit() err=%x", err);
        }
      }
    }
  }

  virtual void setCallback_onReceive(void (*callback)(usb_transfer_t *transfer)) {onReceiveCB = callback;}
  
protected:
  uint8_t interval;
  unsigned long lastCheck;
  usb_transfer_t *usbTransfer;
  uint8_t InterfaceProtocol = 0;
  int16_t InterfaceNumber = -1;
  
  EspUsbHostHID(uint8_t InterfaceProtocol) : InterfaceProtocol(InterfaceProtocol) {};
  
  void onConfig(const usb_device_desc_t *dev_desc) {
    ESP_LOGI("EspUsbHostHID", "idVendor=%04x idProduct=%04x", dev_desc->idVendor, dev_desc->idProduct);
  }
  
  void onConfig(const uint8_t bDescriptorType, const uint8_t *p) {
    switch (bDescriptorType) {
      case USB_B_DESCRIPTOR_TYPE_INTERFACE:
        {
          const usb_intf_desc_t *intf = (const usb_intf_desc_t *)p;
          ESP_LOGI("EspUsbHostHID", "bDescriptorType=%02x bInterfaceClass=%02x bInterfaceSubClass=%02x bInterfaceProtocol=%02x bAlternateSetting=%02x bNumEndpoints=%d", bDescriptorType, intf->bInterfaceClass, intf->bInterfaceSubClass, intf->bInterfaceProtocol, intf->bAlternateSetting, intf->bNumEndpoints);

          if ((intf->bInterfaceClass == USB_CLASS_HID) && (intf->bInterfaceSubClass == 0x01) && (intf->bInterfaceProtocol == InterfaceProtocol)) {
            esp_err_t err = usb_host_interface_claim(this->clientHandle, this->deviceHandle, intf->bInterfaceNumber, intf->bAlternateSetting);
            if (err != ESP_OK) {
              ESP_LOGI("EspUsbHostHID", "usb_host_interface_claim() err=%x", err);
            } else {
              this->InterfaceNumber = intf->bInterfaceNumber;
              ESP_LOGI("EspUsbHostHID", "bInterfaceNumber=%d", intf->bInterfaceNumber);
            }
          }
        }
        break;

      case USB_B_DESCRIPTOR_TYPE_ENDPOINT:
        if (this->usbTransfer == NULL) {
          const usb_ep_desc_t *endpoint = (const usb_ep_desc_t *)p;
          ESP_LOGI("EspUsbHostHID", "bDescriptorType=%02x bEndpointAddress=%02x bmAttributes=%02x wMaxPacketSize=%d bInterval=%d", bDescriptorType, endpoint->bEndpointAddress, endpoint->bmAttributes, endpoint->wMaxPacketSize, endpoint->bInterval);
          esp_err_t err;

          if ((endpoint->bmAttributes & USB_BM_ATTRIBUTES_XFERTYPE_MASK) != USB_BM_ATTRIBUTES_XFER_INT) {
            ESP_LOGI("EspUsbHostHID", "err endpoint->bmAttributes=%x", endpoint->bmAttributes);
            return;
          }

          if (endpoint->bEndpointAddress & USB_B_ENDPOINT_ADDRESS_EP_DIR_MASK) {
            err = usb_host_transfer_alloc(endpoint->wMaxPacketSize, 0, &this->usbTransfer);
            if (err != ESP_OK) {
              this->usbTransfer = NULL;
              ESP_LOGI("EspUsbHostHID", "usb_host_transfer_alloc() err=%x", err);
              return;
            }

            this->usbTransfer->device_handle = this->deviceHandle;
            this->usbTransfer->bEndpointAddress = endpoint->bEndpointAddress;
            this->usbTransfer->callback = this->_onReceive;
            this->usbTransfer->context = this;
            this->usbTransfer->num_bytes = endpoint->wMaxPacketSize;
            interval = endpoint->bInterval;
            ESP_LOGI("EspUsbHostHID", "usbTransfer");
          }
        }
        break;

      case USB_B_DESCRIPTOR_TYPE_WIRE_ADAPTER: {
          const tusb_hid_descriptor_hid_t *hid_desc = (const tusb_hid_descriptor_hid_t *)p;
          ESP_LOGI("EspUsbHostHID", "USB_HID_DESC(0x21) bLength=%d, bDescriptorType=0x%x, bcdHID=0x%x, bCountryCode=0x%x, bNumDescriptors=%d, bReportType=0x%x, wReportLength=%d",
                 hid_desc->bLength,
                 hid_desc->bDescriptorType,
                 hid_desc->bcdHID,
                 hid_desc->bCountryCode,
                 hid_desc->bNumDescriptors,
                 hid_desc->bReportType,
                 hid_desc->wReportLength);      
        }
        break;

      default:
        ESP_LOGI("EspUsbHostHID", "bDescriptorType=%02x", bDescriptorType);
        break;
    }
  }

  void onGone(const usb_device_handle_t *dev_hdl) {
    if(this->usbTransfer) {
      usb_host_endpoint_clear(*dev_hdl, usbTransfer->bEndpointAddress);
      usb_host_transfer_free(this->usbTransfer);
      this->usbTransfer = nullptr;
    }
    usb_host_interface_release(this->clientHandle, this->deviceHandle, this->InterfaceNumber);
    this->InterfaceNumber = -1;
  }
  
  virtual void onReceive(usb_transfer_t *transfer) { if(onReceiveCB) onReceiveCB(transfer); }
  virtual bool isReady() { return usbTransfer; }

private:
  static void _onReceive(usb_transfer_t *transfer) {
    if (transfer->status == 0 && transfer->actual_num_bytes > 0) {
      ((EspUsbHostHID*)transfer->context)->onReceive(transfer);
    }
  }

  void (*onReceiveCB)(usb_transfer_t *transfer) = nullptr;
};

#endif
