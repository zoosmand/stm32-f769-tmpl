#ifndef SELF_TEST_H
#define SELF_TEST_H

#include "common.h"

/** Initialized devices exercised by the peripheral tests. */
typedef struct
{
  CRC_HandleTypeDef *Crc;         /**< Initialized CRC unit (default CRC-32). */
  DMA2D_HandleTypeDef *Dma2d;     /**< DMA2D engine shared with the display. */
  DSI_HandleTypeDef *Dsi;         /**< Started DSI host. */
  I2C_HandleTypeDef *I2c1;        /**< Arduino/extension I2C bus. */
  I2C_HandleTypeDef *I2c4;        /**< Touch and audio-codec I2C bus. */
  LTDC_HandleTypeDef *Ltdc;       /**< Initialized LTDC. */
  UART_HandleTypeDef *Uart;       /**< Console USART1. */
  SDRAM_HandleTypeDef *Sdram;     /**< Initialized external SDRAM. */
  Display_TypeDef *Display;       /**< Display returned by OTM8009A_Init(). */
  TouchScreen_TypeDef *Touch;     /**< Touchscreen returned by FT6206_Init(). */
} SelfTest_Context_TypeDef;

/** Aggregate pass/fail count of executed checks. */
typedef struct
{
  uint16_t Passed;                /**< Tests that passed. */
  uint16_t Failed;                /**< Tests that failed. */
} SelfTest_Result_TypeDef;

/**
  * @brief Run the non-destructive power-on self-test suite.
  * @param context (const SelfTest_Context_TypeDef*) Non-null initialized devices.
  * @retval (SelfTest_Result_TypeDef) Test counts; per-check details go to stdout.
  * @note  Blocking (well under one second). Thread/main-loop context only, after
  *        every tested device has been initialized. Does not alter the display,
  *        framebuffers, or any SDRAM content outside a saved-and-restored probe.
  */
SelfTest_Result_TypeDef SelfTest_Run(const SelfTest_Context_TypeDef *context);

/**
  * @brief Run one named test, the POST suite ("post"), or every test ("all").
  * @param context (const SelfTest_Context_TypeDef*) Non-null initialized devices.
  * @param name (const char*) Non-null test name as printed by SelfTest_PrintList().
  * @param result (SelfTest_Result_TypeDef*) Non-null counters, accumulated.
  * @retval (HAL_StatusTypeDef) HAL_OK when the name was recognized and run;
  *         HAL_ERROR for an unknown name or invalid argument.
  * @note  Blocking. Intrusive tests overwrite the unused upper 8 MiB of SDRAM
  *        and briefly draw a pattern on the display before restoring it.
  */
HAL_StatusTypeDef SelfTest_RunByName(const SelfTest_Context_TypeDef *context,
                                     const char *name,
                                     SelfTest_Result_TypeDef *result);

/** @brief Print every test name, its POST membership, and a description. */
void SelfTest_PrintList(void);

#endif /* SELF_TEST_H */
