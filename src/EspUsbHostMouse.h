#ifndef __EspUsbHostMouse_H__
#define __EspUsbHostMouse_H__

#include "EspUsbHostHID.h"

class EspUsbHostMouse : public EspUsbHostHID {
public:
  EspUsbHostMouse() : EspUsbHostHID(0x02) {};
  virtual void setCallback_onMouse(void (*callback)(const uint8_t *data, const size_t length)) { onMouseCB = callback; }
  
protected:
  virtual void onMouse(const uint8_t *data, const size_t length)            { if(onMouseCB) onMouseCB(data, length); }
private:
  virtual void onReceive(const uint8_t *data, const size_t length) override { onMouse(data, length); }
  
  void (*onMouseCB)(const uint8_t *data, const size_t length) = nullptr;
};

#endif
