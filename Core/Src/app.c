#include "app.h"

#include "ft6206.h"
#include "ui.h"

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

  if (Ui_Init(display) != HAL_OK)
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

  /* One touch sample per controller interrupt; a completed tap (released
     press or hold) is a button click. */
  if (touch_activated_flag == TOUCH_ACTIVE)
  {
    if ((TouchScreen_Process(appTouch) == HAL_OK) &&
        ((appTouch->Event == TOUCH_ON_UP) || (appTouch->Event == TOUCH_ON_HOLD)))
    {
      /* Touch coordinates share the display's bottom-left origin; the UI
         uses a top-left origin. */
      const uint16_t touchY = appTouch->Context->Y;
      if (touchY < appDisplay->Height)
      {
        (void)Ui_HandleTap(appTouch->Context->X,
                           (uint16_t)(appDisplay->Height - 1U - touchY));
      }
    }
  }

  Ui_Process();
}
