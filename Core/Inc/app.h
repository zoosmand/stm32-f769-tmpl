#ifndef APP_H
#define APP_H

#include "common.h"

/**
  * @brief Bind initialized board devices to the application.
  * @param display Non-null, unlocked display returned by OTM8009A_Init().
  * @param touch Non-null, active touchscreen returned by FT6206_Init().
  * @retval HAL_OK when both devices are ready; HAL_ERROR otherwise.
  * @note Call once after platform initialization and before App_Process().
  */
HAL_StatusTypeDef App_Init(Display_TypeDef *display, TouchScreen_TypeDef *touch);

/**
  * @brief Execute one non-blocking iteration of application work.
  * @note The FreeRTOS platform task calls this periodically after board
  *       initialization and POST complete.
  */
void App_Process(void);

#endif /* APP_H */
