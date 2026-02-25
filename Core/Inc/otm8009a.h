/**
  ******************************************************************************
  * File Name          : otm8009a.h
  * Description        : This file provides code for the configuration
  *                      of the OTM8009A TFT Display.
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __OTM8009A_H
#define __OTM8009A_H


#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"


/* Private macro -------------------------------------------------------------*/



/* Private defines -----------------------------------------------------------*/


/* Display defines -----------------------------------------------------------*/

// #define _PORTRAIT_
#ifndef _PORTRAIT_
  #define _LANDSCAPE_
#endif

// -------- Portrait ---------
#ifdef _PORTRAIT_
  #define DISPLAY_HEIGHT              (uint16_t)(800)
  #define DISPLAY_WIDTH               (uint16_t)(480)
  #define ORIENTATION                 0
#endif

// -------- Landscape --------
#ifdef _LANDSCAPE_
  #define DISPLAY_HEIGHT             (uint16_t)(480)
  #define DISPLAY_WIDTH              (uint16_t)(800)
  #define ORIENTATION                 1
#endif


/* All padding in portrait mode should be zero */
#ifdef _PORTRAIT_ 
  #define L1_PADDING_LEFT             (uint8_t)0
  #define L1_PADDING_RIGHT            (uint8_t)0
  #define L1_PADDING_TOP              (uint8_t)0
  #define L1_PADDING_BOTTOM           (uint8_t)0
  #define L2_PADDING_LEFT             (uint8_t)0
  #define L2_PADDING_RIGHT            (uint8_t)0
  #define L2_PADDING_TOP              (uint8_t)0
  #define L2_PADDING_BOTTOM           (uint8_t)0
#endif

#ifdef _LANDSCAPE_
  #define L1_PADDING_LEFT             (uint8_t)0
  #define L1_PADDING_RIGHT            (uint8_t)0
  #define L1_PADDING_TOP              (uint8_t)0
  #define L1_PADDING_BOTTOM           (uint8_t)0
  #define L2_PADDING_LEFT             (uint8_t)0
  #define L2_PADDING_RIGHT            (uint8_t)0
  #define L2_PADDING_TOP              (uint8_t)0
  #define L2_PADDING_BOTTOM           (uint8_t)0
#endif

#define L1_HEIGHT                   (uint16_t)(DISPLAY_HEIGHT - L1_PADDING_BOTTOM - L1_PADDING_TOP)
#define L1_WIDTH                    (uint16_t)(DISPLAY_WIDTH - L1_PADDING_LEFT - L1_PADDING_RIGHT)
#define L2_HEIGHT                   (uint16_t)(DISPLAY_HEIGHT - L2_PADDING_BOTTOM - L2_PADDING_TOP)
#define L2_WIDTH                    (uint16_t)(DISPLAY_WIDTH - L2_PADDING_LEFT - L2_PADDING_RIGHT)

#define L1_BYTES                    (uint32_t)(L1_HEIGHT * L1_WIDTH * 4)
#define L2_BYTES                    (uint32_t)(L2_HEIGHT * L2_WIDTH * 4)

#define BUF_SIZE                    (uint32_t)0x00200000
#define L1_ADDR                     SDRAM_START_ADDR
#define L1_MAX_ADDR                 (uint32_t)(SDRAM_START_ADDR + BUF_SIZE - 1)  // 2Mb
#define L2_ADDR                     (uint32_t)(L1_MAX_ADDR + 1)
#define L2_MAX_ADDR                 (uint32_t)(L2_ADDR + BUF_SIZE - 1)  // 2Mb

#define BUF2_SIZE                   (uint32_t)0x00200000
#define L1_BUF2_ADDR                (uint32_t)(L2_MAX_ADDR + 1)
#define L1_MAX_BUF2_ADDR            (uint32_t)(L1_BUF2_ADDR + BUF2_SIZE - 1)  // 2Mb
#define L2_BUF2_ADDR                (uint32_t)(L1_MAX_BUF2_ADDR + 1)
#define L2_MAX_BUF2_ADDR            (uint32_t)(L2_BUF2_ADDR + BUF2_SIZE - 1)  // 2Mb

#define L1_FRONT                    L1_ADDR
#define L1_BACK                     L1_BUF2_ADDR
#define L2_FRONT                    L2_ADDR
#define L2_BACK                     L2_BUF2_ADDR




/**
 *  @brief  Possible values of
 *  pixel data format (ie color coding) transmitted on DSI Data lane in DSI packets
 */
#define OTM8009A_FORMAT_RGB888    (uint32_t)0x00 /* Pixel format chosen is RGB888 : 24 bpp */
#define OTM8009A_FORMAT_RBG565    (uint32_t)0x02 /* Pixel format chosen is RGB565 : 16 bpp */

/**
  * @brief  OTM8009A_480X800 Timing parameters for Portrait orientation mode
  */
#define  OTM8009A_480X800_HSYNC             (uint16_t)2      /* Horizontal synchronization */
#define  OTM8009A_480X800_HBP               (uint16_t)34     /* Horizontal back porch      */
#define  OTM8009A_480X800_HFP               (uint16_t)34     /* Horizontal front porch     */
#define  OTM8009A_480X800_VSYNC             (uint16_t)1      /* Vertical synchronization   */
#define  OTM8009A_480X800_VBP               (uint16_t)15     /* Vertical back porch        */
#define  OTM8009A_480X800_VFP               (uint16_t)16     /* Vertical front porch       */

/**
  * @brief  OTM8009A_800X480 Timing parameters for Landscape orientation mode
  *         Same values as for Portrait mode in fact.
  */
 #define  OTM8009A_800X480_HSYNC             OTM8009A_480X800_VSYNC  /* Horizontal synchronization */
 #define  OTM8009A_800X480_HBP               OTM8009A_480X800_VBP    /* Horizontal back porch      */
 #define  OTM8009A_800X480_HFP               OTM8009A_480X800_VFP    /* Horizontal front porch     */
 #define  OTM8009A_800X480_VSYNC             OTM8009A_480X800_HSYNC  /* Vertical synchronization   */
 #define  OTM8009A_800X480_VBP               OTM8009A_480X800_HBP    /* Vertical back porch        */
 #define  OTM8009A_800X480_VFP               OTM8009A_480X800_HFP    /* Vertical front porch       */
 
 /* The following values are same for portrait and landscape orientations */
 #ifdef _LANDSCAPE_
  #define _VSA_         OTM8009A_480X800_VSYNC
  #define _VBP_         OTM8009A_480X800_VBP
  #define _VFP_         OTM8009A_480X800_VFP
  #define _HSA_         OTM8009A_480X800_HSYNC
  #define _HBP_         OTM8009A_480X800_HBP
  #define _HFP_         OTM8009A_480X800_HFP
#endif
 
 #ifdef _PORTRAIT_
  #define _VSA_         OTM8009A_800X480_VSYNC 
  #define _VBP_         OTM8009A_800X480_VBP
  #define _VFP_         OTM8009A_800X480_VFP
  #define _HSA_         OTM8009A_800X480_HSYNC
  #define _HBP_         OTM8009A_800X480_HBP
  #define _HFP_         OTM8009A_800X480_HFP
#endif
 



/* List of OTM8009A used commands                                  */
/* Detailed in OTM8009A Data Sheet 'DATA_SHEET_OTM8009A_V0 92.pdf' */
/* Version of 14 June 2012                                         */
#define  OTM8009A_CMD_NOP                   0x00  /* NOP command      */
#define  OTM8009A_CMD_SWRESET               0x01  /* Sw reset command */
#define  OTM8009A_CMD_RDDMADCTL             0x0B  /* Read Display MADCTR command : read memory display access ctrl */
#define  OTM8009A_CMD_RDDCOLMOD             0x0C  /* Read Display pixel format */
#define  OTM8009A_CMD_SLPIN                 0x10  /* Sleep In command */
#define  OTM8009A_CMD_SLPOUT                0x11  /* Sleep Out command */
#define  OTM8009A_CMD_PTLON                 0x12  /* Partial mode On command */

#define  OTM8009A_CMD_DISPOFF               0x28  /* Display Off command */
#define  OTM8009A_CMD_DISPON                0x29  /* Display On command */

#define  OTM8009A_CMD_CASET                 0x2A  /* Column address set command */
#define  OTM8009A_CMD_PASET                 0x2B  /* Page address set command */

#define  OTM8009A_CMD_RAMWR                 0x2C  /* Memory (GRAM) write command */
#define  OTM8009A_CMD_RAMRD                 0x2E  /* Memory (GRAM) read command  */

#define  OTM8009A_CMD_PLTAR                 0x30  /* Partial area command (4 parameters) */

#define  OTM8009A_CMD_TEOFF                 0x34  /* Tearing Effect Line Off command : command with no parameter */

#define  OTM8009A_CMD_TEEON                 0x35  /* Tearing Effect Line On command : command with 1 parameter 'TELOM' */

/* Parameter TELOM : Tearing Effect Line Output Mode : possible values */
#define OTM8009A_TEEON_TELOM_VBLANKING_INFO_ONLY            0x00
#define OTM8009A_TEEON_TELOM_VBLANKING_AND_HBLANKING_INFO   0x01

#define  OTM8009A_CMD_MADCTR                0x36  /* Memory Access write control command  */

/* Possible used values of MADCTR */
#define OTM8009A_MADCTR_MODE_PORTRAIT       0x00
#define OTM8009A_MADCTR_MODE_LANDSCAPE      0xa0  /* MY = 0, MX = 1, MV = 1, ML = 0, RGB = 0 */

#define  OTM8009A_CMD_IDMOFF                0x38  /* Idle mode Off command */
#define  OTM8009A_CMD_IDMON                 0x39  /* Idle mode On command  */

#define  OTM8009A_CMD_COLMOD                0x3A  /* Interface Pixel format command */

/* Possible values of COLMOD parameter corresponding to used pixel formats */
#define  OTM8009A_COLMOD_RGB565             0x55
#define  OTM8009A_COLMOD_RGB888             0x77

#define  OTM8009A_CMD_RAMWRC                0x3C  /* Memory write continue command */
#define  OTM8009A_CMD_RAMRDC                0x3E  /* Memory read continue command  */

#define  OTM8009A_CMD_WRTESCN               0x44  /* Write Tearing Effect Scan line command */
#define  OTM8009A_CMD_RDSCNL                0x45  /* Read  Tearing Effect Scan line command */

/* CABC Management : ie : Content Adaptive Back light Control in IC OTM8009a */
#define  OTM8009A_CMD_WRDISBV               0x51  /* Write Display Brightness command          */
#define  OTM8009A_CMD_WRCTRLD               0x53  /* Write CTRL Display command                */
#define  OTM8009A_CMD_WRCABC                0x55  /* Write Content Adaptive Brightness command */
#define  OTM8009A_CMD_WRCABCMB              0x5E  /* Write CABC Minimum Brightness command     */

/**
  * @brief  OTM8009A_480X800 frequency divider
  */
#define OTM8009A_480X800_FREQUENCY_DIVIDER  2   /* LCD Frequency divider      */


#define RGB565_Red                  (uint16_t)0xf800
#define RGB565_Green                (uint16_t)0x07e0
#define RGB565_Blue                 (uint16_t)0x001f
#define RGB565_Yellow               (uint16_t)0xffe0
#define RGB565_Black                (uint16_t)0x0000
#define RGB565_White                (uint16_t)0xffff
#define RGB565_Orange               (uint16_t)0xfbe0
#define RGB565_Lightblue            (uint16_t)0x07ff
#define RGB565_Lemon                (uint16_t)0xe7e7
#define RGB565_Apple                (uint16_t)0x0400
#define RGB565_Darkgreen            (uint16_t)0x0380

#define ARGB8888_Red                (uint32_t)0x00ff0000
#define ARGB8888_Green              (uint32_t)0x0000ff00
#define ARGB8888_Blue               (uint32_t)0x000000ff
#define ARGB8888_Yellow             (uint32_t)0x00ffff00
#define ARGB8888_Black              (uint32_t)0x00000000
#define ARGB8888_White              (uint32_t)0x00ffffff
#define ARGB8888_Orange             (uint32_t)0x00ff7f00
#define ARGB8888_Lemon              (uint32_t)0x007fff7f
#define ARGB8888_Lightblue          (uint32_t)0x0000cfff
#define ARGB8888_Apple              (uint32_t)0x0099ee2f
#define ARGB8888_Darkgreen          (uint32_t)0x00003f00


/* Exported macro ------------------------------------------------------------*/
extern DSI_HandleTypeDef hdsi;
extern SDRAM_HandleTypeDef hsdram1;
extern DMA2D_HandleTypeDef hdma2d;
extern LTDC_HandleTypeDef hltdc;





/* Private defines -----------------------------------------------------------*/


/* Exported macro ------------------------------------------------------------*/
#define GET_POSITIOIN_ADDRESS(layer, x, y)  (layer->FBStartAdress + ((x * layer->ImageWidth) + y) * 4)


/* DSI related */
#define LCD_CLOCK                         (uint16_t)27429 // 27429 kHz
#define LANE_BYTE_CLOCK                   (uint16_t)62500 // 500 MHz / 8 = 62.5 MHz = 62500 kHz



/* Exported functions prototypes ---------------------------------------------*/
Display_TypeDef* OTM8009A_Init(void);
HAL_StatusTypeDef OTM8009A_DisplayOn(Display_TypeDef*);
HAL_StatusTypeDef OTM8009A_SetBrightness(Display_TypeDef*, uint8_t);




HAL_StatusTypeDef __attribute__((weak)) Display_FillLayer(Display_TypeDef*, LTCDLayer_t, uint32_t);

HAL_StatusTypeDef __attribute__((weak)) Display_FillRectangle(Display_TypeDef*, LTCDLayer_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t);

HAL_StatusTypeDef __attribute__((weak)) Display_DrawPixel(Display_TypeDef*, LTCDLayer_t, uint16_t, uint16_t, uint32_t);

HAL_StatusTypeDef __attribute__((weak)) Display_DrawVLine(Display_TypeDef*, LTCDLayer_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t);

HAL_StatusTypeDef __attribute__((weak)) Display_DrawHLine(Display_TypeDef*, LTCDLayer_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t);

HAL_StatusTypeDef __attribute__((weak)) Display_DrawRectangle(Display_TypeDef*, LTCDLayer_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t);

/**
  * @brief  Draws a symbol on a layer of the display
  * @param  layer: pointer to the given layer structure
  * @param  x: pointer to x coordinate
  * @param  y: pointer to y coordinate 
  * @param  f: pointer to the given font structure
  * @param  ch: the target sysmbol to draw
  * @retval None
  */
HAL_StatusTypeDef __attribute__((weak)) Display_DrawSymbol(Display_TypeDef*, LTCDLayer_t, uint16_t*, uint16_t*, const Font_TypeDef*, uint8_t);

/**
  * @brief  Draws a string on a layer of the display
  * @param  layer: pointer to the given layer structure
  * @param  x: pointer to x coordinate
  * @param  y: pointer to y coordinate 
  * @param  f: pointer to the given font structure
  * @param  str: pointer to the string buffer
  * @param  wrap: wrap line flag 
  * @retval status of the operation
  */
HAL_StatusTypeDef __attribute__((weak)) Display_PrintString(Display_TypeDef*, LTCDLayer_t, uint16_t*, uint16_t*, const Font_TypeDef*, const char*, bool);

HAL_StatusTypeDef __attribute__((weak)) Display_DrawCircle(Display_TypeDef*, LTCDLayer_t, uint16_t, uint16_t, uint16_t, uint16_t, uint32_t);

HAL_StatusTypeDef __attribute__((weak)) Display_FillCircle(Display_TypeDef*, LTCDLayer_t, uint16_t, uint16_t, uint16_t, uint32_t);

#ifdef __cplusplus
}
#endif
#endif /*__OTM8009A_H */
