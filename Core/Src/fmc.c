#include "fmc.h"

/** Return true when an offset/length pair lies inside the 16 MiB SDRAM. */
static bool sdram_IsRangeValid(uint32_t offset, uint32_t lengthBytes)
{
  return (offset <= SDRAM_SIZE_BYTES) &&
         (lengthBytes <= (SDRAM_SIZE_BYTES - offset));
}

/** Convert a byte offset from SDRAM_START_ADDR to a volatile address. */
static uintptr_t sdram_Address(uint32_t offset)
{
  return (uintptr_t)SDRAM_START_ADDR + offset;
}

HAL_StatusTypeDef SDRAM_BusyStatusCheck(SDRAM_HandleTypeDef *sdram)
{
  if ((sdram == NULL) || (sdram->Instance == NULL))
  {
    return HAL_ERROR;
  }

  const uint32_t startedAt = HAL_GetTick();
  while ((sdram->Instance->SDSR & FMC_SDSR_BUSY) != 0U)
  {
    if ((HAL_GetTick() - startedAt) >= SDRAM_BUSY_STATUS_TIMEOUT_MS)
    {
      return HAL_TIMEOUT;
    }
  }

  return HAL_OK;
}

HAL_StatusTypeDef SDRAM_Write8b(SDRAM_HandleTypeDef *sdram, uint32_t offset,
                                uint8_t data)
{
  if (!sdram_IsRangeValid(offset, sizeof(data)) ||
      (SDRAM_BusyStatusCheck(sdram) != HAL_OK))
  {
    return HAL_ERROR;
  }

  *(__IO uint8_t *)sdram_Address(offset) = data;
  return HAL_OK;
}

uint8_t SDRAM_Read8b(SDRAM_HandleTypeDef *sdram, uint32_t offset)
{
  if (!sdram_IsRangeValid(offset, sizeof(uint8_t)) ||
      (SDRAM_BusyStatusCheck(sdram) != HAL_OK))
  {
    return 0U;
  }

  return *(__IO uint8_t *)sdram_Address(offset);
}

HAL_StatusTypeDef SDRAM_Write16b(SDRAM_HandleTypeDef *sdram, uint32_t offset,
                                 uint16_t data)
{
  if (!sdram_IsRangeValid(offset, sizeof(data)) ||
      ((offset % sizeof(data)) != 0U) ||
      (SDRAM_BusyStatusCheck(sdram) != HAL_OK))
  {
    return HAL_ERROR;
  }

  *(__IO uint16_t *)sdram_Address(offset) = data;
  return HAL_OK;
}

uint16_t SDRAM_Read16b(SDRAM_HandleTypeDef *sdram, uint32_t offset)
{
  if (!sdram_IsRangeValid(offset, sizeof(uint16_t)) ||
      ((offset % sizeof(uint16_t)) != 0U) ||
      (SDRAM_BusyStatusCheck(sdram) != HAL_OK))
  {
    return 0U;
  }

  return *(__IO uint16_t *)sdram_Address(offset);
}

HAL_StatusTypeDef SDRAM_Write32b(SDRAM_HandleTypeDef *sdram, uint32_t offset,
                                 uint32_t data)
{
  if (!sdram_IsRangeValid(offset, sizeof(data)) ||
      ((offset % sizeof(data)) != 0U) ||
      (SDRAM_BusyStatusCheck(sdram) != HAL_OK))
  {
    return HAL_ERROR;
  }

  *(__IO uint32_t *)sdram_Address(offset) = data;
  return HAL_OK;
}

uint32_t SDRAM_Read32b(SDRAM_HandleTypeDef *sdram, uint32_t offset)
{
  if (!sdram_IsRangeValid(offset, sizeof(uint32_t)) ||
      ((offset % sizeof(uint32_t)) != 0U) ||
      (SDRAM_BusyStatusCheck(sdram) != HAL_OK))
  {
    return 0U;
  }

  return *(__IO uint32_t *)sdram_Address(offset);
}

HAL_StatusTypeDef SDRAM_ReadBuffer8b(SDRAM_HandleTypeDef *sdram,
                                     uint32_t offset, uint32_t count,
                                     uint8_t *buffer)
{
  if ((buffer == NULL) || !sdram_IsRangeValid(offset, count) ||
      (SDRAM_BusyStatusCheck(sdram) != HAL_OK))
  {
    return HAL_ERROR;
  }

  const __IO uint8_t *source = (const __IO uint8_t *)sdram_Address(offset);
  while (count-- > 0U)
  {
    *buffer++ = *source++;
  }
  return HAL_OK;
}

HAL_StatusTypeDef SDRAM_WriteBuffer8b(SDRAM_HandleTypeDef *sdram,
                                      uint32_t offset, uint32_t count,
                                      const uint8_t *buffer)
{
  if ((buffer == NULL) || !sdram_IsRangeValid(offset, count) ||
      (SDRAM_BusyStatusCheck(sdram) != HAL_OK))
  {
    return HAL_ERROR;
  }

  __IO uint8_t *destination = (__IO uint8_t *)sdram_Address(offset);
  while (count-- > 0U)
  {
    *destination++ = *buffer++;
  }
  return HAL_OK;
}

HAL_StatusTypeDef SDRAM_ReadBuffer(SDRAM_HandleTypeDef *sdram,
                                   uint32_t offset, uint32_t count,
                                   uint32_t *buffer)
{
  if ((count > (UINT32_MAX / sizeof(uint32_t))) ||
      (buffer == NULL) || ((offset % sizeof(uint32_t)) != 0U) ||
      !sdram_IsRangeValid(offset, count * sizeof(uint32_t)) ||
      (SDRAM_BusyStatusCheck(sdram) != HAL_OK))
  {
    return HAL_ERROR;
  }

  const __IO uint32_t *source = (const __IO uint32_t *)sdram_Address(offset);
  while (count-- > 0U)
  {
    *buffer++ = *source++;
  }
  return HAL_OK;
}

HAL_StatusTypeDef SDRAM_WriteBuffer(SDRAM_HandleTypeDef *sdram,
                                    uint32_t offset, uint32_t count,
                                    const uint32_t *buffer)
{
  if ((count > (UINT32_MAX / sizeof(uint32_t))) ||
      (buffer == NULL) || ((offset % sizeof(uint32_t)) != 0U) ||
      !sdram_IsRangeValid(offset, count * sizeof(uint32_t)) ||
      (SDRAM_BusyStatusCheck(sdram) != HAL_OK))
  {
    return HAL_ERROR;
  }

  __IO uint32_t *destination = (__IO uint32_t *)sdram_Address(offset);
  while (count-- > 0U)
  {
    *destination++ = *buffer++;
  }
  return HAL_OK;
}
