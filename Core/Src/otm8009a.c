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







static HAL_StatusTypeDef DSI_Write(uint32_t cnt, uint8_t *buf) {
	if(cnt <= 1) {
		if (HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, buf[0], buf[1]) != HAL_OK) return HAL_ERROR;
	} else {
		if (HAL_DSI_LongWrite(&hdsi, 0, DSI_DCS_LONG_PKT_WRITE, cnt, buf[cnt], buf) != HAL_OK) return HAL_ERROR;
	}
  return HAL_OK;
}









/**
  * @brief  Send the sequence of initialization command and data to display.
  * @retval None
  */
HAL_StatusTypeDef OTM8009A_Init(uint32_t colorSchema) {
  	// uint32_t orientation = OTM8009A_ORIENTATION_LANDSCAPE;
  /* Enable CMD2 to access vendor specific commands                               */
  /* Enter in command 2 mode and set EXTC to enable address shift function (0x00) */
  // DSI_Write(0, (uint8_t*){ OTM8009A_CMD_NOP, 0x00 });
  if (DSI_Write(0, (uint8_t *)ShortRegData1) != HAL_OK) return HAL_ERROR;
  // DSI_Write( 3, (const uint8_t *){0x80, 0x09, 0x01, 0xff});
  if (DSI_Write( 3, (uint8_t *)lcdRegData1) != HAL_OK) return HAL_ERROR;

  /* Enter ORISE Command 2 */
  if (DSI_Write(0, (uint8_t *)ShortRegData2) != HAL_OK) return HAL_ERROR; /* Shift address to 0x80 */
  if (DSI_Write( 2, (uint8_t *)lcdRegData2) != HAL_OK) return HAL_ERROR;

  /////////////////////////////////////////////////////////////////////
  /* SD_PCH_CTRL - 0xC480h - 129th parameter - Default 0x00          */
  /* Set SD_PT                                                       */
  /* -> Source output level during porch and non-display area to GND */
  if (DSI_Write(0, (uint8_t *)ShortRegData2) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData3) != HAL_OK) return HAL_ERROR;
  HAL_Delay(10);
  /* Not documented */
  if (DSI_Write(0, (uint8_t *)ShortRegData4) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData5) != HAL_OK) return HAL_ERROR;
  HAL_Delay(10);
  /////////////////////////////////////////////////////////////////////

  /* PWR_CTRL4 - 0xC4B0h - 178th parameter - Default 0xA8 */
  /* Set gvdd_en_test                                     */
  /* -> enable GVDD test mode !!!                         */
  if (DSI_Write(0, (uint8_t *)ShortRegData6) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData7) != HAL_OK) return HAL_ERROR;

  /* PWR_CTRL2 - 0xC590h - 146th parameter - Default 0x79      */
  /* Set pump 4 vgh voltage                                    */
  /* -> from 15.0v down to 13.0v                               */
  /* Set pump 5 vgh voltage                                    */
  /* -> from -12.0v downto -9.0v                               */
  if (DSI_Write(0, (uint8_t *)ShortRegData8) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData9) != HAL_OK) return HAL_ERROR;

  /* P_DRV_M - 0xC0B4h - 181th parameter - Default 0x00 */
  /* -> Column inversion                                */
  if (DSI_Write(0, (uint8_t *)ShortRegData10) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData11) != HAL_OK) return HAL_ERROR;

  /* VCOMDC - 0xD900h - 1st parameter - Default 0x39h */
  /* VCOM Voltage settings                            */
  /* -> from -1.0000v downto -1.2625v                 */
  if (DSI_Write(0, (uint8_t *)ShortRegData1) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData12) != HAL_OK) return HAL_ERROR;

  /* Oscillator adjustment for Idle/Normal mode (LPDT only) set to 65Hz (default is 60Hz) */
  if (DSI_Write(0, (uint8_t *)ShortRegData13) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData14) != HAL_OK) return HAL_ERROR;

  /* Video mode internal */
  if (DSI_Write(0, (uint8_t *)ShortRegData15) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData16) != HAL_OK) return HAL_ERROR;

  /* PWR_CTRL2 - 0xC590h - 147h parameter - Default 0x00 */
  /* Set pump 4&5 x6                                     */
  /* -> ONLY VALID when PUMP4_EN_ASDM_HV = "0"           */
  if (DSI_Write(0, (uint8_t *)ShortRegData17) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData18) != HAL_OK) return HAL_ERROR;

  /* PWR_CTRL2 - 0xC590h - 150th parameter - Default 0x33h */
  /* Change pump4 clock ratio                              */
  /* -> from 1 line to 1/2 line                            */
  if (DSI_Write(0, (uint8_t *)ShortRegData19) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData9) != HAL_OK) return HAL_ERROR;

  /* GVDD/NGVDD settings */
  if (DSI_Write(0, (uint8_t *)ShortRegData1) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 2, (uint8_t *)lcdRegData5) != HAL_OK) return HAL_ERROR;

  /* PWR_CTRL2 - 0xC590h - 149th parameter - Default 0x33h */
  /* Rewrite the default value !                           */
  if (DSI_Write(0, (uint8_t *)ShortRegData20) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData21) != HAL_OK) return HAL_ERROR;

  /* Panel display timing Setting 3 */
  if (DSI_Write(0, (uint8_t *)ShortRegData22) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData23) != HAL_OK) return HAL_ERROR;

  /* Power control 1 */
  if (DSI_Write(0, (uint8_t *)ShortRegData24) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData25) != HAL_OK) return HAL_ERROR;

  /* Source driver precharge */
  if (DSI_Write(0, (uint8_t *)ShortRegData13) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData26) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData15) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData27) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData28) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 2, (uint8_t *)lcdRegData6) != HAL_OK) return HAL_ERROR;

  /* GOAVST */
  if (DSI_Write(0, (uint8_t *)ShortRegData2) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 6, (uint8_t *)lcdRegData7) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData29) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 14, (uint8_t *)lcdRegData8) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData30) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 14, (uint8_t *)lcdRegData9) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData31) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 10, (uint8_t *)lcdRegData10) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData32) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData46) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData2) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 10, (uint8_t *)lcdRegData11) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData33) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 15, (uint8_t *)lcdRegData12) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData29) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 15, (uint8_t *)lcdRegData13) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData30) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 10, (uint8_t *)lcdRegData14) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData31) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 15, (uint8_t *)lcdRegData15) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData32) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 15, (uint8_t *)lcdRegData16) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData34) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 10, (uint8_t *)lcdRegData17) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData35) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 10, (uint8_t *)lcdRegData18) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData2) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 10, (uint8_t *)lcdRegData19) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData33) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 15, (uint8_t *)lcdRegData20) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData29) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 15, (uint8_t *)lcdRegData21) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData30) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 10, (uint8_t *)lcdRegData22) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData31) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 15, (uint8_t *)lcdRegData23) != HAL_OK) return HAL_ERROR;

  if (DSI_Write(0, (uint8_t *)ShortRegData32) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 15, (uint8_t *)lcdRegData24) != HAL_OK) return HAL_ERROR;

  /////////////////////////////////////////////////////////////////////////////
  /* PWR_CTRL1 - 0xc580h - 130th parameter - default 0x00 */
  /* Pump 1 min and max DM                                */
  if (DSI_Write(0, (uint8_t *)ShortRegData13) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData47) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData48) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData49) != HAL_OK) return HAL_ERROR;
  /////////////////////////////////////////////////////////////////////////////

  /* CABC LEDPWM frequency adjusted to 19,5kHz */
  if (DSI_Write(0, (uint8_t *)ShortRegData50) != HAL_OK) return HAL_ERROR;
  if (DSI_Write(0, (uint8_t *)ShortRegData51) != HAL_OK) return HAL_ERROR;

  /* Exit CMD2 mode */
  if (DSI_Write(0, (uint8_t *)ShortRegData1) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 3, (uint8_t *)lcdRegData25) != HAL_OK) return HAL_ERROR;

  /*************************************************************************** */
  /* Standard DCS Initialization TO KEEP CAN BE DONE IN HSDT                   */
  /*************************************************************************** */

  /* NOP - goes back to DCS std command ? */
  if (DSI_Write(0, (uint8_t *)ShortRegData1) != HAL_OK) return HAL_ERROR;

  /* Gamma correction 2.2+ table (HSDT possible) */
  if (DSI_Write(0, (uint8_t *)ShortRegData1) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 16, (uint8_t *)lcdRegData3) != HAL_OK) return HAL_ERROR;

  /* Gamma correction 2.2- table (HSDT possible) */
  if (DSI_Write(0, (uint8_t *)ShortRegData1) != HAL_OK) return HAL_ERROR;
  if (DSI_Write( 16, (uint8_t *)lcdRegData4) != HAL_OK) return HAL_ERROR;

  /* Send Sleep Out command to display : no parameter */
  if (DSI_Write(0, (uint8_t *)ShortRegData36) != HAL_OK) return HAL_ERROR;

  /* Wait for sleep out exit */
  HAL_Delay(120);

  switch(colorSchema)
  {
  case OTM8009A_FORMAT_RBG565 :
    /* Set Pixel color format to RGB565 */
    if (DSI_Write(0, (uint8_t *)ShortRegData37) != HAL_OK) return HAL_ERROR;
    break;
  case OTM8009A_FORMAT_RGB888 :
    /* Set Pixel color format to RGB888 */
    if (DSI_Write(0, (uint8_t *)ShortRegData38) != HAL_OK) return HAL_ERROR;
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
    if (DSI_Write(0, (uint8_t *)ShortRegData39) != HAL_OK) return HAL_ERROR;
    if (DSI_Write(4, (uint8_t *)lcdRegData27) != HAL_OK) return HAL_ERROR;
    if (DSI_Write(4, (uint8_t *)lcdRegData28) != HAL_OK) return HAL_ERROR;
  #endif
    // if (DSI_Write(0, (uint8_t *){OTM8009A_CMD_MADCTR, 0xf0}) != HAL_OK) return HAL_ERROR;
    // if (DSI_Write( 4, (uint8_t *)lcdRegData27) != HAL_OK) return HAL_ERROR;
    // if (DSI_Write( 4, (uint8_t *)lcdRegData28) != HAL_OK) return HAL_ERROR;
  // }

  /** CABC : Content Adaptive Backlight Control section start >> */
  /* Note : defaut is 0 (lowest Brightness), 0xFF is highest Brightness, try 0x7F : intermediate value */
  if (DSI_Write(0, (uint8_t *)ShortRegData40) != HAL_OK) return HAL_ERROR;

  /* defaut is 0, try 0x2C - Brightness Control Block, Display Dimming & BackLight on */
  if (DSI_Write(0, (uint8_t *)ShortRegData41) != HAL_OK) return HAL_ERROR;

  /* defaut is 0, try 0x02 - image Content based Adaptive Brightness [Still Picture] */
  if (DSI_Write(0, (uint8_t *)ShortRegData42) != HAL_OK) return HAL_ERROR;

  /* defaut is 0 (lowest Brightness), 0xFF is highest Brightness */
  if (DSI_Write(0, (uint8_t *)ShortRegData43) != HAL_OK) return HAL_ERROR;

  /** CABC : Content Adaptive Backlight Control section end << */

  /* Send Command Display On */
  if (DSI_Write(0, (uint8_t *)ShortRegData44) != HAL_OK) return HAL_ERROR;

  /* NOP command */
  if (DSI_Write(0, (uint8_t *)ShortRegData1) != HAL_OK) return HAL_ERROR;

  /* Send Command GRAM memory write (no parameters) : this initiates frame write via other DSI commands sent by */
  /* DSI host from LTDC incoming pixels in video mode */
  if (DSI_Write(0, (uint8_t *)ShortRegData45) != HAL_OK) return HAL_ERROR;


  if (OTM8009A_DisplayOn() != HAL_OK) return HAL_ERROR;
  if (OTM8009A_SetBrightness(0xff) != HAL_OK) return HAL_ERROR;


  return HAL_OK;
}



HAL_StatusTypeDef OTM8009A_DisplayOn(void) {
  if (HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, OTM8009A_CMD_DISPON, 0x00) != HAL_OK) return HAL_ERROR;
  return HAL_OK;
}



HAL_StatusTypeDef OTM8009A_SetBrightness(uint8_t val) {
  if (HAL_DSI_ShortWrite(&hdsi, 0, DSI_DCS_SHORT_PKT_WRITE_P1, OTM8009A_CMD_WRDISBV, val) != HAL_OK) return HAL_ERROR;
  return HAL_OK;
}







/**
  * @brief  Fills a layer of display with color
  * @param  layer: number of layer
  * @param  color: color
  * @retval None
  */
HAL_StatusTypeDef __attribute__((weak)) Display_FillLayer(LTDC_LayerCfgTypeDef* layer, uint32_t color) {
  if (Display_FillRectangle(layer, 0, 0, layer->ImageHeight, layer->ImageWidth, color) != HAL_OK) return HAL_ERROR;
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
HAL_StatusTypeDef __attribute__((weak)) Display_DrawPixel(LTDC_LayerCfgTypeDef* layer, uint16_t x, uint16_t y, uint32_t color) {
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
HAL_StatusTypeDef __attribute__((weak)) Display_DrawVLine(LTDC_LayerCfgTypeDef* layer, uint16_t x, uint16_t y, uint16_t h, uint16_t t, uint32_t color) {
  if (Display_FillRectangle(layer, x, y, t, h, color) != HAL_OK) return HAL_ERROR;
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
HAL_StatusTypeDef __attribute__((weak)) Display_DrawHLine(LTDC_LayerCfgTypeDef* layer, uint16_t x, uint16_t y, uint16_t w, uint16_t t, uint32_t color) {
  if (Display_FillRectangle(layer, x, y, w, t, color) != HAL_OK) return HAL_ERROR;
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
HAL_StatusTypeDef __attribute__((weak)) Display_DrawRectangle(LTDC_LayerCfgTypeDef* layer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t t, uint32_t color) {
  if (Display_DrawHLine(layer, x, y, w, t, color) != HAL_OK) return HAL_ERROR;
  if (Display_DrawHLine(layer, x, (y + h), (w + t), t, color) != HAL_OK) return HAL_ERROR;
  if (Display_DrawVLine(layer, x, y, h, t, color) != HAL_OK) return HAL_ERROR;
  if (Display_DrawVLine(layer, (x + w), y, (h + t), t, color) != HAL_OK) return HAL_ERROR;
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
HAL_StatusTypeDef __attribute__((weak)) Display_FillRectangle(LTDC_LayerCfgTypeDef* layer, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint32_t color) {
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
HAL_StatusTypeDef __attribute__((weak)) Display_DrawSymbol(LTDC_LayerCfgTypeDef* layer, uint16_t* x, uint16_t* y, const Font_TypeDef *font, uint8_t ch) {
  if ((ch > 126) || (ch < 32)) {
    if (ch == 176) ch = 95;
    else return HAL_ERROR;
  } else {
    ch -= 32;
  }


  return HAL_OK;
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
HAL_StatusTypeDef __attribute__((weak)) Display_PrintString(LTDC_LayerCfgTypeDef* layer, uint16_t *x, uint16_t *y, const Font_TypeDef *font, const char *buf, uint8_t wrap) {

  return HAL_OK;
}

