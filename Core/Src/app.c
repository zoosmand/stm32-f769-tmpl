#include "app.h"

#include "display.h"

/** Devices owned by the application after successful App_Init(). */
static Display_TypeDef *appDisplay;
static TouchScreen_TypeDef *appTouch;

HAL_StatusTypeDef App_Init(Display_TypeDef *display, TouchScreen_TypeDef *touch)
{
  if ((display == NULL) || (touch == NULL))
  {
    return HAL_ERROR;
  }

  if ((display->Lock == ENABLE) ||
      (touch->State == TOUCH_LOCKED) ||
      (touch->State == TOUCH_DISABLED))
  {
    return HAL_ERROR;
  }

  appDisplay = display;
  appTouch = touch;
  return HAL_OK;
}

void App_Process(void)
{
  if ((appDisplay == NULL) || (appTouch == NULL))
  {
    return;
  }

  Display_Run(appDisplay, appTouch);
}
