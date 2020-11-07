#pragma once

#include "usbh_def.h"

#ifdef __cplusplus
extern "C"
{
#endif

void setUSBKeyboardLEDs(uint8_t leds);
void HID_Keybd_UserProcess(USBH_HandleTypeDef *phost);

#ifdef __cplusplus
}
#endif
