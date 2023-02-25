#ifndef __EspUsbHostKeyboard_H__
#define __EspUsbHostKeyboard_H__

#include "EspUsbHostHID.h"

class EspUsbHostKeyboard : public EspUsbHostHID {
public:
  EspUsbHostKeyboard() : EspUsbHostHID(0x01) {};
  virtual void setCallback_onKey(void (*callback)(const uint8_t *data, const size_t length)) { onKeyCB = callback; }
  
protected:
  virtual void onKey(const uint8_t *data, const size_t length)              { if(onKeyCB) onKeyCB(data, length); }
private:
  virtual void onReceive(const uint8_t *data, const size_t length) override { onKey(data, length); }
  
  void (*onKeyCB)(const uint8_t *data, const size_t length) = nullptr;
};

#endif
