#ifndef __DISPLAY_H
#define __DISPLAY_H

#ifdef __cplusplus
  extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "main.h"



HAL_StatusTypeDef Display_TestSimplePrimitives(Display_TypeDef*, LTCDLayer_t);

void Display_Run(Display_TypeDef*);

#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_H */
