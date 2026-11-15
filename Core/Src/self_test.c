#include "self_test.h"

#include "fmc.h"
#include "main.h"

#include <stdio.h>

#define SELF_TEST_SDRAM_WORDS 16U
#define SELF_TEST_SDRAM_OFFSET \
  (SDRAM_SIZE_BYTES - (SELF_TEST_SDRAM_WORDS * sizeof(uint32_t)))

static void selfTest_Record(SelfTest_Result_TypeDef *result, const char *name,
                            bool passed, uint32_t detail)
{
  if (passed)
  {
    result->Passed++;
    printf("[PASS] %-18s 0x%08lX\r\n", name, (unsigned long)detail);
  }
  else
  {
    result->Failed++;
    printf("[FAIL] %-18s 0x%08lX\r\n", name, (unsigned long)detail);
  }
}

static bool selfTest_Sdram(SDRAM_HandleTypeDef *sdram)
{
  uint32_t saved[SELF_TEST_SDRAM_WORDS];
  uint32_t pattern[SELF_TEST_SDRAM_WORDS];
  uint32_t observed[SELF_TEST_SDRAM_WORDS];

  if (SDRAM_ReadBuffer(sdram, SELF_TEST_SDRAM_OFFSET,
                       SELF_TEST_SDRAM_WORDS, saved) != HAL_OK)
  {
    return false;
  }

  for (uint32_t index = 0U; index < SELF_TEST_SDRAM_WORDS; index++)
  {
    pattern[index] = UINT32_C(0xA5A50000) ^
                     (index * UINT32_C(0x01010101));
  }

  bool passed =
      (SDRAM_WriteBuffer(sdram, SELF_TEST_SDRAM_OFFSET,
                         SELF_TEST_SDRAM_WORDS, pattern) == HAL_OK) &&
      (SDRAM_ReadBuffer(sdram, SELF_TEST_SDRAM_OFFSET,
                        SELF_TEST_SDRAM_WORDS, observed) == HAL_OK);

  if (passed)
  {
    for (uint32_t index = 0U; index < SELF_TEST_SDRAM_WORDS; index++)
    {
      if (observed[index] != pattern[index])
      {
        passed = false;
        break;
      }
    }
  }

  if (SDRAM_WriteBuffer(sdram, SELF_TEST_SDRAM_OFFSET,
                        SELF_TEST_SDRAM_WORDS, saved) != HAL_OK)
  {
    passed = false;
  }

  return passed;
}

SelfTest_Result_TypeDef SelfTest_Run(const SelfTest_Context_TypeDef *context)
{
  SelfTest_Result_TypeDef result = {0U, 0U};
  printf("\r\nSTM32F769I-DISCO power-on self-test\r\n");
  printf("-----------------------------------\r\n");

  if (context == NULL)
  {
    selfTest_Record(&result, "test context", false, 0U);
    return result;
  }

  const uint32_t deviceId = HAL_GetDEVID();
  selfTest_Record(&result, "device ID", deviceId == 0x451U, deviceId);
  selfTest_Record(&result, "system clock", SystemCoreClock == 216000000U,
                  SystemCoreClock);

  const uint32_t coreFeatures = SCB->CCR & (SCB_CCR_IC_Msk | SCB_CCR_DC_Msk);
  selfTest_Record(&result, "I/D cache", coreFeatures ==
                  (SCB_CCR_IC_Msk | SCB_CCR_DC_Msk), coreFeatures);
  selfTest_Record(&result, "MPU enabled",
                  (MPU->CTRL & MPU_CTRL_ENABLE_Msk) != 0U, MPU->CTRL);

  selfTest_Record(&result, "USART1",
                  (context->Uart != NULL) &&
                  (HAL_UART_GetState(context->Uart) == HAL_UART_STATE_READY),
                  (context->Uart != NULL) ? HAL_UART_GetState(context->Uart) : 0U);
  selfTest_Record(&result, "CRC",
                  (context->Crc != NULL) &&
                  (HAL_CRC_GetState(context->Crc) == HAL_CRC_STATE_READY),
                  (context->Crc != NULL) ? HAL_CRC_GetState(context->Crc) : 0U);
  selfTest_Record(&result, "I2C1",
                  (context->I2c1 != NULL) &&
                  (HAL_I2C_GetState(context->I2c1) == HAL_I2C_STATE_READY),
                  (context->I2c1 != NULL) ? HAL_I2C_GetState(context->I2c1) : 0U);
  selfTest_Record(&result, "I2C4",
                  (context->I2c4 != NULL) &&
                  (HAL_I2C_GetState(context->I2c4) == HAL_I2C_STATE_READY),
                  (context->I2c4 != NULL) ? HAL_I2C_GetState(context->I2c4) : 0U);
  selfTest_Record(&result, "FT6206",
                  (context->Touch != NULL) &&
                  (context->Touch->State != TOUCH_LOCKED) &&
                  (context->Touch->State != TOUCH_DISABLED),
                  (context->Touch != NULL) ? context->Touch->State : 0U);

  const bool sdramPassed = (context->Sdram != NULL) &&
                           selfTest_Sdram(context->Sdram);
  selfTest_Record(&result, "FMC SDRAM", sdramPassed,
                  SELF_TEST_SDRAM_OFFSET);
  selfTest_Record(&result, "DMA2D",
                  (context->Dma2d != NULL) &&
                  (HAL_DMA2D_GetState(context->Dma2d) == HAL_DMA2D_STATE_READY),
                  (context->Dma2d != NULL) ? HAL_DMA2D_GetState(context->Dma2d) : 0U);
  selfTest_Record(&result, "DSI host",
                  (context->Dsi != NULL) &&
                  (HAL_DSI_GetState(context->Dsi) == HAL_DSI_STATE_READY),
                  (context->Dsi != NULL) ? HAL_DSI_GetState(context->Dsi) : 0U);
  selfTest_Record(&result, "LTDC/display",
                  (context->Ltdc != NULL) && (context->Display != NULL) &&
                  (context->Display->Lock == DISABLE) &&
                  (HAL_LTDC_GetState(context->Ltdc) == HAL_LTDC_STATE_READY),
                  (context->Ltdc != NULL) ? HAL_LTDC_GetState(context->Ltdc) : 0U);

  HAL_GPIO_WritePin(LD_USER1_GPIO_Port, LD_USER1_Pin, GPIO_PIN_SET);
  const bool ledSet = (LD_USER1_GPIO_Port->ODR & LD_USER1_Pin) != 0U;
  HAL_GPIO_WritePin(LD_USER1_GPIO_Port, LD_USER1_Pin, GPIO_PIN_RESET);
  const bool ledReset = (LD_USER1_GPIO_Port->ODR & LD_USER1_Pin) == 0U;
  selfTest_Record(&result, "user LED GPIO", ledSet && ledReset,
                  LD_USER1_GPIO_Port->ODR);

  printf("-----------------------------------\r\n");
  printf("POST result: %u passed, %u failed\r\n\r\n",
         result.Passed, result.Failed);
  return result;
}
