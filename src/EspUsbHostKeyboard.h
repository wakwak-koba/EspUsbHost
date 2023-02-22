#ifndef __EspUsbHostKeyboard_H__
#define __EspUsbHostKeyboard_H__

#include "EspUsbHostHID.h"

class EspUsbHostKeyboard : public EspUsbHostHID {
public:
  EspUsbHostKeyboard() : EspUsbHostHID(0x01) {};
};

#endif
