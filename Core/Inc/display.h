#ifndef __DISPLAY_H
#define __DISPLAY_H

#ifdef __cplusplus
  extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "common.h"
#include "fonts.h"
#include "ft6206.h"
#include "otm8009a.h"



/** Set by the touch EXTI callback and consumed by Display_Run(). */
extern volatile TouchState_t touch_activated_flag;

/**
  * @brief Process one pending touch sample and update the display demo.
  * @param screen Non-null initialized and unlocked display.
  * @param touch Non-null initialized touchscreen.
  * @note Non-blocking when no touch interrupt is pending; call from thread or
  *       main-loop context, never from an interrupt handler.
  */
void Display_Run(Display_TypeDef *screen, TouchScreen_TypeDef *touch);




#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_H */
