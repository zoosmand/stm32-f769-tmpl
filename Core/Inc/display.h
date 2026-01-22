#ifndef __DISPLAY_H
#define __DISPLAY_H

#ifdef __cplusplus
  extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "main.h"


/* Private typedef -----------------------------------------------------------*/
typedef struct {
  uint8_t     width;
  uint8_t     height;
  uint32_t    color;
  uint32_t    bgcolor;
  uint8_t     use_bg;
  uint8_t     *font;
} Font_TypeDef;



extern SDRAM_HandleTypeDef hsdram1;

/* Display defines -----------------------------------------------------------*/

// #define _PORTRAIT_
#ifndef _PORTRAIT_
  #define _LANDSCAPE_
#endif

// -------- Portrait ---------
#ifdef _PORTRAIT_
  #define DISPLAY_HEIGHT              (uint16_t)(800)
  #define DISPLAY_WIDTH               (uint16_t)(480)
#endif

// -------- Landscape --------
#ifdef _LANDSCAPE_
  #define DISPLAY_HEIGHT             (uint16_t)(480)
  #define DISPLAY_WIDTH              (uint16_t)(800)
#endif


/* All padding in portrait mode should be zero */
#ifdef _PORTRAIT_ 
  #define L1_PADDING_LEFT             (uint8_t)10
  #define L1_PADDING_RIGHT            (uint8_t)10
  #define L1_PADDING_TOP              (uint8_t)10
  #define L1_PADDING_BOTTOM           (uint8_t)10
  #define L2_PADDING_LEFT             (uint8_t)40
  #define L2_PADDING_RIGHT            (uint8_t)40
  #define L2_PADDING_TOP              (uint8_t)40
  #define L2_PADDING_BOTTOM           (uint8_t)40
#endif

#ifdef _LANDSCAPE_
  #define L1_PADDING_LEFT             (uint8_t)10
  #define L1_PADDING_RIGHT            (uint8_t)10
  #define L1_PADDING_TOP              (uint8_t)10
  #define L1_PADDING_BOTTOM           (uint8_t)10
  #define L2_PADDING_LEFT             (uint8_t)40
  #define L2_PADDING_RIGHT            (uint8_t)40
  #define L2_PADDING_TOP              (uint8_t)40
  #define L2_PADDING_BOTTOM           (uint8_t)40
#endif

#define L1_HEIGHT                   (uint16_t)(DISPLAY_HEIGHT - L1_PADDING_BOTTOM - L1_PADDING_TOP)
#define L1_WIDTH                    (uint16_t)(DISPLAY_WIDTH - L1_PADDING_LEFT - L1_PADDING_RIGHT)
#define L2_HEIGHT                   (uint16_t)(DISPLAY_HEIGHT - L2_PADDING_BOTTOM - L2_PADDING_TOP)
#define L2_WIDTH                    (uint16_t)(DISPLAY_WIDTH - L2_PADDING_LEFT - L2_PADDING_RIGHT)

#define L1_BYTES                    (uint32_t)(L1_HEIGHT * L1_WIDTH * 4)
#define L2_BYTES                    (uint32_t)(L2_HEIGHT * L2_WIDTH * 4)
#define L1_ADDR                     SDRAM_START_ADDR
#define L2_ADDR                     (uint32_t)(L1_ADDR + 0x00800000)
#define L1_MAX_ADDR                 (uint32_t)(L1_ADDR - 1)
#define L2_MAX_ADDR                 SDRAM_MAX_ADDR


/* Private defines -----------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/
#define GET_POSITIOIN_ADDRESS(layer, x, y)  (layer->FBStartAdress + ((x * layer->ImageWidth) + y) * 4)










/* DSI related */
#define LCD_CLOCK                         (uint16_t)27429 // 27429 kHz
#define LANE_BYTE_CLOCK                   (uint16_t)62500 // 500 MHz / 8 = 62.5 MHz = 62500 kHz



/* Exported functions prototypes ---------------------------------------------*/

HAL_StatusTypeDef FillLayer(LTDC_LayerCfgTypeDef*, uint32_t);

HAL_StatusTypeDef FillRectangle(LTDC_LayerCfgTypeDef*, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t);

HAL_StatusTypeDef DrawPixel(LTDC_LayerCfgTypeDef*, uint16_t, uint16_t, uint32_t);

HAL_StatusTypeDef DrawVLine(LTDC_LayerCfgTypeDef*, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t);

HAL_StatusTypeDef DrawHLine(LTDC_LayerCfgTypeDef*, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t);

HAL_StatusTypeDef DrawRectangle(LTDC_LayerCfgTypeDef*, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t);

HAL_StatusTypeDef DrawSymbol(LTDC_LayerCfgTypeDef*, uint16_t*, uint16_t*, const Font_TypeDef*, uint8_t);

HAL_StatusTypeDef PrintString(LTDC_LayerCfgTypeDef*, uint16_t*, uint16_t*, const Font_TypeDef*, const char*, uint8_t);







#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_H */
