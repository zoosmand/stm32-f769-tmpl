/**
 ******************************************************************************
  * File Name          : LWIP.c
  * Description        : Bare-metal (NO_SYS) lwIP initialization, processing,
  *                      and read-only network status for the UI and console.
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

#include "lwip.h"
#include "lwip/init.h"
#include "lwip/netif.h"
#include "lwip/prot/dhcp.h"
#if LWIP_DNS
#include "lwip/dns.h"
#endif
#include "ethernetif.h"

#include <stdio.h>
#include <string.h>

/** Link/DHCP polling period in milliseconds. */
#define NETWORK_POLL_PERIOD_MS  100U

struct netif gnetif;

static uint32_t networkPollTimer;
static bool networkInitialized;
static bool networkReportedLink;
static uint32_t networkReportedAddress;

/** Log link and address changes once, through printf(). */
static void network_ReportChanges(void)
{
  Network_StatusTypeDef status;
  if (Network_GetStatus(&status) != HAL_OK)
  {
    return;
  }

  if (status.LinkUp != networkReportedLink)
  {
    networkReportedLink = status.LinkUp;
    if (status.LinkUp)
    {
      printf("ETH: link up, %lu Mbit/s %s duplex\r\n",
             (unsigned long)status.SpeedMbps,
             status.FullDuplex ? "full" : "half");
    }
    else
    {
      printf("ETH: link down\r\n");
    }
  }

  const uint32_t address = ip4_addr_get_u32(&status.Address);
  if (address != networkReportedAddress)
  {
    networkReportedAddress = address;
    if (address != 0U)
    {
      char ip[16];
      char mask[16];
      char gw[16];
      (void)ip4addr_ntoa_r(&status.Address, ip, sizeof(ip));
      (void)ip4addr_ntoa_r(&status.Netmask, mask, sizeof(mask));
      (void)ip4addr_ntoa_r(&status.Gateway, gw, sizeof(gw));
      printf("DHCP: address %s mask %s gateway %s\r\n", ip, mask, gw);
    }
    else
    {
      printf("DHCP: address released\r\n");
    }
  }
}

void MX_LWIP_Init(void)
{
  ip4_addr_t address;
  ip4_addr_t netmask;
  ip4_addr_t gateway;

  lwip_init();

  ip4_addr_set_zero(&address);
  ip4_addr_set_zero(&netmask);
  ip4_addr_set_zero(&gateway);

  /* ethernetif_init() starts the MAC and probes the PHY without blocking. */
  netif_add(&gnetif, &address, &netmask, &gateway, NULL, &ethernetif_init,
            &ethernet_input);
  netif_set_default(&gnetif);

  /* dhcp_start() requires an administratively up interface; DHCP then waits
     in DHCP_STATE_INIT until the link comes up and restarts on link changes. */
  netif_set_up(&gnetif);
  const err_t dhcpStatus = dhcp_start(&gnetif);
  if (dhcpStatus != ERR_OK)
  {
    printf("DHCP: start failed (%d)\r\n", (int)dhcpStatus);
  }

  networkPollTimer = HAL_GetTick();
  networkInitialized = true;
}

void MX_LWIP_Process(void)
{
  if (!networkInitialized)
  {
    return;
  }

  ethernetif_input(&gnetif);
  sys_check_timeouts();

  if ((HAL_GetTick() - networkPollTimer) >= NETWORK_POLL_PERIOD_MS)
  {
    networkPollTimer = HAL_GetTick();
    ethernet_link_check_state(&gnetif);
    network_ReportChanges();
  }
}

HAL_StatusTypeDef Network_GetStatus(Network_StatusTypeDef *status)
{
  if (status == NULL)
  {
    return HAL_ERROR;
  }

  memset(status, 0, sizeof(*status));
  status->Initialized = networkInitialized;
  if (!networkInitialized)
  {
    return HAL_OK;
  }

  status->LinkUp = netif_is_link_up(&gnetif) != 0U;
  status->InterfaceUp = netif_is_up(&gnetif) != 0U;
  memcpy(status->Mac, gnetif.hwaddr, sizeof(status->Mac));

  const struct dhcp *dhcp = netif_dhcp_data(&gnetif);
  status->DhcpState = (dhcp != NULL) ? dhcp->state : DHCP_STATE_OFF;
  status->DhcpBound = dhcp_supplied_address(&gnetif) != 0U;

  ip4_addr_copy(status->Address, *netif_ip4_addr(&gnetif));
  ip4_addr_copy(status->Netmask, *netif_ip4_netmask(&gnetif));
  ip4_addr_copy(status->Gateway, *netif_ip4_gw(&gnetif));
#if LWIP_DNS
  ip4_addr_copy(status->Dns, *ip_2_ip4(dns_getserver(0)));
#endif

  if (status->LinkUp)
  {
    ETH_MACConfigTypeDef mac;
    if (HAL_ETH_GetMACConfig(&heth, &mac) == HAL_OK)
    {
      status->SpeedMbps = (mac.Speed == ETH_SPEED_100M) ? 100U : 10U;
      status->FullDuplex = mac.DuplexMode == ETH_FULLDUPLEX_MODE;
    }
  }
  return HAL_OK;
}

const char *Network_GetDhcpStateName(uint8_t state)
{
  switch (state)
  {
    case DHCP_STATE_OFF:         return "off";
    case DHCP_STATE_REQUESTING:  return "requesting";
    case DHCP_STATE_INIT:        return "init";
    case DHCP_STATE_REBOOTING:   return "rebooting";
    case DHCP_STATE_REBINDING:   return "rebinding";
    case DHCP_STATE_RENEWING:    return "renewing";
    case DHCP_STATE_SELECTING:   return "selecting";
    case DHCP_STATE_INFORMING:   return "informing";
    case DHCP_STATE_CHECKING:    return "checking";
    case DHCP_STATE_BOUND:       return "bound";
    case DHCP_STATE_BACKING_OFF: return "backing off";
    default:                     return "unknown";
  }
}
