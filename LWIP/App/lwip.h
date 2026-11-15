/**
  ******************************************************************************
  * File Name          : LWIP.h
  * Description        : Bare-metal lwIP integration and network status.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#ifndef __mx_lwip_H
#define __mx_lwip_H
#ifdef __cplusplus
 extern "C" {
#endif

#include <stdbool.h>

#include "lwip/opt.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "lwip/dns.h"
#include "ethernetif.h"

extern ETH_HandleTypeDef heth;

/** Snapshot of the Ethernet interface state. */
typedef struct
{
  bool Initialized;        /**< MX_LWIP_Init() has run. */
  bool LinkUp;             /**< PHY reports an established link. */
  bool InterfaceUp;        /**< lwIP netif administratively up. */
  bool DhcpBound;          /**< DHCP supplied the current address. */
  uint8_t DhcpState;       /**< lwIP DHCP_STATE_* value. */
  uint32_t SpeedMbps;      /**< 10 or 100 while the link is up, else 0. */
  bool FullDuplex;         /**< Duplex mode while the link is up. */
  ip4_addr_t Address;      /**< Interface IPv4 address (0 when none). */
  ip4_addr_t Netmask;      /**< IPv4 netmask. */
  ip4_addr_t Gateway;      /**< Default gateway. */
  ip4_addr_t Dns[DNS_MAX_SERVERS]; /**< DNS servers from DHCP option 6. */
  uint8_t Mac[6];          /**< Interface MAC address. */
} Network_StatusTypeDef;

/**
  * @brief Initialize lwIP (NO_SYS), the Ethernet MAC, and start DHCP.
  * @note  Does not wait for the PHY or a link; both are polled by
  *        MX_LWIP_Process(). Call once from main() after HAL and clocks.
  */
void MX_LWIP_Init(void);

/**
  * @brief Receive frames, run lwIP timers and the SNTP client, poll the link
  *        every 100 ms, and log link/DHCP/DNS changes through printf().
  * @note  Non-blocking; call continuously from the main loop only.
  */
void MX_LWIP_Process(void);

/**
  * @brief Copy the current interface state.
  * @param status (Network_StatusTypeDef*) Non-null destination.
  * @retval (HAL_StatusTypeDef) HAL_OK, or HAL_ERROR for a null argument.
  * @note  Main-loop context only (reads lwIP state without locking).
  */
HAL_StatusTypeDef Network_GetStatus(Network_StatusTypeDef *status);

/**
  * @brief Return a short static name for a DHCP_STATE_* value.
  * @param state (uint8_t) lwIP DHCP state.
  * @retval (const char*) Non-null constant string.
  */
const char *Network_GetDhcpStateName(uint8_t state);

#ifdef __cplusplus
}
#endif
#endif /*__ mx_lwip_H */
