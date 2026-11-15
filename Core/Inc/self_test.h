#ifndef SELF_TEST_H
#define SELF_TEST_H

#include "common.h"

/** Initialized devices inspected by the non-destructive power-on self-test. */
typedef struct
{
  CRC_HandleTypeDef *Crc;
  DMA2D_HandleTypeDef *Dma2d;
  DSI_HandleTypeDef *Dsi;
  I2C_HandleTypeDef *I2c1;
  I2C_HandleTypeDef *I2c4;
  LTDC_HandleTypeDef *Ltdc;
  UART_HandleTypeDef *Uart;
  SDRAM_HandleTypeDef *Sdram;
  Display_TypeDef *Display;
  TouchScreen_TypeDef *Touch;
} SelfTest_Context_TypeDef;

/** Aggregate result printed after all power-on tests have executed. */
typedef struct
{
  uint16_t Passed;
  uint16_t Failed;
} SelfTest_Result_TypeDef;

/**
  * @brief Run bounded, non-destructive tests of initialized platform devices.
  * @param context Non-null collection of initialized handles and devices.
  * @retval Number of passing and failing checks; details are printed to stdout.
  * @note Run in thread/main-loop context after all tested devices initialize.
  */
SelfTest_Result_TypeDef SelfTest_Run(const SelfTest_Context_TypeDef *context);

#endif /* SELF_TEST_H */
