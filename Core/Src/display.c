/**
  ******************************************************************************
  * @file           : display.c
  * @brief          : This file contain display routones code.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017-2026 Askug Ltd.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

#include "display.h"



#define SIMPLE_PAUSE 1000U;

static __IO uint32_t step = 0;





/////////////////////////////////////////////////////////////////////////////





// --------------------------------------------------------------------------

void Display_Run(Display_TypeDef* dev) {

  if (dev->Lock == ENABLE) return;

  // Font_TypeDef font = {
  //   .Bgcolor      = COLOR_BLUE,
  //   .Color        = COLOR_LIME,
  //   .Font         = (uint8_t*)&font_dot_20x28,
  //   .Height       = 32,
  //   .Width        = 24,
  //   .BytesPerGlif = 96,
  // };

  // Font_TypeDef font2 = {
  //   .Bgcolor      = COLOR_BLUE,
  //   .Color        = COLOR_LIME,
  //   .Font         = (uint8_t*)&font_dot_5x7,
  //   .Height       = 8,
  //   .Width        = 6,
  //   .BytesPerGlif = 6,
  // };


  uint32_t tick = HAL_GetTick();
  
  if (step >= tick) {
    return;
  } else {
    step = tick + SIMPLE_PAUSE;
    


    if (step % 2) {
      if (Display_FillRectangle(dev->Layer1, 200, 100, 170, 150, (ARGB8888_White | 0xa0000000)) != HAL_OK) return;
    } else {
      if (Display_FillRectangle(dev->Layer2, 200, 100, 170, 150, (ARGB8888_White | 0xa0000000)) != HAL_OK) return;
    }

    // Display_PrintSymbol(dev, 140, 80, &font, '3');
    // Display_PrintString(dev, 40, 180, &font, "CoroideVO!86728543\n");
    
    // Display_PrintString(dev, 40, 86, &font2, "1234567890123456789012345678901234567890123456789012345678901234567890\n");

  }
}



HAL_StatusTypeDef Display_TestSimplePrimitives(LTDC_LayerCfgTypeDef* layer) {
  
  // if (Display_FillRectangle(layer, 200, 100, 170, 150, (ARGB8888_White | 0xa0000000)) != HAL_OK) return HAL_ERROR;

  if (Display_DrawPixel(layer, 10, 10, (ARGB8888_Yellow | 0xff000000)) != HAL_OK) return HAL_ERROR;

  if (Display_DrawVLine(layer, 200, 320, 100, 3, (ARGB8888_Apple | 0xff000000)) != HAL_OK) return HAL_ERROR;
  if (Display_DrawHLine(layer, 200, 320, 100, 2, (ARGB8888_Apple | 0xff000000)) != HAL_OK) return HAL_ERROR;

  if (Display_DrawRectangle(layer, 350, 250, 104, 69, 4, (ARGB8888_Red | 0xff000000)) != HAL_OK) return HAL_ERROR;

  return HAL_OK;
}

