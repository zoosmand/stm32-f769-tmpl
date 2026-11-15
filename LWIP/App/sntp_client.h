#ifndef SNTP_CLIENT_H
#define SNTP_CLIENT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "lwip/ip4_addr.h"
#include "stm32f7xx_hal.h"

/** Maximum NTP servers accepted from DHCP option 42. */
#define SNTP_MAX_SERVERS          2U

/** Synchronization state of the SNTP client. */
typedef enum
{
  SNTP_STATE_NO_SERVER,       /**< DHCP has not offered an NTP server. */
  SNTP_STATE_WAITING_NETWORK, /**< Server known; waiting for link and address. */
  SNTP_STATE_REQUESTING,      /**< Request sent; waiting for a reply. */
  SNTP_STATE_SYNCHRONIZED,    /**< Last exchange succeeded. */
  SNTP_STATE_RETRYING,        /**< Last exchange failed; retry scheduled. */
} Sntp_StateTypeDef;

/** Snapshot of the SNTP client and the software UTC clock. */
typedef struct
{
  Sntp_StateTypeDef State;            /**< Current state. */
  uint8_t ServerCount;                /**< Servers offered by DHCP (0..2). */
  ip4_addr_t Servers[SNTP_MAX_SERVERS]; /**< Offered servers, DHCP order. */
  bool Synchronized;                  /**< The clock has been set at least once. */
  ip4_addr_t SyncServer;              /**< Server of the last good reply. */
  uint8_t Stratum;                    /**< Stratum of the last good reply. */
  uint32_t RoundTripMs;               /**< Network round trip of the last reply. */
  int32_t LastStepMs;                 /**< Clock correction applied at last sync. */
  uint32_t LastSyncAgeMs;             /**< Milliseconds since the last sync. */
  uint32_t SyncCount;                 /**< Successful synchronizations since boot. */
} Sntp_StatusTypeDef;

/**
  * @brief Create the client UDP endpoint.
  * @note  Call once after lwip_init(); requests start when DHCP supplies a
  *        server and the interface has an address.
  */
void Sntp_Init(void);

/**
  * @brief Run timeouts, failover, and periodic resynchronization.
  * @note  Non-blocking; called from MX_LWIP_Process() in main-loop context.
  */
void Sntp_Process(void);

/** @brief Start a new exchange as soon as the network allows. */
void Sntp_RequestSync(void);

/**
  * @brief Copy the client state.
  * @param status (Sntp_StatusTypeDef*) Non-null destination.
  * @retval (HAL_StatusTypeDef) HAL_OK, or HAL_ERROR for a null argument.
  */
HAL_StatusTypeDef Sntp_GetStatus(Sntp_StatusTypeDef *status);

/**
  * @brief Read the software UTC clock.
  * @param unixMs (uint64_t*) Non-null; receives milliseconds since
  *        1970-01-01T00:00:00Z.
  * @retval (bool) true once the clock has been synchronized at least once.
  * @note  Advances with the 1 kHz HAL tick between synchronizations.
  */
bool Sntp_GetUnixTimeMs(uint64_t *unixMs);

/**
  * @brief Format Unix milliseconds as "YYYY-MM-DD HH:MM:SS" (UTC).
  * @param unixMs (uint64_t) Time to format.
  * @param text (char*) Non-null destination.
  * @param size (size_t) Destination size; at least 20 bytes for the full text.
  */
void Sntp_FormatUtc(uint64_t unixMs, char *text, size_t size);

/** @brief Short constant description of a state. */
const char *Sntp_GetStateName(Sntp_StateTypeDef state);

#endif /* SNTP_CLIENT_H */
