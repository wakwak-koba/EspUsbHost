#ifndef __EspUsbHost_H__
#define __EspUsbHost_H__

#include <Arduino.h>
#include <usb/usb_host.h>

class EspUsbHost {
public:
  virtual void begin(void);
  virtual void task(void);
  
  std::string getManufacturer();
  std::string getProduct();
  std::string getSerialNum();
  
  virtual void setCallback_onNew (void (*callback)()) {onNewCB  = callback;}
  virtual void setCallback_onGone(void (*callback)()) {onGoneCB = callback;}
  
protected:
  usb_host_client_handle_t clientHandle;
  usb_device_handle_t deviceHandle;
  usb_device_info_t deviceInfo;
  usb_device_desc_t deviceDesc;
  
  virtual void onConfig(const usb_device_info_t *dev_info) {};
  virtual void onConfig(const usb_device_desc_t *dev_desc) {};
  virtual void onConfig(const uint8_t bDescriptorType, const uint8_t *p);
  virtual void onConfig(const usb_config_desc_t *config_desc) {};
  virtual void onNew()  { if(onNewCB ) onNewCB (); }
  virtual void onGone() { if(onGoneCB) onGoneCB(); }
  virtual void onGone(const usb_device_handle_t *dev_hdl) {};
  virtual bool isReady() { return false; }
          esp_err_t submit_control(const uint8_t requestType, const uint8_t bRequest, const uint16_t wValue);
          esp_err_t submit_control(const uint8_t requestType, const uint8_t bRequest, const uint16_t wValue, const uint16_t wIndex, const uint16_t wLength, const void *data);
  
private:
  uint32_t eventFlags;
  bool raisedOnNew = false;
  void (*onNewCB )() = nullptr;
  void (*onGoneCB)() = nullptr;
  
  static void _clientEventCallback(const usb_host_client_event_msg_t *eventMsg, void *arg);
  void _configCallback(const usb_device_info_t *dev_info);
  void _configCallback(const usb_device_desc_t *dev_desc);
  void _configCallback(const usb_config_desc_t *config_desc);
};

#endif
