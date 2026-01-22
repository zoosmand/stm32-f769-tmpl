/* Includes ------------------------------------------------------------------*/
#include "display.h"


// /* Private variables ---------------------------------------------------------*/
// const Font_TypeDef font_dot15x21 = {
//   .width    = 18,
//   .height   = 24,
//   .color    = ARGB8888_White,
//   .bgcolor  = ARGB8888_Darkgreen,
//   .use_bg   = 1,
//   .font     = (uint8_t*)font_15x21
// };

// const Font_TypeDef font_dot20x28 = {
//   .width    = 24,
//   .height   = 32,
//   .color    = ARGB8888_White,
//   .bgcolor  = ARGB8888_Darkgreen,
//   .use_bg   = 1,
//   .font =   (uint8_t*)font_20x28
// };

// const Font_TypeDef font_dot10x14 = {
//   .width = 12,
//   .height = 16,
//   .color = _YELLOW,
//   .bgcolor = _RED_0,
//   .use_bg = 1,
//   .font = (uint8_t*)font_10x14
// };

// const Font_TypeDef dateTimeFont = {
//   .width = 12,
//   .height = 16,
//   .color = _RED_128,
//   .bgcolor = _RED_0,
//   .use_bg = 1,
//   .font = (uint8_t*)font_10x14
// };

// static uint16_t xStart = 0;
// static uint16_t yStart = 0;










////////////////////////////////////////////////////////////////////////////////




/**
  * @brief  Fills a layer of display with color
  * @param  layer: number of layer
  * @param  color: color
  * @retval None
  */
HAL_StatusTypeDef FillLayer(LTDC_LayerCfgTypeDef* layer, uint32_t color) {
  if (FillRectangle(layer, 0, 0, layer->ImageHeight, layer->ImageWidth, color) != HAL_OK) return HAL_ERROR;
  return HAL_OK;
}




/**
  * @brief  Draws a pixel on a layer of display with color
  * @param  layer: number of layer
  * @param  x: x coordinate
  * @param  y: y coordinate 
  * @param  color: color
  * @retval None
  */
HAL_StatusTypeDef DrawPixel(LTDC_LayerCfgTypeDef* layer, uint16_t x, uint16_t y, uint32_t color) {
  if (SDRAM_BusyStatusCheck(&hsdram1) != HAL_OK) return HAL_ERROR;
  __O uint32_t* fb = (uint32_t*)(GET_POSITIOIN_ADDRESS(layer, x, y));
  *fb = color;
  return HAL_OK;
}




/**
  * @brief  Draws a vertical line on a layer of display with color
  * @param  layer: number of layer
  * @param  x: x coordinate
  * @param  y: y coordinate 
  * @param  h: height of a line
  * @param  color: color
  * @retval None
  */
HAL_StatusTypeDef DrawVLine(LTDC_LayerCfgTypeDef* layer, uint16_t x, uint16_t y, uint16_t h, uint16_t t, uint32_t color) {
  if (FillRectangle(layer, x, y, t, h, color) != HAL_OK) return HAL_ERROR;
  return HAL_OK;
}





/**
  * @brief  Draws a horizontal line on a layer of display with color
  * @param  layer: number of layer
  * @param  x: x coordinate
  * @param  y: y coordinate 
  * @param  w: weight of a line
  * @param  color: color
  * @retval None
  */
HAL_StatusTypeDef DrawHLine(LTDC_LayerCfgTypeDef* layer, uint16_t x, uint16_t y, uint16_t w, uint16_t t, uint32_t color) {
  if (FillRectangle(layer, x, y, w, t, color) != HAL_OK) return HAL_ERROR;
  return HAL_OK;
}



/**
  * @brief  Draws a rectangle 
  * @param  layer: number of layer
  * @param  x: x coordinate
  * @param  y: y coordinate 
  * @param  w: weight of a line
  * @param  color: color
  * @retval None
  */
HAL_StatusTypeDef DrawRectangle(LTDC_LayerCfgTypeDef* layer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t t, uint32_t color) {
  if (DrawHLine(layer, x, y, w, t, color) != HAL_OK) return HAL_ERROR;
  if (DrawHLine(layer, x, (y + h), (w + t), t, color) != HAL_OK) return HAL_ERROR;
  if (DrawVLine(layer, x, y, h, t, color) != HAL_OK) return HAL_ERROR;
  if (DrawVLine(layer, (x + w), y, (h + t), t, color) != HAL_OK) return HAL_ERROR;
  return HAL_OK;
}



/**
  * @brief  Fills rectangle on a layer of display with color
  * @param  layer: number of layer
  * @param  x: x coordinate
  * @param  y: y coordinate 
  * @param  h: height of a rectangle
  * @param  w: weight of a rectangle
  * @param  color: color
  * @retval None
  */
HAL_StatusTypeDef FillRectangle(LTDC_LayerCfgTypeDef* layer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) {
  if ((x + w - 1) >= layer->ImageHeight) return HAL_ERROR;
  if ((y + h - 1) >= layer->ImageWidth) return HAL_ERROR;

  for (uint32_t iw = 0; iw < w ; iw++) {

    if (SDRAM_BusyStatusCheck(&hsdram1) != HAL_OK) return HAL_ERROR;

    __O uint32_t* fb = (uint32_t*)(GET_POSITIOIN_ADDRESS(layer, (x + (iw * 1)), y)); 
    
    uint16_t hh = h;
    while (hh--) *fb++ = color;
  }

  return HAL_OK;
}




/**
  * @brief  Draws symbol on a layer of display
  * @param  layer: number of layer
  * @param  x: pointer to x coordinate
  * @param  y: pointer to y coordinate 
  * @param  font: pointer to font structure containes font paramenters:
  *               - width of a symbol
  *               - height of a symbol
  *               - color of a symbol
  *               - background color behind a symbol
  *               - flag - use or don't use background color
  *               - array of symbols
  * @param  pos: position in the sysmbol's array
  * @retval None
  */
void DrawSymbol(LTDC_LayerCfgTypeDef* layer, uint16_t* x, uint16_t* y, const Font_TypeDef *font, uint8_t ch) {
  if ((ch > 126) || (ch < 32)) {
    if (ch == 176) ch = 95;
    else return;
  } else {
    ch -= 32;
  }



}




/**
  * @brief  Prints a string on a layer of display
  * @param  layer: number of layer
  * @param  x: pointer to x coordinate
  * @param  y: pointer to y coordinate 
  * @param  font: pointer to font structure containes font paramenters:
  *               - width of a symbol
  *               - height of a symbol
  *               - color of a symbol
  *               - background color behind a symbol
  *               - flag - use or not use background color
  *               - array of symbols
  * @param  buf: pointer to buffer with a string
  * @param  wrap: wrap or not wrap to the next line 
  * @retval None
  */
void PrintString(LTDC_LayerCfgTypeDef* layer, uint16_t *x, uint16_t *y, const Font_TypeDef *font, const char *buf, uint8_t wrap) {

}

