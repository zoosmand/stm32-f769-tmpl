/**
  ******************************************************************************
  * File Name          : otm8009a.c
  * Description        : This file provides code for the configuration
  *                      of the OTM8009A TFT Display.
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "otm8009a.h"





__IO uint8_t vblank_ready = 0;

/*
 * @brief Constant tables of register settings used to transmit DSI
 * command packets as power up initialization sequence of the KoD LCD (OTM8009A LCD Driver)
 */
static const uint8_t lcdRegData1[]  = { 0x80, 0x09, 0x01, 0xff };
static const uint8_t lcdRegData2[]  = { 0x80, 0x09, 0xff };
static const uint8_t lcdRegData3[]  = { 0x00, 0x09, 0x0f, 0x0e, 0x07, 0x10, 0x0b, 0x0a, 0x04, 0x07, 0x0b, 0x08, 0x0f, 0x10, 0x0a, 0x01, 0xe1};
static const uint8_t lcdRegData4[]  = { 0x00, 0x09, 0x0f, 0x0e, 0x07, 0x10, 0x0b, 0x0a, 0x04, 0x07, 0x0b, 0x08, 0x0f, 0x10, 0x0a, 0x01, 0xe2};
static const uint8_t lcdRegData5[]  = { 0x79, 0x79, 0xd8 };
static const uint8_t lcdRegData6[]  = { 0x00, 0x01, 0xb3 };
static const uint8_t lcdRegData7[]  = { 0x85, 0x01, 0x00, 0x84, 0x01, 0x00, 0xce };
static const uint8_t lcdRegData8[]  = { 0x18, 0x04, 0x03, 0x39, 0x00, 0x00, 0x00, 0x18, 0x03, 0x03, 0x3a, 0x00, 0x00, 0x00, 0xce };
static const uint8_t lcdRegData9[]  = { 0x18, 0x02, 0x03, 0x3b, 0x00, 0x00, 0x00, 0x18, 0x01, 0x03, 0x3c, 0x00, 0x00, 0x00, 0xce };
static const uint8_t lcdRegData10[] = { 0x01, 0x01, 0x20, 0x20, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0xcf};
static const uint8_t lcdRegData11[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcb};
static const uint8_t lcdRegData12[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcb };
static const uint8_t lcdRegData13[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcb };
static const uint8_t lcdRegData14[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcb };
static const uint8_t lcdRegData15[] = { 0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,0x00,0x00,0x00,0x00,0xcb };
static const uint8_t lcdRegData16[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x04, 0x04, 0x04,0x00,0x00,0x00,0x00,0xcb };
static const uint8_t lcdRegData17[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcb };
static const uint8_t lcdRegData18[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcb };
static const uint8_t lcdRegData19[] = { 0x00, 0x26, 0x09, 0x0B, 0x01, 0x25, 0x00, 0x00, 0x00, 0x00, 0xcc };
static const uint8_t lcdRegData20[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x0a, 0x0c, 0x02, 0xcc };
static const uint8_t lcdRegData21[] = { 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcc };
static const uint8_t lcdRegData22[] = { 0x00, 0x25, 0x0C, 0x0A, 0x02, 0x26, 0x00, 0x00, 0x00, 0x00, 0xcc };
static const uint8_t lcdRegData23[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x0b, 0x09, 0x01, 0xcc };
static const uint8_t lcdRegData24[] = { 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcc };
static const uint8_t lcdRegData25[] = { 0xff, 0xff, 0xff, 0xff };


static const uint8_t ShortRegData1[]  = { OTM8009A_CMD_NOP, 0x00 };
static const uint8_t ShortRegData2[]  = { OTM8009A_CMD_NOP, 0x80 };
static const uint8_t ShortRegData3[]  = { 0xc4, 0x30 };
static const uint8_t ShortRegData4[]  = { OTM8009A_CMD_NOP, 0x8a };
static const uint8_t ShortRegData5[]  = { 0xc4, 0x40 };
static const uint8_t ShortRegData6[]  = { OTM8009A_CMD_NOP, 0xb1 };
static const uint8_t ShortRegData7[]  = { 0xc5, 0xa9};
static const uint8_t ShortRegData8[]  = { OTM8009A_CMD_NOP, 0x91 };
static const uint8_t ShortRegData9[]  = { 0xc5, 0x34};
static const uint8_t ShortRegData10[] = { OTM8009A_CMD_NOP, 0xB4 };
static const uint8_t ShortRegData11[] = { 0xc0, 0x50 };
static const uint8_t ShortRegData12[] = { 0xd9, 0x4e };
static const uint8_t ShortRegData13[] = { OTM8009A_CMD_NOP, 0x81 };
static const uint8_t ShortRegData14[] = { 0xc1, 0x66 };
static const uint8_t ShortRegData15[] = { OTM8009A_CMD_NOP, 0xa1 };
static const uint8_t ShortRegData16[] = { 0xc1, 0x08 };
static const uint8_t ShortRegData17[] = { OTM8009A_CMD_NOP, 0x92 };
static const uint8_t ShortRegData18[] = { 0xc5, 0x01 };
static const uint8_t ShortRegData19[] = { OTM8009A_CMD_NOP, 0x95 };
static const uint8_t ShortRegData20[] = { OTM8009A_CMD_NOP, 0x94 };
static const uint8_t ShortRegData21[] = { 0xc5, 0x33};
static const uint8_t ShortRegData22[] = { OTM8009A_CMD_NOP, 0xa3 };
static const uint8_t ShortRegData23[] = { 0xc0, 0x1b };
static const uint8_t ShortRegData24[] = { OTM8009A_CMD_NOP, 0x82};
static const uint8_t ShortRegData25[] = { 0xc5, 0x83 };
static const uint8_t ShortRegData26[] = { 0xc4, 0x83 };
static const uint8_t ShortRegData27[] = { 0xc1, 0x0e };
static const uint8_t ShortRegData28[] = { OTM8009A_CMD_NOP, 0xa6 };
static const uint8_t ShortRegData29[] = { OTM8009A_CMD_NOP, 0xa0 };
static const uint8_t ShortRegData30[] = { OTM8009A_CMD_NOP, 0xb0 };
static const uint8_t ShortRegData31[] = { OTM8009A_CMD_NOP, 0xc0 };
static const uint8_t ShortRegData32[] = { OTM8009A_CMD_NOP, 0xd0 };
static const uint8_t ShortRegData33[] = { OTM8009A_CMD_NOP, 0x90 };
static const uint8_t ShortRegData34[] = { OTM8009A_CMD_NOP, 0xe0 };
static const uint8_t ShortRegData35[] = { OTM8009A_CMD_NOP, 0xf0 };
static const uint8_t ShortRegData36[] = { OTM8009A_CMD_SLPOUT, 0x00 };
static const uint8_t ShortRegData37[] = { OTM8009A_CMD_COLMOD, OTM8009A_COLMOD_RGB565 };
static const uint8_t ShortRegData38[] = { OTM8009A_CMD_COLMOD, OTM8009A_COLMOD_RGB888 };
static const uint8_t ShortRegData40[] = { OTM8009A_CMD_WRDISBV, 0x7f };
static const uint8_t ShortRegData41[] = { OTM8009A_CMD_WRCTRLD, 0x2c };
static const uint8_t ShortRegData42[] = { OTM8009A_CMD_WRCABC, 0x02 };
static const uint8_t ShortRegData43[] = { OTM8009A_CMD_WRCABCMB, 0xff };
static const uint8_t ShortRegData44[] = { OTM8009A_CMD_DISPON, 0x00 };
static const uint8_t ShortRegData45[] = { OTM8009A_CMD_RAMWR, 0x00 };
static const uint8_t ShortRegData46[] = { 0xcf, 0x00 };
static const uint8_t ShortRegData47[] = { 0xc5, 0x66 };
static const uint8_t ShortRegData48[] = { OTM8009A_CMD_NOP, 0xb6 };
static const uint8_t ShortRegData49[] = { 0xf5, 0x06 };
static const uint8_t ShortRegData50[] = { OTM8009A_CMD_NOP, 0xb1 };
static const uint8_t ShortRegData51[] = { 0xc6, 0x06 };


static LTDC_LayerCfgTypeDef layer1 = {
  .WindowX0         = L1_PADDING_LEFT,
  .WindowX1         = (L1_HEIGHT + L1_PADDING_RIGHT),
  .WindowY0         = L1_PADDING_BOTTOM,
  .WindowY1         = (L1_WIDTH + L1_PADDING_TOP),
  .PixelFormat      = LTDC_PIXEL_FORMAT_ARGB8888,
  .FBStartAdress    = L1_ADDR,
  .Alpha            = 125,
  .Alpha0           = 0,
  .BlendingFactor1  = LTDC_BLENDING_FACTOR1_PAxCA,
  .BlendingFactor2  = LTDC_BLENDING_FACTOR2_PAxCA,
  .ImageWidth       = L1_HEIGHT,
  .ImageHeight      = L1_WIDTH,
  .Backcolor.Blue   = 0x00,
  .Backcolor.Green  = 0x00,
  .Backcolor.Red    = 0x00,
};

static LTDC_LayerCfgTypeDef layer2 = {
  .WindowX0         = L2_PADDING_LEFT,
  .WindowX1         = (L2_HEIGHT + L2_PADDING_RIGHT),
  .WindowY0         = L2_PADDING_BOTTOM,
  .WindowY1         = (L2_WIDTH + L2_PADDING_TOP),
  .PixelFormat      = LTDC_PIXEL_FORMAT_ARGB8888,
  .FBStartAdress    = L2_ADDR,
  .Alpha            = 125,
  .Alpha0           = 0,
  .BlendingFactor1  = LTDC_BLENDING_FACTOR1_PAxCA,
  .BlendingFactor2  = LTDC_BLENDING_FACTOR2_PAxCA,
  .ImageWidth       = L2_HEIGHT,
  .ImageHeight      = L2_WIDTH,
  .Backcolor.Blue   = 0x00,
  .Backcolor.Green  = 0x00,
  .Backcolor.Red    = 0x00,
};





__STATIC_INLINE HAL_StatusTypeDef dsi_write(uint32_t, uint8_t*);









/**
  * @brief  Send the sequence of initialization command and data to display.
  * @retval None
  */
Display_TypeDef* OTM8009A_Init(void) {

  static Display_TypeDef display_0 = {
    .Model            = 8009,
    .Lock             = DISABLE,
    .DMADevHandler    = &hdma2d,
    .DSIDevHandler    = &hdsi,
    .SDRAMDevHandler  = &hsdram1,
    .LTDCDevHandler   = &hltdc,
    .Layer1           = &layer1,
    .Layer2           = &layer2,
    .BgLayer1         = (ARGB8888_Lightblue | 0xff000000),
    .BgLayer2         = (ARGB8888_Apple | 0xff000000),
    .Width            = DISPLAY_WIDTH,
    .Height           = DISPLAY_HEIGHT,
    .ColorSchema      = OTM8009A_FORMAT_RGB888,
  };

  Display_TypeDef* dev = &display_0;

  if (dev->Lock == DISABLE) dev->Lock = ENABLE;
  if (dev->DMADevHandler->Lock == HAL_LOCKED) return dev;
  if (dev->DSIDevHandler->Lock == HAL_LOCKED) return dev;
  if (dev->SDRAMDevHandler->Lock == HAL_LOCKED) return dev;


  /* Enable CMD2 to access vendor specific commands                               */
  /* Enter in command 2 mode and set EXTC to enable address shift function (0x00) */
  // dsi_write(0, (uint8_t*){ OTM8009A_CMD_NOP, 0x00 });
  if (dsi_write(0, (uint8_t *)ShortRegData1) != HAL_OK) return dev;
  // dsi_write( 3, (const uint8_t *){0x80, 0x09, 0x01, 0xff});
  if (dsi_write( 3, (uint8_t *)lcdRegData1) != HAL_OK) return dev;

  /* Enter ORISE Command 2 */
  if (dsi_write(0, (uint8_t *)ShortRegData2) != HAL_OK) return dev; /* Shift address to 0x80 */
  if (dsi_write( 2, (uint8_t *)lcdRegData2) != HAL_OK) return dev;

  /////////////////////////////////////////////////////////////////////
  /* SD_PCH_CTRL - 0xC480h - 129th parameter - Default 0x00          */
  /* Set SD_PT                                                       */
  /* -> Source output level during porch and non-display area to GND */
  if (dsi_write(0, (uint8_t *)ShortRegData2) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData3) != HAL_OK) return dev;
  HAL_Delay(10);
  /* Not documented */
  if (dsi_write(0, (uint8_t *)ShortRegData4) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData5) != HAL_OK) return dev;
  HAL_Delay(10);
  /////////////////////////////////////////////////////////////////////

  /* PWR_CTRL4 - 0xC4B0h - 178th parameter - Default 0xA8 */
  /* Set gvdd_en_test                                     */
  /* -> enable GVDD test mode !!!                         */
  if (dsi_write(0, (uint8_t *)ShortRegData6) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData7) != HAL_OK) return dev;

  /* PWR_CTRL2 - 0xC590h - 146th parameter - Default 0x79      */
  /* Set pump 4 vgh voltage                                    */
  /* -> from 15.0v down to 13.0v                               */
  /* Set pump 5 vgh voltage                                    */
  /* -> from -12.0v downto -9.0v                               */
  if (dsi_write(0, (uint8_t *)ShortRegData8) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData9) != HAL_OK) return dev;

  /* P_DRV_M - 0xC0B4h - 181th parameter - Default 0x00 */
  /* -> Column inversion                                */
  if (dsi_write(0, (uint8_t *)ShortRegData10) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData11) != HAL_OK) return dev;

  /* VCOMDC - 0xD900h - 1st parameter - Default 0x39h */
  /* VCOM Voltage settings                            */
  /* -> from -1.0000v downto -1.2625v                 */
  if (dsi_write(0, (uint8_t *)ShortRegData1) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData12) != HAL_OK) return dev;

  /* Oscillator adjustment for Idle/Normal mode (LPDT only) set to 65Hz (default is 60Hz) */
  if (dsi_write(0, (uint8_t *)ShortRegData13) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData14) != HAL_OK) return dev;

  /* Video mode internal */
  if (dsi_write(0, (uint8_t *)ShortRegData15) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData16) != HAL_OK) return dev;

  /* PWR_CTRL2 - 0xC590h - 147h parameter - Default 0x00 */
  /* Set pump 4&5 x6                                     */
  /* -> ONLY VALID when PUMP4_EN_ASDM_HV = "0"           */
  if (dsi_write(0, (uint8_t *)ShortRegData17) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData18) != HAL_OK) return dev;

  /* PWR_CTRL2 - 0xC590h - 150th parameter - Default 0x33h */
  /* Change pump4 clock ratio                              */
  /* -> from 1 line to 1/2 line                            */
  if (dsi_write(0, (uint8_t *)ShortRegData19) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData9) != HAL_OK) return dev;

  /* GVDD/NGVDD settings */
  if (dsi_write(0, (uint8_t *)ShortRegData1) != HAL_OK) return dev;
  if (dsi_write( 2, (uint8_t *)lcdRegData5) != HAL_OK) return dev;

  /* PWR_CTRL2 - 0xC590h - 149th parameter - Default 0x33h */
  /* Rewrite the default value !                           */
  if (dsi_write(0, (uint8_t *)ShortRegData20) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData21) != HAL_OK) return dev;

  /* Panel display timing Setting 3 */
  if (dsi_write(0, (uint8_t *)ShortRegData22) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData23) != HAL_OK) return dev;

  /* Power control 1 */
  if (dsi_write(0, (uint8_t *)ShortRegData24) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData25) != HAL_OK) return dev;

  /* Source driver precharge */
  if (dsi_write(0, (uint8_t *)ShortRegData13) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData26) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData15) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData27) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData28) != HAL_OK) return dev;
  if (dsi_write( 2, (uint8_t *)lcdRegData6) != HAL_OK) return dev;

  /* GOAVST */
  if (dsi_write(0, (uint8_t *)ShortRegData2) != HAL_OK) return dev;
  if (dsi_write( 6, (uint8_t *)lcdRegData7) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData29) != HAL_OK) return dev;
  if (dsi_write( 14, (uint8_t *)lcdRegData8) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData30) != HAL_OK) return dev;
  if (dsi_write( 14, (uint8_t *)lcdRegData9) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData31) != HAL_OK) return dev;
  if (dsi_write( 10, (uint8_t *)lcdRegData10) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData32) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData46) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData2) != HAL_OK) return dev;
  if (dsi_write( 10, (uint8_t *)lcdRegData11) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData33) != HAL_OK) return dev;
  if (dsi_write( 15, (uint8_t *)lcdRegData12) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData29) != HAL_OK) return dev;
  if (dsi_write( 15, (uint8_t *)lcdRegData13) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData30) != HAL_OK) return dev;
  if (dsi_write( 10, (uint8_t *)lcdRegData14) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData31) != HAL_OK) return dev;
  if (dsi_write( 15, (uint8_t *)lcdRegData15) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData32) != HAL_OK) return dev;
  if (dsi_write( 15, (uint8_t *)lcdRegData16) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData34) != HAL_OK) return dev;
  if (dsi_write( 10, (uint8_t *)lcdRegData17) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData35) != HAL_OK) return dev;
  if (dsi_write( 10, (uint8_t *)lcdRegData18) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData2) != HAL_OK) return dev;
  if (dsi_write( 10, (uint8_t *)lcdRegData19) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData33) != HAL_OK) return dev;
  if (dsi_write( 15, (uint8_t *)lcdRegData20) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData29) != HAL_OK) return dev;
  if (dsi_write( 15, (uint8_t *)lcdRegData21) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData30) != HAL_OK) return dev;
  if (dsi_write( 10, (uint8_t *)lcdRegData22) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData31) != HAL_OK) return dev;
  if (dsi_write( 15, (uint8_t *)lcdRegData23) != HAL_OK) return dev;

  if (dsi_write(0, (uint8_t *)ShortRegData32) != HAL_OK) return dev;
  if (dsi_write( 15, (uint8_t *)lcdRegData24) != HAL_OK) return dev;

  /////////////////////////////////////////////////////////////////////////////
  /* PWR_CTRL1 - 0xc580h - 130th parameter - default 0x00 */
  /* Pump 1 min and max DM                                */
  if (dsi_write(0, (uint8_t *)ShortRegData13) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData47) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData48) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData49) != HAL_OK) return dev;
  /////////////////////////////////////////////////////////////////////////////

  /* CABC LEDPWM frequency adjusted to 19,5kHz */
  if (dsi_write(0, (uint8_t *)ShortRegData50) != HAL_OK) return dev;
  if (dsi_write(0, (uint8_t *)ShortRegData51) != HAL_OK) return dev;

  /* Exit CMD2 mode */
  if (dsi_write(0, (uint8_t *)ShortRegData1) != HAL_OK) return dev;
  if (dsi_write( 3, (uint8_t *)lcdRegData25) != HAL_OK) return dev;

  /*************************************************************************** */
  /* Standard DCS Initialization TO KEEP CAN BE DONE IN HSDT                   */
  /*************************************************************************** */

  /* NOP - goes back to DCS std command ? */
  if (dsi_write(0, (uint8_t *)ShortRegData1) != HAL_OK) return dev;

  /* Gamma correction 2.2+ table (HSDT possible) */
  if (dsi_write(0, (uint8_t *)ShortRegData1) != HAL_OK) return dev;
  if (dsi_write( 16, (uint8_t *)lcdRegData3) != HAL_OK) return dev;

  /* Gamma correction 2.2- table (HSDT possible) */
  if (dsi_write(0, (uint8_t *)ShortRegData1) != HAL_OK) return dev;
  if (dsi_write( 16, (uint8_t *)lcdRegData4) != HAL_OK) return dev;

  /* Send Sleep Out command to display : no parameter */
  if (dsi_write(0, (uint8_t *)ShortRegData36) != HAL_OK) return dev;

  /* Wait for sleep out exit */
  HAL_Delay(120);

  switch(dev->ColorSchema) {
    case OTM8009A_FORMAT_RBG565 :
      /* Set Pixel color format to RGB565 */
      if (dsi_write(0, (uint8_t *)ShortRegData37) != HAL_OK) return dev;
      break;
    case OTM8009A_FORMAT_RGB888 :
      /* Set Pixel color format to RGB888 */
      if (dsi_write(0, (uint8_t *)ShortRegData38) != HAL_OK) return dev;
      break;
    default :
      break;
  }

  /* Send command to configure display in landscape orientation mode. By default the orientation mode is portrait  */
  // Vertical
  #ifndef _LANDSCAPE_
    static const uint8_t ShortRegData39[] = { OTM8009A_CMD_MADCTR, OTM8009A_MADCTR_MODE_LANDSCAPE };
    /*
      * CASET value (Column Address Set) : X direction LCD GRAM boundaries
      * depending on LCD orientation mode and PASET value (Page Address Set) : Y direction
      * LCD GRAM boundaries depending on LCD orientation mode
      * XS[15:0] = 0x000 = 0, XE[15:0] = 0x31f = 799 for landscape mode : apply to CASET
      * YS[15:0] = 0x000 = 0, YE[15:0] = 0x31f = 799 for portrait mode : : apply to PASET
      */
    static const uint8_t lcdRegData27[] = {0x00, 0x00, 0x03, 0x1f, OTM8009A_CMD_CASET};
    /*
      * XS[15:0] = 0x000 = 0, XE[15:0] = 0x1df = 479 for portrait mode : apply to CASET
      * YS[15:0] = 0x000 = 0, YE[15:0] = 0x1df = 479 for landscape mode : apply to PASET
    */
    static const uint8_t lcdRegData28[] = {0x00, 0x00, 0x01, 0xdf, OTM8009A_CMD_PASET};

    if (dsi_write(0, (uint8_t *)ShortRegData39) != HAL_OK) return dev;
    if (dsi_write(4, (uint8_t *)lcdRegData27) != HAL_OK) return dev;
    if (dsi_write(4, (uint8_t *)lcdRegData28) != HAL_OK) return dev;
  #endif

  /** CABC : Content Adaptive Backlight Control section start >> */
  /* Note : defaut is 0 (lowest Brightness), 0xFF is highest Brightness, try 0x7F : intermediate value */
  if (dsi_write(0, (uint8_t *)ShortRegData40) != HAL_OK) return dev;

  /* defaut is 0, try 0x2C - Brightness Control Block, Display Dimming & BackLight on */
  if (dsi_write(0, (uint8_t *)ShortRegData41) != HAL_OK) return dev;

  /* defaut is 0, try 0x02 - image Content based Adaptive Brightness [Still Picture] */
  if (dsi_write(0, (uint8_t *)ShortRegData42) != HAL_OK) return dev;

  /* defaut is 0 (lowest Brightness), 0xFF is highest Brightness */
  if (dsi_write(0, (uint8_t *)ShortRegData43) != HAL_OK) return dev;

  /** CABC : Content Adaptive Backlight Control section end << */

  /* Send Command Display On */
  if (dsi_write(0, (uint8_t *)ShortRegData44) != HAL_OK) return dev;

  /* NOP command */
  if (dsi_write(0, (uint8_t *)ShortRegData1) != HAL_OK) return dev;

  /* Send Command GRAM memory write (no parameters) : this initiates frame write via other DSI commands sent by */
  /* DSI host from LTDC incoming pixels in video mode */
  if (dsi_write(0, (uint8_t *)ShortRegData45) != HAL_OK) return dev;


  if (OTM8009A_DisplayOn(dev) != HAL_OK) return dev;

  /* TODO Implement PWM brightness */
  if (OTM8009A_SetBrightness(dev, 0xff) != HAL_OK) return dev;


  /* Configure layers */
  if (HAL_LTDC_ConfigLayer(dev->LTDCDevHandler, dev->Layer1, 0) != HAL_OK) return dev;
  if (HAL_LTDC_ConfigLayer(dev->LTDCDevHandler, dev->Layer2, 1) != HAL_OK) return dev;
  
  if (Display_FillLayer(dev, L1, dev->BgLayer1)) return dev;
  if (Display_FillLayer(dev, L2, dev->BgLayer2)) return dev;


  dev->Lock = DISABLE;

  return dev;
}



// --------------------------------------------------------------------------

__STATIC_INLINE HAL_StatusTypeDef dsi_write(uint32_t cnt, uint8_t* buf) {
	if(cnt <= 1) {
		if (HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, buf[0], buf[1]) != HAL_OK) return HAL_ERROR;
	} else {
		if (HAL_DSI_LongWrite(&hdsi, 0, DSI_DCS_LONG_PKT_WRITE, cnt, buf[cnt], buf) != HAL_OK) return HAL_ERROR;
	}
  return HAL_OK;
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef OTM8009A_DisplayOn(Display_TypeDef* dev) {
  if (HAL_DSI_ShortWrite(dev->DSIDevHandler, 0, DSI_DCS_SHORT_PKT_WRITE_P1, OTM8009A_CMD_DISPON, 0x00) != HAL_OK) return HAL_ERROR;
  return HAL_OK;
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef OTM8009A_SetBrightness(Display_TypeDef* dev, uint8_t val) {
  if (HAL_DSI_ShortWrite(dev->DSIDevHandler, 0, DSI_DCS_SHORT_PKT_WRITE_P1, OTM8009A_CMD_WRDISBV, val) != HAL_OK) return HAL_ERROR;
  return HAL_OK;
}




// --------------------------------------------------------------------------

/**
  * @brief  Fills a layer of display with color
  * @param  layer: number of layer
  * @param  color: color
  * @retval None
  */
HAL_StatusTypeDef __attribute__((weak)) Display_FillLayer(Display_TypeDef* dev, LTCDLayer_t layer, uint32_t color) {
  LTDC_LayerCfgTypeDef* l = (layer == L1) ? dev->Layer1 : dev->Layer2;
  if (Display_FillRectangle(dev, layer, 0, 0, l->ImageHeight, l->ImageWidth, color) != HAL_OK) return HAL_ERROR;
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
HAL_StatusTypeDef __attribute__((weak)) Display_DrawPixel(Display_TypeDef* dev, LTCDLayer_t layer, uint16_t x, uint16_t y, uint32_t color) {
  if (SDRAM_BusyStatusCheck(&hsdram1) != HAL_OK) return HAL_ERROR;
  LTDC_LayerCfgTypeDef* l = (layer == L1) ? dev->Layer1 : dev->Layer2;
  __O uint32_t* fb = (uint32_t*)(GET_POSITIOIN_ADDRESS(l, x, y));
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
HAL_StatusTypeDef __attribute__((weak)) Display_DrawVLine(Display_TypeDef* dev, LTCDLayer_t layer, uint16_t x, uint16_t y, uint16_t h, uint16_t t, uint32_t color) {
  if (Display_FillRectangle(dev, layer, x, y, t, h, color) != HAL_OK) return HAL_ERROR;
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
HAL_StatusTypeDef __attribute__((weak)) Display_DrawHLine(Display_TypeDef* dev, LTCDLayer_t layer, uint16_t x, uint16_t y, uint16_t w, uint16_t t, uint32_t color) {
  if (Display_FillRectangle(dev, layer, x, y, w, t, color) != HAL_OK) return HAL_ERROR;
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
HAL_StatusTypeDef __attribute__((weak)) Display_DrawRectangle(Display_TypeDef* dev, LTCDLayer_t layer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t t, uint32_t color) {
  if (Display_DrawHLine(dev, layer, x, y, w, t, color) != HAL_OK) return HAL_ERROR;
  if (Display_DrawHLine(dev, layer, x, (y + h), (w + t), t, color) != HAL_OK) return HAL_ERROR;
  if (Display_DrawVLine(dev, layer, x, y, h, t, color) != HAL_OK) return HAL_ERROR;
  if (Display_DrawVLine(dev, layer, (x + w), y, (h + t), t, color) != HAL_OK) return HAL_ERROR;
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
HAL_StatusTypeDef __attribute__((weak)) Display_FillRectangle(Display_TypeDef* dev, LTCDLayer_t layer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) {
  
  LTDC_LayerCfgTypeDef* l = (layer == L1) ? dev->Layer1 : dev->Layer2;

  if ((x + w - 1) >= l->ImageHeight) return HAL_ERROR;
  if ((y + h - 1) >= l->ImageWidth) return HAL_ERROR;

  for (uint32_t iw = 0; iw < w ; iw++) {

    if (SDRAM_BusyStatusCheck(dev->SDRAMDevHandler) != HAL_OK) return HAL_ERROR;

    __O uint32_t* fb = (uint32_t*)(GET_POSITIOIN_ADDRESS(l, (x + (iw * 1)), y)); 
    // __IO uint32_t* bb = (uint32_t*)(GET_BUF2_ADDRESS(l, (x + (iw * 1)), y)); 
    
    uint16_t hh = h;
    while (hh--) {
      // *bb++ = *fb;
      *fb++ = color;
    }
  }

  return HAL_OK;
}



// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_DrawCircle(Display_TypeDef* dev, LTCDLayer_t layer, uint16_t x0, uint16_t y0, uint16_t r, uint16_t t, uint32_t color) {
  int16_t x = 0;
  int16_t y = r;
  int16_t d = 1 - r;

  while (x <= y) {
    Display_DrawHLine(dev, layer, (x0 + x - t), (y0 + y), t, t, color);
    Display_DrawHLine(dev, layer, (x0 + x - t), (y0 - y), t, t, color);
    Display_DrawHLine(dev, layer, (x0 + y - t), (y0 + x), t, t, color);
    Display_DrawHLine(dev, layer, (x0 + y - t), (y0 - x), t, t, color);

    Display_DrawHLine(dev, layer, (x0 - x - t), (y0 + y), t, t, color);
    Display_DrawHLine(dev, layer, (x0 - x - t), (y0 - y), t, t, color);
    Display_DrawHLine(dev, layer, (x0 - y - t), (y0 + x), t, t, color);
    Display_DrawHLine(dev, layer, (x0 - y - t), (y0 - x), t, t, color);

    if (d < 0) {
      d += 2 * x + 3;
    } else {
      d += 2 * (x - y) + 5;
      y--;
    }
    x++;
  }

  return HAL_OK;
}




// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_FillCircle(Display_TypeDef* dev, LTCDLayer_t layer, uint16_t x0, uint16_t y0, uint16_t r, uint32_t color) {
  int16_t x = 0;
  int16_t y = r;
  int16_t d = 1 - r;

  while (x <= y) {

    Display_DrawHLine(dev, layer, (x0 - x - 1), (y0 + y), (2 * x + 1), 1, color);
    Display_DrawHLine(dev, layer, (x0 - x - 1), (y0 - y), (2 * x + 1), 1, color);
    Display_DrawHLine(dev, layer, (x0 - y - 1), (y0 + x), (2 * y + 1), 1, color);
    Display_DrawHLine(dev, layer, (x0 - y - 1), (y0 - x), (2 * y + 1), 1, color);

    if (d < 0) {
      d += 2 * x + 3;
    } else {
      d += 2 * (x - y) + 5;
      y--;
    }
    x++;
  }

  return HAL_OK;
}




// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_DrawSymbol(Display_TypeDef* dev, LTCDLayer_t layer, uint16_t* x, uint16_t* y, const Font_TypeDef *f, uint8_t ch) {
  if ((ch > 126) || (ch < 32)) {
    if (ch == 176) ch = 95;
    else return HAL_ERROR;
  } else {
    ch -= 32;
  }

  LTDC_LayerCfgTypeDef* l = (layer == L1) ? dev->Layer1 : dev->Layer2;
  
  if ((*x + f->Width - 1) >= l->ImageHeight) return HAL_ERROR;
  if ((*y + f->Height - 1) >= l->ImageWidth) return HAL_ERROR;
  
  const uint8_t *glyph = f->Font + (ch * f->BytesPerGlif);
  
  uint32_t pixel_count = 0;
  
  for (uint32_t iw = 0; iw < f->Width ; iw++) {
    
    if (SDRAM_BusyStatusCheck(dev->SDRAMDevHandler) != HAL_OK) return HAL_ERROR;
    __O uint32_t* fb = (uint32_t*)(GET_POSITIOIN_ADDRESS(l, (*x + (iw * 1)), *y));
    
    for (uint16_t ih = 0; ih < f->Height; ih += 8) {
      uint8_t bits = glyph[pixel_count++];
      
      for (uint8_t bit = 0; bit < 8; bit++) {
        *fb++ = (bits & 0x01) ? f->Color : f->Bgcolor;
        bits >>= 1;
      }
    }
  }
  *x += f->Width;
  
  return HAL_OK;
}





// --------------------------------------------------------------------------

HAL_StatusTypeDef __attribute__((weak)) Display_PrintString(Display_TypeDef* dev, LTCDLayer_t layer, uint16_t *x, uint16_t *y, const Font_TypeDef *f, const char *str, bool wrap) {
  
  uint16_t char_count = 0;
  
  while (str[char_count++] != '\n') {
    if (char_count > 64) break;
  }
  
  char_count--;
  
  while (!vblank_ready) __WFI();
  vblank_ready = 0;

  LTDC_LayerCfgTypeDef* l = (layer == L1) ? dev->Layer1 : dev->Layer2;
  
  SCB_CleanDCache_by_Addr((uint32_t*)l->FBStartAdress, l->ImageWidth * l->ImageHeight * 4);
  
  for (uint16_t ic = 0; ic < char_count; ic++) {
    Display_DrawSymbol(dev, layer, x, y, f, str[ic]);
  }
  
  return HAL_OK;
}



// --------------------------------------------------------------------------

void HAL_LTDC_LineEventCallback(LTDC_HandleTypeDef *hltdc) {
  if (hltdc->Instance == LTDC) {
    /* We are now in VBlank */
    vblank_ready = 1;
    /* Re-arm the interrupt */
    #ifdef _PORTRAIT_
      HAL_LTDC_ProgramLineEvent(hltdc, _HSA_ + _HBP_ + DISPLAY_WIDTH - 1);
    #endif
    #ifdef _LANDSCAPE_
      HAL_LTDC_ProgramLineEvent(hltdc, _VSA_ + _VBP_ + DISPLAY_HEIGHT - 1);
    #endif
  }
}
