/**
  * @file  sntp_client.c
  * @brief Minimal unicast SNTPv4 client (RFC 4330) for bare-metal lwIP.
  *
  * Servers come exclusively from DHCP option 42 through lwIP's
  * dhcp_set_ntp_servers() hook. The client keeps a software UTC clock:
  * the Unix time of the last good reply plus the HAL tick elapsed since.
  * All callbacks run from MX_LWIP_Process() in main-loop context.
  */
#include "sntp_client.h"

#include "lwip/def.h"
#include "lwip/dhcp.h"
#include "lwip/netif.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"

#include <stdio.h>
#include <string.h>

#define SNTP_PORT                   123U
#define SNTP_PACKET_SIZE            48U
#define SNTP_REPLY_TIMEOUT_MS       3000U
#define SNTP_RETRY_INTERVAL_MS      30000U
#define SNTP_UPDATE_INTERVAL_MS     3600000U   /* 1 h; HSE drift is ~20 ppm. */
#define SNTP_REBASE_INTERVAL_MS     86400000U  /* Keep tick deltas far from wrap. */
#define SNTP_UNIX_EPOCH_IN_NTP      2208988800ULL
#define SNTP_ERA_SECONDS            4294967296ULL

/* Packet fields (RFC 4330 section 4). */
#define SNTP_OFFSET_FLAGS           0U
#define SNTP_OFFSET_STRATUM         1U
#define SNTP_OFFSET_REFERENCE_ID    12U
#define SNTP_OFFSET_ORIGINATE       24U
#define SNTP_OFFSET_RECEIVE         32U
#define SNTP_OFFSET_TRANSMIT        40U
#define SNTP_LEAP_UNSYNCHRONIZED    3U
#define SNTP_MODE_CLIENT            3U
#define SNTP_MODE_SERVER            4U
#define SNTP_VERSION                4U

extern struct netif gnetif;

static struct udp_pcb *sntpPcb;
static ip4_addr_t sntpServers[SNTP_MAX_SERVERS];
static uint8_t sntpServerCount;
static bool sntpServersReported;
static uint8_t sntpServerIndex;
static uint8_t sntpFailures;

static Sntp_StateTypeDef sntpState = SNTP_STATE_NO_SERVER;
static bool sntpPending;
static uint32_t sntpSentTick;
static uint8_t sntpCookie[8];          /* Our transmit timestamp (random). */
static uint32_t sntpNextAttemptTick;
static bool sntpAttemptDue = true;

static bool sntpSynchronized;
static uint64_t sntpBaseUnixMs;        /* Clock value at sntpBaseTick. */
static uint32_t sntpBaseTick;
static ip4_addr_t sntpSyncServer;
static uint8_t sntpStratum;
static uint32_t sntpRoundTripMs;
static int32_t sntpLastStepMs;
static uint32_t sntpLastSyncTick;
static uint32_t sntpSyncCount;

static uint32_t sntp_ReadBe32(const uint8_t *data)
{
  return ((uint32_t)data[0] << 24) | ((uint32_t)data[1] << 16) |
         ((uint32_t)data[2] << 8) | (uint32_t)data[3];
}

/** Convert a 64-bit NTP timestamp to Unix milliseconds (eras 0 and 1). */
static uint64_t sntp_NtpToUnixMs(const uint8_t *timestamp)
{
  uint64_t seconds = sntp_ReadBe32(timestamp);
  const uint64_t fraction = sntp_ReadBe32(timestamp + 4);
  /* Era 1 begins in 2036: a clear MSB means the counter has wrapped. */
  if ((seconds & 0x80000000ULL) == 0U)
  {
    seconds += SNTP_ERA_SECONDS;
  }
  return ((seconds - SNTP_UNIX_EPOCH_IN_NTP) * 1000ULL) +
         ((fraction * 1000ULL) >> 32);
}

/** Wrap-safe "deadline reached" for HAL ticks. */
static bool sntp_IsDue(uint32_t deadline)
{
  return (int32_t)(HAL_GetTick() - deadline) >= 0;
}

static void sntp_ScheduleAttempt(uint32_t delayMs)
{
  sntpNextAttemptTick = HAL_GetTick() + delayMs;
  sntpAttemptDue = false;
}

static void sntp_SetClock(uint64_t unixMs)
{
  uint64_t previous = 0U;
  const bool hadClock = Sntp_GetUnixTimeMs(&previous);
  sntpBaseUnixMs = unixMs;
  sntpBaseTick = HAL_GetTick();
  sntpLastStepMs = hadClock ? (int32_t)((int64_t)unixMs - (int64_t)previous) : 0;
  sntpSynchronized = true;
}

static void sntp_Fail(const char *reason)
{
  char server[16];
  (void)ip4addr_ntoa_r(&sntpServers[sntpServerIndex], server, sizeof(server));
  printf("NTP: %s from %s\r\n", reason, server);

  sntpPending = false;
  sntpState = SNTP_STATE_RETRYING;
  sntpFailures++;
  if (sntpServerCount > 0U)
  {
    sntpServerIndex = (uint8_t)((sntpServerIndex + 1U) % sntpServerCount);
  }
  /* Try the next server at once; after a full round, back off. */
  sntp_ScheduleAttempt((sntpFailures < sntpServerCount) ? 0U :
                                                          SNTP_RETRY_INTERVAL_MS);
}

static void sntp_Receive(void *arg, struct udp_pcb *pcb, struct pbuf *p,
                         const ip_addr_t *address, u16_t port)
{
  (void)arg;
  (void)pcb;
  const uint32_t receivedTick = HAL_GetTick();
  uint8_t packet[SNTP_PACKET_SIZE];
  const bool complete =
      pbuf_copy_partial(p, packet, SNTP_PACKET_SIZE, 0U) == SNTP_PACKET_SIZE;
  pbuf_free(p);

  /* Ignore anything that is not the reply to our outstanding request. */
  if (!sntpPending || !complete || (port != SNTP_PORT) ||
      (address == NULL) || !IP_IS_V4(address) ||
      !ip4_addr_cmp(ip_2_ip4(address), &sntpServers[sntpServerIndex]) ||
      (memcmp(&packet[SNTP_OFFSET_ORIGINATE], sntpCookie,
              sizeof(sntpCookie)) != 0))
  {
    return;
  }

  const uint8_t flags = packet[SNTP_OFFSET_FLAGS];
  const uint8_t leap = flags >> 6;
  const uint8_t version = (flags >> 3) & 0x07U;
  const uint8_t mode = flags & 0x07U;
  const uint8_t stratum = packet[SNTP_OFFSET_STRATUM];

  if ((mode != SNTP_MODE_SERVER) || (version < 3U) || (version > 4U))
  {
    sntp_Fail("malformed reply");
    return;
  }
  if (stratum == 0U)
  {
    char code[5];
    memcpy(code, &packet[SNTP_OFFSET_REFERENCE_ID], 4U);
    code[4] = '\0';
    printf("NTP: kiss-o'-death \"%s\"\r\n", code);
    sntp_Fail("rejected request");
    return;
  }
  if ((stratum > 15U) || (leap == SNTP_LEAP_UNSYNCHRONIZED) ||
      (sntp_ReadBe32(&packet[SNTP_OFFSET_TRANSMIT]) == 0U))
  {
    sntp_Fail("unsynchronized server");
    return;
  }

  /* delay = (T4 - T1) - (T3 - T2); T1/T4 from the local tick. */
  const uint64_t serverReceive = sntp_NtpToUnixMs(&packet[SNTP_OFFSET_RECEIVE]);
  const uint64_t serverTransmit = sntp_NtpToUnixMs(&packet[SNTP_OFFSET_TRANSMIT]);
  const uint32_t localElapsed = receivedTick - sntpSentTick;
  const uint64_t serverElapsed = (serverTransmit > serverReceive) ?
                                 (serverTransmit - serverReceive) : 0U;
  const uint32_t roundTrip = (serverElapsed < localElapsed) ?
                             (uint32_t)(localElapsed - serverElapsed) : 0U;

  const bool firstSync = !sntpSynchronized;
  sntp_SetClock(serverTransmit + (roundTrip / 2U));

  sntpPending = false;
  sntpFailures = 0U;
  sntpState = SNTP_STATE_SYNCHRONIZED;
  ip4_addr_copy(sntpSyncServer, sntpServers[sntpServerIndex]);
  sntpStratum = stratum;
  sntpRoundTripMs = roundTrip;
  sntpLastSyncTick = receivedTick;
  sntpSyncCount++;
  sntp_ScheduleAttempt(SNTP_UPDATE_INTERVAL_MS);

  char server[16];
  char utc[24];
  uint64_t now = 0U;
  (void)Sntp_GetUnixTimeMs(&now);
  (void)ip4addr_ntoa_r(&sntpSyncServer, server, sizeof(server));
  Sntp_FormatUtc(now, utc, sizeof(utc));
  if (firstSync)
  {
    printf("NTP: clock set to %s UTC from %s (stratum %u, rtt %lu ms)\r\n",
           utc, server, stratum, (unsigned long)roundTrip);
  }
  else
  {
    printf("NTP: %s UTC from %s (stratum %u, rtt %lu ms, step %+ld ms)\r\n",
           utc, server, stratum, (unsigned long)roundTrip,
           (long)sntpLastStepMs);
  }
}

static void sntp_SendRequest(void)
{
  struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, SNTP_PACKET_SIZE, PBUF_RAM);
  if (p == NULL)
  {
    sntp_ScheduleAttempt(SNTP_RETRY_INTERVAL_MS);
    return;
  }

  uint8_t *packet = (uint8_t *)p->payload;
  memset(packet, 0, SNTP_PACKET_SIZE);
  packet[SNTP_OFFSET_FLAGS] = (SNTP_VERSION << 3) | SNTP_MODE_CLIENT;

  /* RFC 4330 lets the client put any value in the transmit timestamp; a random
     cookie echoed as the originate timestamp rejects stale or forged replies
     and does not reveal the local clock. */
  for (uint32_t index = 0U; index < sizeof(sntpCookie); index += 4U)
  {
    const uint32_t random = LWIP_RAND();
    memcpy(&sntpCookie[index], &random, sizeof(random));
  }
  memcpy(&packet[SNTP_OFFSET_TRANSMIT], sntpCookie, sizeof(sntpCookie));

  ip_addr_t server;
  ip_addr_copy_from_ip4(server, sntpServers[sntpServerIndex]);
  sntpSentTick = HAL_GetTick();
  const err_t status = udp_sendto(sntpPcb, p, &server, SNTP_PORT);
  pbuf_free(p);

  if (status != ERR_OK)
  {
    sntp_ScheduleAttempt(SNTP_RETRY_INTERVAL_MS);
    return;
  }
  sntpPending = true;
  sntpState = SNTP_STATE_REQUESTING;
}

/* lwIP hook (LWIP_DHCP_GET_NTP_SRV): called on every DHCP ACK. */
void dhcp_set_ntp_servers(u8_t num_ntp_servers, const ip4_addr_t *ntp_server_addrs)
{
  const uint8_t count = (num_ntp_servers > SNTP_MAX_SERVERS) ?
                        SNTP_MAX_SERVERS : num_ntp_servers;
  bool changed = (count != sntpServerCount) || !sntpServersReported;
  for (uint8_t index = 0U; index < count; index++)
  {
    if (!ip4_addr_cmp(&sntpServers[index], &ntp_server_addrs[index]))
    {
      changed = true;
    }
  }
  if (!changed)
  {
    return;   /* Lease renewal with the same servers. */
  }

  for (uint8_t index = 0U; index < count; index++)
  {
    ip4_addr_copy(sntpServers[index], ntp_server_addrs[index]);
  }
  sntpServerCount = count;
  sntpServersReported = true;
  sntpServerIndex = 0U;
  sntpFailures = 0U;
  sntpPending = false;

  if (count == 0U)
  {
    sntpState = SNTP_STATE_NO_SERVER;
    printf("DHCP: no NTP server offered (option 42)\r\n");
    return;
  }

  printf("DHCP: NTP server%s", (count > 1U) ? "s" : "");
  for (uint8_t index = 0U; index < count; index++)
  {
    char text[16];
    (void)ip4addr_ntoa_r(&sntpServers[index], text, sizeof(text));
    printf("%s%s", (index == 0U) ? " " : ", ", text);
  }
  printf("\r\n");
  sntpState = SNTP_STATE_WAITING_NETWORK;
  sntpAttemptDue = true;
}

void Sntp_Init(void)
{
  sntpPcb = udp_new_ip_type(IPADDR_TYPE_V4);
  if (sntpPcb == NULL)
  {
    printf("NTP: no UDP PCB available\r\n");
    return;
  }
  (void)udp_bind(sntpPcb, IP4_ADDR_ANY, 0U);
  udp_recv(sntpPcb, sntp_Receive, NULL);
}

void Sntp_Process(void)
{
  /* Re-base the clock periodically so tick differences never wrap (49 days). */
  if (sntpSynchronized &&
      ((HAL_GetTick() - sntpBaseTick) >= SNTP_REBASE_INTERVAL_MS))
  {
    const uint32_t now = HAL_GetTick();
    sntpBaseUnixMs += (uint64_t)(now - sntpBaseTick);
    sntpBaseTick = now;
  }

  if ((sntpPcb == NULL) || (sntpServerCount == 0U))
  {
    return;
  }

  if (sntpPending)
  {
    if ((HAL_GetTick() - sntpSentTick) >= SNTP_REPLY_TIMEOUT_MS)
    {
      sntp_Fail("no reply");
    }
    return;
  }

  const bool networkReady = netif_is_up(&gnetif) && netif_is_link_up(&gnetif) &&
                            !ip4_addr_isany_val(*netif_ip4_addr(&gnetif));
  if (!networkReady)
  {
    if (sntpState != SNTP_STATE_SYNCHRONIZED)
    {
      sntpState = SNTP_STATE_WAITING_NETWORK;
    }
    return;
  }

  if (sntpAttemptDue || sntp_IsDue(sntpNextAttemptTick))
  {
    sntpAttemptDue = false;
    sntp_SendRequest();
  }
}

void Sntp_RequestSync(void)
{
  sntpFailures = 0U;
  if (!sntpPending)
  {
    sntpAttemptDue = true;
  }
}

HAL_StatusTypeDef Sntp_GetStatus(Sntp_StatusTypeDef *status)
{
  if (status == NULL)
  {
    return HAL_ERROR;
  }
  memset(status, 0, sizeof(*status));
  status->State = sntpState;
  status->ServerCount = sntpServerCount;
  memcpy(status->Servers, sntpServers, sizeof(status->Servers));
  status->Synchronized = sntpSynchronized;
  ip4_addr_copy(status->SyncServer, sntpSyncServer);
  status->Stratum = sntpStratum;
  status->RoundTripMs = sntpRoundTripMs;
  status->LastStepMs = sntpLastStepMs;
  status->LastSyncAgeMs = sntpSynchronized ? (HAL_GetTick() - sntpLastSyncTick) : 0U;
  status->SyncCount = sntpSyncCount;
  return HAL_OK;
}

bool Sntp_GetUnixTimeMs(uint64_t *unixMs)
{
  if ((unixMs == NULL) || !sntpSynchronized)
  {
    return false;
  }
  *unixMs = sntpBaseUnixMs + (uint64_t)(HAL_GetTick() - sntpBaseTick);
  return true;
}

void Sntp_FormatUtc(uint64_t unixMs, char *text, size_t size)
{
  if ((text == NULL) || (size == 0U))
  {
    return;
  }

  const uint64_t seconds = unixMs / 1000U;
  const uint32_t secondOfDay = (uint32_t)(seconds % 86400U);

  /* Civil date from days since 1970-01-01 (H. Hinnant, days_from_civil
     inverse); valid for every non-negative Unix time. */
  const uint64_t z = (seconds / 86400U) + 719468U;
  const uint64_t era = z / 146097U;
  const uint32_t dayOfEra = (uint32_t)(z - (era * 146097U));
  const uint32_t yearOfEra = (dayOfEra - (dayOfEra / 1460U) +
                              (dayOfEra / 36524U) - (dayOfEra / 146096U)) / 365U;
  const uint32_t dayOfYear = dayOfEra - ((365U * yearOfEra) + (yearOfEra / 4U) -
                                         (yearOfEra / 100U));
  const uint32_t monthIndex = ((5U * dayOfYear) + 2U) / 153U;
  const uint32_t day = dayOfYear - (((153U * monthIndex) + 2U) / 5U) + 1U;
  const uint32_t month = (monthIndex < 10U) ? (monthIndex + 3U) : (monthIndex - 9U);
  const uint64_t year = yearOfEra + (era * 400U) + ((month <= 2U) ? 1U : 0U);

  (void)snprintf(text, size, "%04lu-%02lu-%02lu %02lu:%02lu:%02lu",
                 (unsigned long)year, (unsigned long)month, (unsigned long)day,
                 (unsigned long)(secondOfDay / 3600U),
                 (unsigned long)((secondOfDay / 60U) % 60U),
                 (unsigned long)(secondOfDay % 60U));
}

const char *Sntp_GetStateName(Sntp_StateTypeDef state)
{
  switch (state)
  {
    case SNTP_STATE_NO_SERVER:       return "no server from DHCP";
    case SNTP_STATE_WAITING_NETWORK: return "waiting for network";
    case SNTP_STATE_REQUESTING:      return "requesting";
    case SNTP_STATE_SYNCHRONIZED:    return "synchronized";
    case SNTP_STATE_RETRYING:        return "retrying";
    default:                         return "unknown";
  }
}
