/**
  ******************************************************************************
  * File Name          : ft6202.c
  * Description        : This file provides code for the touchscreen controller
  *                      TF6202 working along with the OTM8009A TFT Display.
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ft6206.h"



/* --- private functions --- */
static void tc_int_event_callback(void);


/* --- public variables --- */
EXTI_HandleTypeDef exti_line_touch = {
  .Line             = 13,
  .PendingCallback  = tc_int_event_callback,
};
TouchState_t touch_activated_flag = TOUCH_IDLE;







// --------------------------------------------------------------------------

static void tc_int_event_callback(void) {
  touch_activated_flag = TOUCH_ACTIVE;
}





// --------------------------------------------------------------------------

TouchScreen_TypeDef* FT6206_Init(void) {

  static TouchScreen_TypeDef touch_0 = {
    .Model      = 6206,
    .BusHandler = (uint32_t*)&hi2c4,
    .BusAddr    = (FT6206_I2C_ADDR << 1),
    .State      = TOUCH_DISABLED,
  };

  TouchScreen_TypeDef* dev = &touch_0;
  I2C_HandleTypeDef* bus_handler = (I2C_HandleTypeDef*)dev->BusHandler;


  /* Initialize RESET Pin */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  /* Initialize INT Pin */
  GPIO_InitStruct.Pin = LCD_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_INT_GPIO_Port, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);


  if (dev->State == TOUCH_DISABLED) dev->State = TOUCH_LOCKED;
  if (bus_handler->Lock == HAL_LOCKED) return dev;


  uint8_t id;


  if (HAL_I2C_IsDeviceReady(bus_handler, dev->BusAddr, 3, 50) != HAL_OK) return dev;

  HAL_I2C_Mem_Read(bus_handler, dev->BusAddr, 0xa8, I2C_MEMADD_SIZE_8BIT, &id, 1, 30);
  printf("FT6206 ID = 0x%02X\r\n", id);

  dev->State = TOUCH_IDLE;

  return dev;
}





// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) TouchScreen_Process(TouchScreen_TypeDef* dev) {

  dev->Event = TOUCH_ON_UP;
  touch_activated_flag = TOUCH_IDLE;
  return HAL_OK;
}