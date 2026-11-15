#ifndef FMC_H
#define FMC_H

#include "common.h"

#define SDRAM_START_ADDR                         UINT32_C(0xC0000000)
#define SDRAM_SIZE_BYTES                         UINT32_C(0x01000000)

#define SDRAM_MODEREG_BURST_LENGTH_1             UINT16_C(0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             UINT16_C(0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             UINT16_C(0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             UINT16_C(0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      UINT16_C(0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     UINT16_C(0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_1              UINT16_C(0x0000)
#define SDRAM_MODEREG_CAS_LATENCY_2              UINT16_C(0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              UINT16_C(0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    UINT16_C(0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED UINT16_C(0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     UINT16_C(0x0200)

#define SDRAM_BANK1                              0U
#define SDRAM_BANK2                              1U

#define SDRAM_CMD_NORMAL_MODE                    UINT32_C(0x00000000)
#define SDRAM_CMD_CLK_ENABLE                     UINT32_C(0x00000001)
#define SDRAM_CMD_PALL                           UINT32_C(0x00000002)
#define SDRAM_CMD_AUTOREFRESH_MODE               UINT32_C(0x00000003)
#define SDRAM_CMD_LOAD_MODE                      UINT32_C(0x00000004)
#define SDRAM_CMD_SELFREFRESH_MODE               UINT32_C(0x00000004)
#define SDRAM_CMD_POWERDOWN_MODE                 UINT32_C(0x00000006)

#define SDRAM_CMD_TARGET_BANK2                   FMC_SDCMR_CTB2
#define SDRAM_CMD_TARGET_BANK1                   FMC_SDCMR_CTB1
#define SDRAM_CMD_TARGET_BANK1_2                 (FMC_SDCMR_CTB2 | FMC_SDCMR_CTB1)

#define SDRAM_NORMAL_MODE                        UINT32_C(0x00000000)
#define SDRAM_SELF_REFRESH_MODE                  FMC_SDSR_MODES1_0
#define SDRAM_POWER_DOWN_MODE                    FMC_SDSR_MODES1_1

#define SDRAM_BUSY_STATUS_TIMEOUT_MS             1000U

/**
  * @brief Wait until the FMC SDRAM controller is no longer busy.
  * @param sdram Non-null initialized SDRAM handle.
  * @retval HAL_OK when ready, HAL_TIMEOUT on expiry, or HAL_ERROR for bad input.
  * @note This polling function blocks for at most SDRAM_BUSY_STATUS_TIMEOUT_MS.
  */
HAL_StatusTypeDef SDRAM_BusyStatusCheck(SDRAM_HandleTypeDef *sdram);

/** @brief Write one byte at a byte offset from SDRAM_START_ADDR. */
HAL_StatusTypeDef SDRAM_Write8b(SDRAM_HandleTypeDef *sdram, uint32_t offset,
                                uint8_t data);
/** @brief Read one byte at a byte offset; returns zero on invalid input/error. */
uint8_t SDRAM_Read8b(SDRAM_HandleTypeDef *sdram, uint32_t offset);
/** @brief Write one aligned halfword at a byte offset from SDRAM_START_ADDR. */
HAL_StatusTypeDef SDRAM_Write16b(SDRAM_HandleTypeDef *sdram, uint32_t offset,
                                 uint16_t data);
/** @brief Read one aligned halfword; returns zero on invalid input/error. */
uint16_t SDRAM_Read16b(SDRAM_HandleTypeDef *sdram, uint32_t offset);
/** @brief Write one aligned word at a byte offset from SDRAM_START_ADDR. */
HAL_StatusTypeDef SDRAM_Write32b(SDRAM_HandleTypeDef *sdram, uint32_t offset,
                                 uint32_t data);
/** @brief Read one aligned word; returns zero on invalid input/error. */
uint32_t SDRAM_Read32b(SDRAM_HandleTypeDef *sdram, uint32_t offset);

/** @brief Copy count bytes from SDRAM into non-null buffer. */
HAL_StatusTypeDef SDRAM_ReadBuffer8b(SDRAM_HandleTypeDef *sdram,
                                     uint32_t offset, uint32_t count,
                                     uint8_t *buffer);
/** @brief Copy count bytes from non-null buffer into SDRAM. */
HAL_StatusTypeDef SDRAM_WriteBuffer8b(SDRAM_HandleTypeDef *sdram,
                                      uint32_t offset, uint32_t count,
                                      const uint8_t *buffer);
/** @brief Copy count aligned 32-bit words from SDRAM into non-null buffer. */
HAL_StatusTypeDef SDRAM_ReadBuffer(SDRAM_HandleTypeDef *sdram,
                                   uint32_t offset, uint32_t count,
                                   uint32_t *buffer);
/** @brief Copy count aligned 32-bit words from non-null buffer into SDRAM. */
HAL_StatusTypeDef SDRAM_WriteBuffer(SDRAM_HandleTypeDef *sdram,
                                    uint32_t offset, uint32_t count,
                                    const uint32_t *buffer);

#endif /* FMC_H */
