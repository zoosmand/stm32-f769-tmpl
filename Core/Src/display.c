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



static __IO uint32_t step = 0;






/////////////////////////////////////////////////////////////////////////////



// --------------------------------------------------------------------------

static void on_move(Display_TypeDef* screen, TouchScreen_TypeDef* touch) {
  //
}



// --------------------------------------------------------------------------

static void on_up(Display_TypeDef* screen, TouchScreen_TypeDef* touch) {
  //
  if (Display_DrawCircle(screen, L1, 100, 300, 50, 2, (ARGB8888_Blue | 0xff000000)) != HAL_OK) return;
}


// --------------------------------------------------------------------------

static void on_hold(Display_TypeDef* screen, TouchScreen_TypeDef* touch) {
  //
}


// --------------------------------------------------------------------------

static void on_down(Display_TypeDef* screen, TouchScreen_TypeDef* touch) {
  //
  if (Display_FillCircle(screen, L2, 100, 200, 50, (ARGB8888_Blue | 0xff000000)) != HAL_OK) return;
}



// --------------------------------------------------------------------------

void Display_Run(Display_TypeDef* screen, TouchScreen_TypeDef* touch) {

  if (screen->Lock == ENABLE) return;
  if (touch->State == TOUCH_LOCKED) return;


  if (touch_activated_flag != TOUCH_ACTIVE) return;

  TouchScreen_Process(touch);

  switch (touch->Event) {
    case TOUCH_ON_DOWN:
      on_down(screen, touch);
      break;
    
    case TOUCH_ON_UP:
      on_up(screen, touch);
      break;
    
    case TOUCH_ON_HOLD:
      on_hold(screen, touch);
      break;
    
    case TOUCH_ON_MOVE:
      on_move(screen, touch);
      break;
    
    case TOUCH_ON_IDLE:
      default:
      __NOP();
      break;
  }

  // Font_TypeDef font = {
  //   .Bgcolor      = (ARGB8888_Black | 0xff000000),
  //   .Color        = (ARGB8888_Lightblue | 0xff000000),
  //   .Font         = (uint8_t*)&font_dot_20x28,
  //   .Height       = 32,
  //   .Width        = 24,
  //   .BytesPerGlif = 96,
  // };

  // Font_TypeDef font2 = {
  //   .Bgcolor      = (ARGB8888_Black | 0xff000000),
  //   .Color        = (ARGB8888_Apple | 0xff000000),
  //   .Font         = (uint8_t*)&font_dot_5x7,
  //   .Height       = 8,
  //   .Width        = 6,
  //   .BytesPerGlif = 6,
  // };


  // uint32_t tick = HAL_GetTick();
  
  // if (step >= tick) {
  //   return;
  // } else {
  //   step = tick + SIMPLE_PAUSE;
    

  //   uint16_t x1 = 240;
  //   uint16_t y1 = 80;
  //   uint16_t x2 = 5;
  //   uint16_t y2 = 120;

  //   if (Display_PrintString(screen, L1, &x1, &y1, &font, "CoroideVO!86728543\n", false) != HAL_OK) return;
    
  //   if (Display_PrintString(screen, L2, &x2, &y2, &font2, "1234567890123456789012345678901234567890123456789012345678901234567890\n", false) != HAL_OK) return;




  //   if (Display_DrawPixel(screen, L1, 10, 10, (ARGB8888_Yellow | 0xff000000)) != HAL_OK) return;

  //   if (Display_DrawVLine(screen, L2, 200, 320, 100, 3, (ARGB8888_Black | 0xff000000)) != HAL_OK) return;
  //   if (Display_DrawHLine(screen, L1, 200, 320, 100, 2, (ARGB8888_Black | 0xff000000)) != HAL_OK) return;

  //   if (Display_DrawRectangle(screen, L2, 350, 250, 104, 69, 4, (ARGB8888_Red | 0xff000000)) != HAL_OK) return;
  //   if (Display_FillRectangle(screen, L2, 550, 350, 104, 69, (ARGB8888_Red | 0xff000000)) != HAL_OK) return;

  //   if (Display_DrawCircle(screen, L1, 100, 300, 50, 2, (ARGB8888_Blue | 0xff000000)) != HAL_OK) return;

  //   if (Display_FillCircle(screen, L2, 100, 200, 50, (ARGB8888_Blue | 0xff000000)) != HAL_OK) return;

  // }
}

