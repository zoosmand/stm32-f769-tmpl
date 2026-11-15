#ifndef __DISPLAY_H
#define __DISPLAY_H

#ifdef __cplusplus
  extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "main.h"



#define SIMPLE_PAUSE 1000U;


extern TouchState_t touch_activated_flag;


void Display_Run(Display_TypeDef*, TouchScreen_TypeDef*);




#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_H */
