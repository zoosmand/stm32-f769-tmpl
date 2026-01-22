/**
  ******************************************************************************
  * File Name          : OTM8009A.c
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





/*
 * @brief Constant tables of register settings used to transmit DSI
 * command packets as power up initialization sequence of the KoD LCD (OTM8009A LCD Driver)
 */
const uint8_t lcdRegData1[]  = { 0x80, 0x09, 0x01, 0xff };
const uint8_t lcdRegData2[]  = { 0x80, 0x09, 0xff };
const uint8_t lcdRegData3[]  = { 0x00, 0x09, 0x0f, 0x0e, 0x07, 0x10, 0x0b, 0x0a, 0x04, 0x07, 0x0b, 0x08, 0x0f, 0x10, 0x0a, 0x01, 0xe1};
const uint8_t lcdRegData4[]  = { 0x00, 0x09, 0x0f, 0x0e, 0x07, 0x10, 0x0b, 0x0a, 0x04, 0x07, 0x0b, 0x08, 0x0f, 0x10, 0x0a, 0x01, 0xe2};
const uint8_t lcdRegData5[]  = { 0x79, 0x79, 0xd8 };
const uint8_t lcdRegData6[]  = { 0x00, 0x01, 0xb3 };
const uint8_t lcdRegData7[]  = { 0x85, 0x01, 0x00, 0x84, 0x01, 0x00, 0xce };
const uint8_t lcdRegData8[]  = { 0x18, 0x04, 0x03, 0x39, 0x00, 0x00, 0x00, 0x18, 0x03, 0x03, 0x3a, 0x00, 0x00, 0x00, 0xce };
const uint8_t lcdRegData9[]  = { 0x18, 0x02, 0x03, 0x3b, 0x00, 0x00, 0x00, 0x18, 0x01, 0x03, 0x3c, 0x00, 0x00, 0x00, 0xce };
const uint8_t lcdRegData10[] = { 0x01, 0x01, 0x20, 0x20, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00, 0xcf};
const uint8_t lcdRegData11[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcb};
const uint8_t lcdRegData12[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcb };
const uint8_t lcdRegData13[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcb };
const uint8_t lcdRegData14[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcb };
const uint8_t lcdRegData15[] = { 0x00, 0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00,0x00,0x00,0x00,0x00,0xcb };
const uint8_t lcdRegData16[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04, 0x04, 0x04, 0x04,0x00,0x00,0x00,0x00,0xcb };
const uint8_t lcdRegData17[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcb };
const uint8_t lcdRegData18[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xcb };
const uint8_t lcdRegData19[] = { 0x00, 0x26, 0x09, 0x0B, 0x01, 0x25, 0x00, 0x00, 0x00, 0x00, 0xcc };
const uint8_t lcdRegData20[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x26, 0x0a, 0x0c, 0x02, 0xcc };
const uint8_t lcdRegData21[] = { 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcc };
const uint8_t lcdRegData22[] = { 0x00, 0x25, 0x0C, 0x0A, 0x02, 0x26, 0x00, 0x00, 0x00, 0x00, 0xcc };
const uint8_t lcdRegData23[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x25, 0x0b, 0x09, 0x01, 0xcc };
const uint8_t lcdRegData24[] = { 0x26, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xcc };
const uint8_t lcdRegData25[] = { 0xff, 0xff, 0xff, 0xff };
/*
  * CASET value (Column Address Set) : X direction LCD GRAM boundaries
  * depending on LCD orientation mode and PASET value (Page Address Set) : Y direction
  * LCD GRAM boundaries depending on LCD orientation mode
  * XS[15:0] = 0x000 = 0, XE[15:0] = 0x31F = 799 for landscape mode : apply to CASET
  * YS[15:0] = 0x000 = 0, YE[15:0] = 0x31F = 799 for portrait mode : : apply to PASET
  */
const uint8_t lcdRegData27[] = {0x00, 0x00, 0x03, 0x1F, OTM8009A_CMD_CASET};
/*
  * XS[15:0] = 0x000 = 0, XE[15:0] = 0x1DF = 479 for portrait mode : apply to CASET
  * YS[15:0] = 0x000 = 0, YE[15:0] = 0x1DF = 479 for landscape mode : apply to PASET
 */
const uint8_t lcdRegData28[] = {0x00, 0x00, 0x01, 0xDF, OTM8009A_CMD_PASET};

// #ifdef _LANDSCAPE_
//   const uint8_t lcdRegData27[] = {0x00, 0x00, 0x03, 0x1F, OTM8009A_CMD_CASET};
//   const uint8_t lcdRegData28[] = {0x00, 0x00, 0x01, 0xDF, OTM8009A_CMD_PASET};
// #else
//   const uint8_t lcdRegData27[] = {0x00, 0x00, 0x01, 0xdf, OTM8009A_CMD_CASET};
//   const uint8_t lcdRegData28[] = {0x00, 0x00, 0x03, 0x1f, OTM8009A_CMD_PASET};
// #endif

const uint8_t ShortRegData1[]  = { OTM8009A_CMD_NOP, 0x00 };
const uint8_t ShortRegData2[]  = { OTM8009A_CMD_NOP, 0x80 };
const uint8_t ShortRegData3[]  = { 0xc4, 0x30 };
const uint8_t ShortRegData4[]  = { OTM8009A_CMD_NOP, 0x8a };
const uint8_t ShortRegData5[]  = { 0xc4, 0x40 };
const uint8_t ShortRegData6[]  = { OTM8009A_CMD_NOP, 0xb1 };
const uint8_t ShortRegData7[]  = { 0xc5, 0xA9};
const uint8_t ShortRegData8[]  = { OTM8009A_CMD_NOP, 0x91 };
const uint8_t ShortRegData9[]  = { 0xc5, 0x34};
const uint8_t ShortRegData10[] = { OTM8009A_CMD_NOP, 0xB4 };
const uint8_t ShortRegData11[] = { 0xc0, 0x50 };
const uint8_t ShortRegData12[] = { 0xd9, 0x4e };
const uint8_t ShortRegData13[] = { OTM8009A_CMD_NOP, 0x81 };
const uint8_t ShortRegData14[] = { 0xc1, 0x66 };
const uint8_t ShortRegData15[] = { OTM8009A_CMD_NOP, 0xa1 };
const uint8_t ShortRegData16[] = { 0xc1, 0x08 };
const uint8_t ShortRegData17[] = { OTM8009A_CMD_NOP, 0x92 };
const uint8_t ShortRegData18[] = { 0xc5, 0x01 };
const uint8_t ShortRegData19[] = { OTM8009A_CMD_NOP, 0x95 };
const uint8_t ShortRegData20[] = { OTM8009A_CMD_NOP, 0x94 };
const uint8_t ShortRegData21[] = { 0xc5, 0x33};
const uint8_t ShortRegData22[] = { OTM8009A_CMD_NOP, 0xa3 };
const uint8_t ShortRegData23[] = { 0xc0, 0x1b };
const uint8_t ShortRegData24[] = { OTM8009A_CMD_NOP, 0x82};
const uint8_t ShortRegData25[] = { 0xc5, 0x83 };
const uint8_t ShortRegData26[] = { 0xc4, 0x83 };
const uint8_t ShortRegData27[] = { 0xc1, 0x0e };
const uint8_t ShortRegData28[] = { OTM8009A_CMD_NOP, 0xa6 };
const uint8_t ShortRegData29[] = { OTM8009A_CMD_NOP, 0xa0 };
const uint8_t ShortRegData30[] = { OTM8009A_CMD_NOP, 0xb0 };
const uint8_t ShortRegData31[] = { OTM8009A_CMD_NOP, 0xc0 };
const uint8_t ShortRegData32[] = { OTM8009A_CMD_NOP, 0xd0 };
const uint8_t ShortRegData33[] = { OTM8009A_CMD_NOP, 0x90 };
const uint8_t ShortRegData34[] = { OTM8009A_CMD_NOP, 0xe0 };
const uint8_t ShortRegData35[] = { OTM8009A_CMD_NOP, 0xf0 };
const uint8_t ShortRegData36[] = { OTM8009A_CMD_SLPOUT, 0x00 };
const uint8_t ShortRegData37[] = { OTM8009A_CMD_COLMOD, OTM8009A_COLMOD_RGB565 };
const uint8_t ShortRegData38[] = { OTM8009A_CMD_COLMOD, OTM8009A_COLMOD_RGB888 };
const uint8_t ShortRegData39[] = { OTM8009A_CMD_MADCTR, OTM8009A_MADCTR_MODE_LANDSCAPE };
const uint8_t ShortRegData40[] = { OTM8009A_CMD_WRDISBV, 0x7f };
const uint8_t ShortRegData41[] = { OTM8009A_CMD_WRCTRLD, 0x2c };
const uint8_t ShortRegData42[] = { OTM8009A_CMD_WRCABC, 0x02 };
const uint8_t ShortRegData43[] = { OTM8009A_CMD_WRCABCMB, 0xff };
const uint8_t ShortRegData44[] = { OTM8009A_CMD_DISPON, 0x00 };
const uint8_t ShortRegData45[] = { OTM8009A_CMD_RAMWR, 0x00 };
const uint8_t ShortRegData46[] = { 0xcf, 0x00 };
const uint8_t ShortRegData47[] = { 0xc5, 0x66 };
const uint8_t ShortRegData48[] = { OTM8009A_CMD_NOP, 0xb6 };
const uint8_t ShortRegData49[] = { 0xf5, 0x06 };
const uint8_t ShortRegData50[] = { OTM8009A_CMD_NOP, 0xb1 };
const uint8_t ShortRegData51[] = { 0xc6, 0x06 };







static void DSI_Write(uint32_t cnt, uint8_t *buf) {
	if(cnt <= 1) {
		HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, buf[0], buf[1]);
	} else {
		HAL_DSI_LongWrite(&hdsi, 0, DSI_DCS_LONG_PKT_WRITE, cnt, buf[cnt], buf);
	}
}









/**
  * @brief  Send the sequence of initialization command and data to display.
  * @retval None
  */
void OTM8009A_Init(uint32_t colorSchema) {
  	// uint32_t orientation = OTM8009A_ORIENTATION_LANDSCAPE;
  /* Enable CMD2 to access vendor specific commands                               */
  /* Enter in command 2 mode and set EXTC to enable address shift function (0x00) */
  // DSI_Write(0, (uint8_t*){ OTM8009A_CMD_NOP, 0x00 });
  DSI_Write(0, (uint8_t *)ShortRegData1);
  // DSI_Write( 3, (const uint8_t *){0x80, 0x09, 0x01, 0xff});
  DSI_Write( 3, (uint8_t *)lcdRegData1);

  /* Enter ORISE Command 2 */
  DSI_Write(0, (uint8_t *)ShortRegData2); /* Shift address to 0x80 */
  DSI_Write( 2, (uint8_t *)lcdRegData2);

  /////////////////////////////////////////////////////////////////////
  /* SD_PCH_CTRL - 0xC480h - 129th parameter - Default 0x00          */
  /* Set SD_PT                                                       */
  /* -> Source output level during porch and non-display area to GND */
  DSI_Write(0, (uint8_t *)ShortRegData2);
  DSI_Write(0, (uint8_t *)ShortRegData3);
  HAL_Delay(10);
  /* Not documented */
  DSI_Write(0, (uint8_t *)ShortRegData4);
  DSI_Write(0, (uint8_t *)ShortRegData5);
  HAL_Delay(10);
  /////////////////////////////////////////////////////////////////////

  /* PWR_CTRL4 - 0xC4B0h - 178th parameter - Default 0xA8 */
  /* Set gvdd_en_test                                     */
  /* -> enable GVDD test mode !!!                         */
  DSI_Write(0, (uint8_t *)ShortRegData6);
  DSI_Write(0, (uint8_t *)ShortRegData7);

  /* PWR_CTRL2 - 0xC590h - 146th parameter - Default 0x79      */
  /* Set pump 4 vgh voltage                                    */
  /* -> from 15.0v down to 13.0v                               */
  /* Set pump 5 vgh voltage                                    */
  /* -> from -12.0v downto -9.0v                               */
  DSI_Write(0, (uint8_t *)ShortRegData8);
  DSI_Write(0, (uint8_t *)ShortRegData9);

  /* P_DRV_M - 0xC0B4h - 181th parameter - Default 0x00 */
  /* -> Column inversion                                */
  DSI_Write(0, (uint8_t *)ShortRegData10);
  DSI_Write(0, (uint8_t *)ShortRegData11);

  /* VCOMDC - 0xD900h - 1st parameter - Default 0x39h */
  /* VCOM Voltage settings                            */
  /* -> from -1.0000v downto -1.2625v                 */
  DSI_Write(0, (uint8_t *)ShortRegData1);
  DSI_Write(0, (uint8_t *)ShortRegData12);

  /* Oscillator adjustment for Idle/Normal mode (LPDT only) set to 65Hz (default is 60Hz) */
  DSI_Write(0, (uint8_t *)ShortRegData13);
  DSI_Write(0, (uint8_t *)ShortRegData14);

  /* Video mode internal */
  DSI_Write(0, (uint8_t *)ShortRegData15);
  DSI_Write(0, (uint8_t *)ShortRegData16);

  /* PWR_CTRL2 - 0xC590h - 147h parameter - Default 0x00 */
  /* Set pump 4&5 x6                                     */
  /* -> ONLY VALID when PUMP4_EN_ASDM_HV = "0"           */
  DSI_Write(0, (uint8_t *)ShortRegData17);
  DSI_Write(0, (uint8_t *)ShortRegData18);

  /* PWR_CTRL2 - 0xC590h - 150th parameter - Default 0x33h */
  /* Change pump4 clock ratio                              */
  /* -> from 1 line to 1/2 line                            */
  DSI_Write(0, (uint8_t *)ShortRegData19);
  DSI_Write(0, (uint8_t *)ShortRegData9);

  /* GVDD/NGVDD settings */
  DSI_Write(0, (uint8_t *)ShortRegData1);
  DSI_Write( 2, (uint8_t *)lcdRegData5);

  /* PWR_CTRL2 - 0xC590h - 149th parameter - Default 0x33h */
  /* Rewrite the default value !                           */
  DSI_Write(0, (uint8_t *)ShortRegData20);
  DSI_Write(0, (uint8_t *)ShortRegData21);

  /* Panel display timing Setting 3 */
  DSI_Write(0, (uint8_t *)ShortRegData22);
  DSI_Write(0, (uint8_t *)ShortRegData23);

  /* Power control 1 */
  DSI_Write(0, (uint8_t *)ShortRegData24);
  DSI_Write(0, (uint8_t *)ShortRegData25);

  /* Source driver precharge */
  DSI_Write(0, (uint8_t *)ShortRegData13);
  DSI_Write(0, (uint8_t *)ShortRegData26);

  DSI_Write(0, (uint8_t *)ShortRegData15);
  DSI_Write(0, (uint8_t *)ShortRegData27);

  DSI_Write(0, (uint8_t *)ShortRegData28);
  DSI_Write( 2, (uint8_t *)lcdRegData6);

  /* GOAVST */
  DSI_Write(0, (uint8_t *)ShortRegData2);
  DSI_Write( 6, (uint8_t *)lcdRegData7);

  DSI_Write(0, (uint8_t *)ShortRegData29);
  DSI_Write( 14, (uint8_t *)lcdRegData8);

  DSI_Write(0, (uint8_t *)ShortRegData30);
  DSI_Write( 14, (uint8_t *)lcdRegData9);

  DSI_Write(0, (uint8_t *)ShortRegData31);
  DSI_Write( 10, (uint8_t *)lcdRegData10);

  DSI_Write(0, (uint8_t *)ShortRegData32);
  DSI_Write(0, (uint8_t *)ShortRegData46);

  DSI_Write(0, (uint8_t *)ShortRegData2);
  DSI_Write( 10, (uint8_t *)lcdRegData11);

  DSI_Write(0, (uint8_t *)ShortRegData33);
  DSI_Write( 15, (uint8_t *)lcdRegData12);

  DSI_Write(0, (uint8_t *)ShortRegData29);
  DSI_Write( 15, (uint8_t *)lcdRegData13);

  DSI_Write(0, (uint8_t *)ShortRegData30);
  DSI_Write( 10, (uint8_t *)lcdRegData14);

  DSI_Write(0, (uint8_t *)ShortRegData31);
  DSI_Write( 15, (uint8_t *)lcdRegData15);

  DSI_Write(0, (uint8_t *)ShortRegData32);
  DSI_Write( 15, (uint8_t *)lcdRegData16);

  DSI_Write(0, (uint8_t *)ShortRegData34);
  DSI_Write( 10, (uint8_t *)lcdRegData17);

  DSI_Write(0, (uint8_t *)ShortRegData35);
  DSI_Write( 10, (uint8_t *)lcdRegData18);

  DSI_Write(0, (uint8_t *)ShortRegData2);
  DSI_Write( 10, (uint8_t *)lcdRegData19);

  DSI_Write(0, (uint8_t *)ShortRegData33);
  DSI_Write( 15, (uint8_t *)lcdRegData20);

  DSI_Write(0, (uint8_t *)ShortRegData29);
  DSI_Write( 15, (uint8_t *)lcdRegData21);

  DSI_Write(0, (uint8_t *)ShortRegData30);
  DSI_Write( 10, (uint8_t *)lcdRegData22);

  DSI_Write(0, (uint8_t *)ShortRegData31);
  DSI_Write( 15, (uint8_t *)lcdRegData23);

  DSI_Write(0, (uint8_t *)ShortRegData32);
  DSI_Write( 15, (uint8_t *)lcdRegData24);

  /////////////////////////////////////////////////////////////////////////////
  /* PWR_CTRL1 - 0xc580h - 130th parameter - default 0x00 */
  /* Pump 1 min and max DM                                */
  DSI_Write(0, (uint8_t *)ShortRegData13);
  DSI_Write(0, (uint8_t *)ShortRegData47);
  DSI_Write(0, (uint8_t *)ShortRegData48);
  DSI_Write(0, (uint8_t *)ShortRegData49);
  /////////////////////////////////////////////////////////////////////////////

  /* CABC LEDPWM frequency adjusted to 19,5kHz */
  DSI_Write(0, (uint8_t *)ShortRegData50);
  DSI_Write(0, (uint8_t *)ShortRegData51);

  /* Exit CMD2 mode */
  DSI_Write(0, (uint8_t *)ShortRegData1);
  DSI_Write( 3, (uint8_t *)lcdRegData25);

  /*************************************************************************** */
  /* Standard DCS Initialization TO KEEP CAN BE DONE IN HSDT                   */
  /*************************************************************************** */

  /* NOP - goes back to DCS std command ? */
  DSI_Write(0, (uint8_t *)ShortRegData1);

  /* Gamma correction 2.2+ table (HSDT possible) */
  DSI_Write(0, (uint8_t *)ShortRegData1);
  DSI_Write( 16, (uint8_t *)lcdRegData3);

  /* Gamma correction 2.2- table (HSDT possible) */
  DSI_Write(0, (uint8_t *)ShortRegData1);
  DSI_Write( 16, (uint8_t *)lcdRegData4);

  /* Send Sleep Out command to display : no parameter */
  DSI_Write(0, (uint8_t *)ShortRegData36);

  /* Wait for sleep out exit */
  HAL_Delay(120);

  switch(colorSchema)
  {
  case OTM8009A_FORMAT_RBG565 :
    /* Set Pixel color format to RGB565 */
    DSI_Write(0, (uint8_t *)ShortRegData37);
    break;
  case OTM8009A_FORMAT_RGB888 :
    /* Set Pixel color format to RGB888 */
    DSI_Write(0, (uint8_t *)ShortRegData38);
    break;
  default :
    break;
  }

  /* Send command to configure display in landscape orientation mode. By default
      the orientation mode is portrait  */
  // if(orientation == OTM8009A_ORIENTATION_LANDSCAPE)
  // {
    // Vertical
  #ifndef _LANDSCAPE_
    DSI_Write(0, (uint8_t *)ShortRegData39);
    DSI_Write(4, (uint8_t *)lcdRegData27);
    DSI_Write(4, (uint8_t *)lcdRegData28);
  #endif
    // DSI_Write(0, (uint8_t *){OTM8009A_CMD_MADCTR, 0xf0});
    // DSI_Write( 4, (uint8_t *)lcdRegData27);
    // DSI_Write( 4, (uint8_t *)lcdRegData28);
  // }

  /** CABC : Content Adaptive Backlight Control section start >> */
  /* Note : defaut is 0 (lowest Brightness), 0xFF is highest Brightness, try 0x7F : intermediate value */
  DSI_Write(0, (uint8_t *)ShortRegData40);

  /* defaut is 0, try 0x2C - Brightness Control Block, Display Dimming & BackLight on */
  DSI_Write(0, (uint8_t *)ShortRegData41);

  /* defaut is 0, try 0x02 - image Content based Adaptive Brightness [Still Picture] */
  DSI_Write(0, (uint8_t *)ShortRegData42);

  /* defaut is 0 (lowest Brightness), 0xFF is highest Brightness */
  DSI_Write(0, (uint8_t *)ShortRegData43);

  /** CABC : Content Adaptive Backlight Control section end << */

  /* Send Command Display On */
  DSI_Write(0, (uint8_t *)ShortRegData44);

  /* NOP command */
  DSI_Write(0, (uint8_t *)ShortRegData1);

  /* Send Command GRAM memory write (no parameters) : this initiates frame write via other DSI commands sent by */
  /* DSI host from LTDC incoming pixels in video mode */
  DSI_Write(0, (uint8_t *)ShortRegData45);


  OTM8009A_DisplayOn();
  OTM8009A_SetBrightness(0xff);

}



void OTM8009A_DisplayOn(void) {
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, OTM8009A_CMD_DISPON, 0x00);
}



void OTM8009A_SetBrightness(uint8_t val) {
  HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, OTM8009A_CMD_WRDISBV, val);
}
