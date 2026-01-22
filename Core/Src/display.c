/* Includes ------------------------------------------------------------------*/
#include "display.h"






HAL_StatusTypeDef Display_TestSimplePrimitives(LTDC_LayerCfgTypeDef* layer) {
  
  if (Display_FillRectangle(layer, 200, 100, 170, 150, (ARGB8888_White | 0xa0000000)) != HAL_OK) return HAL_ERROR;

  if (Display_DrawPixel(layer, 10, 10, (ARGB8888_Yellow | 0xff000000)) != HAL_OK) return HAL_ERROR;

  if (Display_DrawVLine(layer, 200, 320, 100, 3, (ARGB8888_Apple | 0xff000000)) != HAL_OK) return HAL_ERROR;
  if (Display_DrawHLine(layer, 200, 320, 100, 2, (ARGB8888_Apple | 0xff000000)) != HAL_OK) return HAL_ERROR;

  if (Display_DrawRectangle(layer, 350, 250, 104, 69, 4, (ARGB8888_Red | 0xff000000)) != HAL_OK) return HAL_ERROR;

  return HAL_OK;
}