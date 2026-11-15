/**
  ******************************************************************************
  * @file           : display.c
  * @brief          : This file contain display routines code.
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

#include <stdio.h>

/** Render the last released touch coordinates and crosshair. */
static void display_HandleTouchUp(Display_TypeDef *screen,
                                  TouchScreen_TypeDef *touch)
{
  Font_TypeDef font = {
    .Bgcolor      = (ARGB8888_Black | 0xff000000),
    .Color        = (ARGB8888_Lightblue | 0xff000000),
    .Font         = (const uint8_t*)font_dot_20x28,
    .Height       = 32,
    .Width        = 24,
    .BytesPerGlif = 96,
  };

  uint16_t lf = 2;

  if (Display_DrawVLine(screen, L1, touch->Context->LastX, 0, screen->Height, lf, screen->BgLayer1) != HAL_OK) return;
  if (Display_DrawHLine(screen, L1, 0, touch->Context->LastY, screen->Width, lf, screen->BgLayer1) != HAL_OK) return;

  if (Display_DrawVLine(screen, L1, touch->Context->X, 0, screen->Height, lf, (ARGB8888_Black | 0xff000000)) != HAL_OK) return;
  if (Display_DrawHLine(screen, L1, 0, touch->Context->Y, screen->Width, lf, (ARGB8888_Black | 0xff000000)) != HAL_OK) return;

  touch->Context->LastX = touch->Context->X;
  touch->Context->LastY = touch->Context->Y;

  uint16_t x1 = 40;
  uint16_t y1 = 40;
  char position[20];
  (void)snprintf(position, sizeof(position), "x:%u y:%u\n",
                 touch->Context->X, touch->Context->Y);

  Display_FillRectangle(screen, L1, 40, 40, (font.Width * 12), font.Height, font.Bgcolor);
  Display_PrintString(screen, L1, &x1, &y1, &font, position, false);

}

void Display_Run(Display_TypeDef *screen, TouchScreen_TypeDef *touch)
{
  if ((screen == NULL) || (touch == NULL)) return;

  if (screen->Lock == ENABLE) return;
  if (touch->State == TOUCH_LOCKED) return;

  if (touch_activated_flag != TOUCH_ACTIVE) return;

  if (TouchScreen_Process(touch) != HAL_OK) return;

  switch (touch->Event) {
    case TOUCH_ON_UP:
      display_HandleTouchUp(screen, touch);
      break;
    case TOUCH_ON_DOWN:
    case TOUCH_ON_HOLD:
    case TOUCH_ON_MOVE:
    case TOUCH_ON_IDLE:
    default:
      __NOP();
      break;
  }
}

