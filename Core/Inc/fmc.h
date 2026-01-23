/**
  ******************************************************************************
  * File Name          : fmc.h
  * Description        : This file provides code for the configuration
  *                      of the FMC instances.
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FMC_H
#define __FMC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"


/* Global variables ----------------------------------------------------------*/

extern DMA2D_HandleTypeDef hdma2d;

/* Private defines -----------------------------------------------------------*/

#define SDRAM_START_ADDR                          (uint32_t)0xc0000000 // SDRAM Bank 1 adress
// #define SDRAM_START_ADDR                          (uint32_t)0xd0000000 // SDRAM Bank 2 address
#define SDRAM_MAX_ADDR                            (uint32_t)0xc0ffffff // 16MB
#define SDRAM_MAX_ADDR_Offset                     (uint32_t)(0x01000000 - 1) // 16MB

#define SDRAM_MODEREG_BURST_LENGTH_1              (uint16_t)0x0000
#define SDRAM_MODEREG_BURST_LENGTH_2              (uint16_t)0x0001
#define SDRAM_MODEREG_BURST_LENGTH_4              (uint16_t)0x0002
#define SDRAM_MODEREG_BURST_LENGTH_8              (uint16_t)0x0004
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL       (uint16_t)0x0000
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED      (uint16_t)0x0008
#define SDRAM_MODEREG_CAS_LATENCY_1               (uint16_t)0x0000
#define SDRAM_MODEREG_CAS_LATENCY_2               (uint16_t)0x0020
#define SDRAM_MODEREG_CAS_LATENCY_3               (uint16_t)0x0030
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD     (uint16_t)0x0000
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED  (uint16_t)0x0000
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE      (uint16_t)0x0200

#define SDRAM_BANK1                               0
#define SDRAM_BANK2                               1


#define SDRAM_CMD_NORMAL_MODE                     0x00000000
#define SDRAM_CMD_CLK_ENABLE                      0x00000001
#define SDRAM_CMD_PALL                            0x00000002
#define SDRAM_CMD_AUTOREFRESH_MODE                0x00000003
#define SDRAM_CMD_LOAD_MODE                       0x00000004
#define SDRAM_CMD_SELFREFRESH_MODE                0x00000004
#define SDRAM_CMD_POWERDOWN_MODE                  0x00000006

#define SDRAM_CMD_TARGET_BANK2                    FMC_SDCMR_CTB2
#define SDRAM_CMD_TARGET_BANK1                    FMC_SDCMR_CTB1
#define SDRAM_CMD_TARGET_BANK1_2                  (FMC_SDCMR_CTB2 | FMC_SDCMR_CTB1)

#define SDRAM_NORMAL_MODE                         0x00000000
#define SDRAM_SELF_REFRESH_MODE                   FMC_SDSR_MODES1_0
#define SDRAM_POWER_DOWN_MODE                     FMC_SDSR_MODES1_1




/* Exported macro ------------------------------------------------------------*/
#define SDRAM_BUSY_STATUS_TIMEOUT                 1000U


/* Exported functions prototypes ---------------------------------------------*/

HAL_StatusTypeDef SDRAM_Write8b(SDRAM_HandleTypeDef*, uint32_t, uint8_t);

uint8_t SDRAM_Read8b(SDRAM_HandleTypeDef*, uint32_t);

HAL_StatusTypeDef SDRAM_Write16b(SDRAM_HandleTypeDef*, uint32_t, uint16_t);

uint16_t SDRAM_Read16b(SDRAM_HandleTypeDef*, uint32_t);

HAL_StatusTypeDef SDRAM_Write32b(SDRAM_HandleTypeDef*, uint32_t, uint32_t);

uint32_t SDRAM_Read32b(SDRAM_HandleTypeDef*, uint32_t);

HAL_StatusTypeDef SDRAM_ReadBuffer8b(SDRAM_HandleTypeDef*, uint32_t, uint32_t, uint8_t*);

HAL_StatusTypeDef SDRAM_WriteBuffer8b(SDRAM_HandleTypeDef*, uint32_t, uint32_t, uint8_t*);

HAL_StatusTypeDef SDRAM_ReadBuffer(SDRAM_HandleTypeDef*, uint32_t, uint32_t, uint32_t*);

HAL_StatusTypeDef SDRAM_WriteBuffer(SDRAM_HandleTypeDef*, uint32_t, uint32_t, uint32_t*);

HAL_StatusTypeDef SDRAM_BusyStatusCheck(SDRAM_HandleTypeDef*);




#ifdef __cplusplus
}
#endif
#endif /*__FMC_H */
