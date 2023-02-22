#ifndef __EspUsbHostMouse_H__
#define __EspUsbHostMouse_H__

#include "EspUsbHostHID.h"

class EspUsbHostMouse : public EspUsbHostHID {
public:
  EspUsbHostMouse() : EspUsbHostHID(0x02) {};
};

#endif
