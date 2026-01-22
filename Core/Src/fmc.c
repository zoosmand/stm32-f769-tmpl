/**
  ******************************************************************************
  * File Name          : fmc.c
  * Description        : This file provides code for the configuration
  *                      of the FMC instances.
  ******************************************************************************
  * @attention
  *
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "fmc.h"

/* Global variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/







////////////////////////////////////////////////////////////////////////////////


HAL_StatusTypeDef SDRAM_BusyStatusCheck(SDRAM_HandleTypeDef* sdram) {
  uint32_t threshold = HAL_GetTick() + SDRAM_BUSY_STATUS_TIMEOUT;

  while (PREG_CHECK(sdram->Instance->SDSR, FMC_SDSR_BUSY_Pos)) {
    if (HAL_GetTick() > threshold) return HAL_ERROR;
  };

  return HAL_OK;
}



/**
  * @brief  Writes a byte into SDRAM's certain address
  * @param  addr: SDRAM address
  * @param  data: a byte   
  * @return none
  */
HAL_StatusTypeDef SDRAM_Write8b(SDRAM_HandleTypeDef* sdram, uint32_t addr, uint8_t data) {

  if (SDRAM_BusyStatusCheck(sdram) != HAL_OK) return HAL_ERROR; 

  *(__IO uint8_t*) (hdma2d.Instance->OMAR + addr) = data;
  return HAL_OK;
}




/**
  * @brief  Reads a byte from SDRAM's certain address
  * @param  addr: SDRAM address
  * @return a byte
  */
uint8_t SDRAM_Read8b(SDRAM_HandleTypeDef* sdram, uint32_t addr) {
  SDRAM_BusyStatusCheck(sdram);
  return (*(__IO uint8_t*)(hdma2d.Instance->OMAR + addr));
}




/**
  * @brief  Writes a halfword into SDRAM's certain address
  * @param  addr: SDRAM address
  * @param  data: a halfword
  * @return none
  */
HAL_StatusTypeDef SDRAM_Write16b(SDRAM_HandleTypeDef* sdram, uint32_t addr, uint16_t data) {
  if (SDRAM_BusyStatusCheck(sdram) != HAL_OK) return HAL_ERROR;
  *(__IO uint16_t*) (hdma2d.Instance->OMAR + addr) = data;
  return HAL_OK;
}




/**
  * @brief  Reads a halfword from SDRAM's certain address
  * @param  addr: SDRAM address
  * @return a halfword
  */
uint16_t SDRAM_Read16b(SDRAM_HandleTypeDef* sdram, uint32_t addr) {
  SDRAM_BusyStatusCheck(sdram);
  return (*(__IO uint16_t*)(hdma2d.Instance->OMAR + addr));
}




/**
  * @brief  Writes a word into SDRAM's certain address
  * @param  addr: SDRAM address
  * @param  data: a word
  * @return none
  */
HAL_StatusTypeDef SDRAM_Write32b(SDRAM_HandleTypeDef* sdram, uint32_t addr, uint32_t data) {
  if (SDRAM_BusyStatusCheck(sdram) != HAL_OK) return HAL_ERROR;
  *(__IO uint32_t*)(hdma2d.Instance->OMAR + addr) = data;
  return HAL_OK;
}




/**
  * @brief  Reads a word from SDRAM's certain address
  * @param  addr SDRAM address
  * @return a word
  */
uint32_t SDRAM_Read32b(SDRAM_HandleTypeDef* sdram, uint32_t addr) {
  SDRAM_BusyStatusCheck(sdram);
  return (*(__IO uint32_t*)(hdma2d.Instance->OMAR + addr));
}




/**
  * @brief  Reads a buffer from SDRAM's certain address
  * @param  addr SDRAM address
  * @param  cnt count of bytes
  * @param  buf pointer of a buffer
  * @return status of reading
  */
HAL_StatusTypeDef SDRAM_ReadBuffer8b(SDRAM_HandleTypeDef* sdram, uint32_t addr, uint32_t cnt, uint8_t *buf) {
  if ((addr + cnt) > SDRAM_MAX_ADDR_Offset) return HAL_ERROR;
  uint32_t ptr = addr;

  if (SDRAM_BusyStatusCheck(sdram) != HAL_OK) return HAL_ERROR;

  while (cnt--) {
    *buf++ = *(__IO uint8_t*)(hdma2d.Instance->OMAR + ptr);
    ptr++;
  }
  return HAL_OK;
}




/**
  * @brief  Writes a buffer into SDRAM's certain address
  * @param  addr SDRAM address
  * @param  cnt count of bytes
  * @param  buf pointer of a buffer
  * @return status of writing
  */
HAL_StatusTypeDef SDRAM_WriteBuffer8b(SDRAM_HandleTypeDef* sdram, uint32_t addr, uint32_t cnt, uint8_t *buf) {
  if ((addr + cnt) > SDRAM_MAX_ADDR_Offset) return HAL_ERROR;
  uint32_t ptr = addr;

  if (SDRAM_BusyStatusCheck(sdram) != HAL_OK) return HAL_ERROR;

  while (cnt--) {
    *(__IO uint8_t*)(hdma2d.Instance->OMAR + ptr) = *buf++;
    ptr++;
  }
  return HAL_OK;
}




/**
  * @brief  Reads a buffer from SDRAM's certain address
  * @param  addr SDRAM address
  * @param  cnt count of words
  * @param  buf pointer of a buffer
  * @return status of reading
  */
HAL_StatusTypeDef SDRAM_ReadBuffer(SDRAM_HandleTypeDef* sdram, uint32_t addr, uint32_t cnt, uint32_t *buf) {
  if ((addr + cnt) > SDRAM_MAX_ADDR_Offset) return HAL_ERROR;
  uint32_t ptr = addr;

  if (SDRAM_BusyStatusCheck(sdram) != HAL_OK) return HAL_ERROR;

  while (cnt--) {
    *buf++ = *(__IO uint32_t*)(hdma2d.Instance->OMAR + ptr);
    ptr += 4;
  }
  return HAL_OK;
}




/**
  * @brief  Writes a buffer into SDRAM's certain address
  * @param  addr SDRAM address
  * @param  cnt count of words
  * @param  buf pointer of a buffer
  * @return status of writing
  */
HAL_StatusTypeDef SDRAM_WriteBuffer(SDRAM_HandleTypeDef* sdram, uint32_t addr, uint32_t cnt, uint32_t *buf) {
  if ((addr + cnt) > SDRAM_MAX_ADDR_Offset) return HAL_ERROR;
  uint32_t ptr = addr;

  if (SDRAM_BusyStatusCheck(sdram) != HAL_OK) return HAL_ERROR;

  while (cnt--) {
    *(__IO uint32_t*)(hdma2d.Instance->OMAR + ptr) = *buf++;
    ptr += 4;
  }
  return HAL_OK;
}
