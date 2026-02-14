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
__STATIC_INLINE void tc_int_event_callback(void);
__STATIC_INLINE HAL_StatusTypeDef tc_read(TouchScreen_TypeDef*);
__STATIC_INLINE void tc_map_to_display(TouchScreen_TypeDef*);


/* --- public variables --- */
EXTI_HandleTypeDef exti_line_touch = {
  .Line             = LCD_INT_Pin_Pos,
  .PendingCallback  = tc_int_event_callback,
};
TouchState_t touch_activated_flag = TOUCH_IDLE;







// --------------------------------------------------------------------------

__STATIC_INLINE void tc_int_event_callback(void) {
  touch_activated_flag = TOUCH_ACTIVE;
}





// --------------------------------------------------------------------------

TouchScreen_TypeDef* FT6206_Init(void) {

  static TouchScreen_TypeDef touch_0 = {
    .Model        = 6206,
    .BusHandler   = (uint32_t*)&hi2c4,
    .BusAddr      = (FT6206_I2C_ADDR << 1),
    .State        = TOUCH_DISABLED,
    .Orientation  = ORIENTATION,
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


  uint8_t raw_data[8];


  if (HAL_I2C_IsDeviceReady(bus_handler, dev->BusAddr, 3, 50) != HAL_OK) return dev;

  HAL_I2C_Mem_Read(bus_handler, dev->BusAddr, FT6x02_DEV_DATA, I2C_MEMADD_SIZE_8BIT, raw_data, 8, 30);
  
  if (raw_data[7] != FT6x02_FOCALTECH_ID_VALUE) {
    printf("FT6x06 ID = 0x%02X, that is not supported by current firmware.\r\n", raw_data[7]);
    return dev;
  }

  /* TODO distribute other hardware data like LIB_VER or FIRMID */

  dev->State = TOUCH_IDLE;

  return dev;
}





// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) TouchScreen_Process(TouchScreen_TypeDef* dev) {

  if (tc_read(dev) != HAL_OK) return HAL_ERROR;
  
  dev->Event = TOUCH_ON_UP;
  touch_activated_flag = TOUCH_IDLE;
  return HAL_OK;
}




// --------------------------------------------------------------------------

__STATIC_INLINE HAL_StatusTypeDef tc_read(TouchScreen_TypeDef* dev) {

  if (dev->State != TOUCH_IDLE) return HAL_ERROR;
  
  uint8_t raw_data[16];

  if (HAL_I2C_Mem_Read((I2C_HandleTypeDef*)dev->BusHandler, dev->BusAddr, FT6x02_DYN_DATA, I2C_MEMADD_SIZE_8BIT, raw_data, sizeof(raw_data), 30) != HAL_OK) return HAL_ERROR;

  tc_map_to_display(dev);

  // dev->Context->Touches = touches;
  // dev->Context->Event   = (buf[1] >> 6) & 0x03;
  dev->Context->RawX    = ((raw_data[3] & 0x0f) << 8) | raw_data[4];
  dev->Context->RawY    = ((raw_data[5] & 0x0f) << 8) | raw_data[6];

  printf("X: %d, Y: %d\r\n", dev->Context->X, dev->Context->Y);


  return HAL_OK;
}



// --------------------------------------------------------------------------

__STATIC_INLINE void tc_map_to_display(TouchScreen_TypeDef* dev) {

  switch (dev->Orientation) {
    case 0:
      dev->Context->X = DISPLAY_WIDTH - dev->Context->RawX;
      dev->Context->Y = dev->Context->RawY;
      break;
      
    case 1:
    default:
      dev->Context->X = dev->Context->RawY;
      dev->Context->Y = dev->Context->RawX;
      break;
  }
}