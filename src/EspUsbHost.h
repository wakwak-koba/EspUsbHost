#ifndef __EspUsbHost_H__
#define __EspUsbHost_H__

#include <Arduino.h>
#include <usb/usb_host.h>

class EspUsbHost {
public:
  void begin(void);
  void task(void);
  
  virtual void setCallback_onNew(void (*callback)(const usb_device_info_t *dev_info, const usb_device_desc_t *dev_desc)) {onNewCB = callback;}
  virtual void setCallback_onGone(void (*callback)()) {onGoneCB = callback;}
  
protected:
  usb_host_client_handle_t clientHandle;
  usb_device_handle_t deviceHandle;
  
  virtual void onConfig(const usb_device_info_t *dev_info) {};
  virtual void onConfig(const usb_device_desc_t *dev_desc) {};
  virtual void onConfig(const uint8_t bDescriptorType, const uint8_t *p);
  virtual void onNew(const usb_device_info_t *dev_info, const usb_device_desc_t *dev_desc) { if(onNewCB) onNewCB(dev_info, dev_desc); }
  virtual void onGone() { if(onGoneCB) onGoneCB(); }
  virtual void onGone(const usb_device_handle_t *dev_hdl) {};
  virtual bool isReady() { return false; }
  
private:
  uint32_t eventFlags;
  void (*onNewCB)(const usb_device_info_t *dev_info, const usb_device_desc_t *dev_desc) = nullptr;
  void (*onGoneCB)() = nullptr;
  
  static void _clientEventCallback(const usb_host_client_event_msg_t *eventMsg, void *arg);
  void _configCallback(const usb_device_info_t *dev_info);
  void _configCallback(const usb_device_desc_t *dev_desc);
  void _configCallback(const usb_config_desc_t *config_desc);
};

#endif
