#include "self_test.h"

#include "fmc.h"
#include "ft6206.h"
#include "lan8742.h"
#include "lwip.h"
#include "sntp_client.h"
#include "main.h"
#include "otm8009a.h"
#include "ui.h"

#include <stdio.h>
#include <string.h>

/* Expected platform configuration. */
#define SELF_TEST_DEVICE_ID           0x451U
#define SELF_TEST_SYSCLK_HZ           216000000U
#define SELF_TEST_PCLK1_HZ            54000000U
#define SELF_TEST_PCLK2_HZ            108000000U
#define SELF_TEST_UART_BAUD           115200U

/* Non-destructive SDRAM probe: 16 words at the very top of SDRAM. */
#define SELF_TEST_SDRAM_WORDS         16U
#define SELF_TEST_SDRAM_OFFSET \
  (SDRAM_SIZE_BYTES - (SELF_TEST_SDRAM_WORDS * sizeof(uint32_t)))

/* Framebuffers occupy the first four 2 MiB slots (L1/L2 front/back). The upper
   8 MiB is unused by firmware and may be overwritten by intrusive tests. */
#define SELF_TEST_SCRATCH_OFFSET      (4U * BUF_SIZE)
#define SELF_TEST_SCRATCH_BYTES       (SDRAM_SIZE_BYTES - SELF_TEST_SCRATCH_OFFSET)
#define SELF_TEST_CHUNK_WORDS         256U

#define SELF_TEST_DMA2D_PIXELS        4096U
#define SELF_TEST_DMA2D_TIMEOUT_MS    10U

#define SELF_TEST_I2C_FIRST_ADDR      0x08U
#define SELF_TEST_I2C_LAST_ADDR       0x77U
#define SELF_TEST_I2C_TIMEOUT_MS      5U

#define SELF_TEST_WM8994_ADDR         (0x1AU << 1)
#define SELF_TEST_WM8994_ID           0x8994U

#define SELF_TEST_FRAME_WINDOW_MS     250U

#define SELF_TEST_DNS_HOSTNAME        "example.com"
#define SELF_TEST_NETWORK_TIMEOUT_MS  6000U
#define SELF_TEST_MIN_UNIX_SECONDS    1735689600ULL   /* 2025-01-01T00:00:00Z */

/** One named test. Post marks members of the non-destructive POST suite. */
typedef struct
{
  const char *Name;
  const char *Description;
  bool Post;
  bool Early;
  bool (*Run)(const SelfTest_Context_TypeDef *context);
} SelfTest_Case_TypeDef;

/* Chunk buffer for SDRAM pattern tests; DTCM, so no cache maintenance. */
static uint32_t selfTestChunk[SELF_TEST_CHUNK_WORDS];

/** Print one sub-check line and return its outcome. */
static bool selfTest_Check(const char *label, bool passed, const char *format,
                           unsigned long value)
{
  printf("    %s %-22s ", passed ? "ok  " : "FAIL", label);
  printf(format, value);
  printf("\r\n");
  return passed;
}

/** Start the DWT cycle counter used for timing measurements. */
static void selfTest_CycleCounterStart(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->LAR = 0xC5ACCE55U;
  DWT->CYCCNT = 0U;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static uint32_t selfTest_CyclesToUs(uint32_t cycles)
{
  return cycles / (SystemCoreClock / 1000000U);
}

/* -------------------------------------------------------------------------- */

static bool test_Core(const SelfTest_Context_TypeDef *context)
{
  (void)context;
  bool passed = true;

  const uint32_t deviceId = HAL_GetDEVID();
  passed &= selfTest_Check("device ID", deviceId == SELF_TEST_DEVICE_ID,
                           "0x%03lX", deviceId);
  (void)selfTest_Check("revision ID", true, "0x%04lX", HAL_GetREVID());

  const uint32_t caches = SCB->CCR & (SCB_CCR_IC_Msk | SCB_CCR_DC_Msk);
  passed &= selfTest_Check("I/D cache enabled",
                           caches == (SCB_CCR_IC_Msk | SCB_CCR_DC_Msk),
                           "CCR=0x%08lX", SCB->CCR);

  const uint32_t fpuAccess = (SCB->CPACR >> 20) & 0xFU;
  passed &= selfTest_Check("FPU CP10/CP11 access", fpuAccess == 0xFU,
                           "0x%lX", fpuAccess);

  passed &= selfTest_Check("MPU enabled",
                           (MPU->CTRL & MPU_CTRL_ENABLE_Msk) != 0U,
                           "CTRL=0x%08lX", MPU->CTRL);

  /* Region 0: Ethernet RAM, 64 KiB (SIZE=15); region 1: SDRAM, 16 MiB (23). */
  static const struct { uint32_t base; uint32_t sizeField; const char *label; }
  regions[] = {
    { 0x20020000U, 15U, "MPU0 ETH RAM uncached" },
    { SDRAM_START_ADDR, 23U, "MPU1 SDRAM uncached" },
  };
  for (uint32_t index = 0U; index < 2U; index++)
  {
    MPU->RNR = index;
    const uint32_t rbar = MPU->RBAR & MPU_RBAR_ADDR_Msk;
    const uint32_t rasr = MPU->RASR;
    const bool ok = (rbar == regions[index].base) &&
                    ((rasr & MPU_RASR_ENABLE_Msk) != 0U) &&
                    (((rasr & MPU_RASR_SIZE_Msk) >> MPU_RASR_SIZE_Pos) ==
                     regions[index].sizeField) &&
                    ((rasr & MPU_RASR_C_Msk) == 0U);
    passed &= selfTest_Check(regions[index].label, ok, "RASR=0x%08lX", rasr);
  }

  return passed;
}

static bool test_Clock(const SelfTest_Context_TypeDef *context)
{
  (void)context;
  bool passed = true;

  passed &= selfTest_Check("HSE ready", (RCC->CR & RCC_CR_HSERDY) != 0U,
                           "CR=0x%08lX", RCC->CR);
  passed &= selfTest_Check("main PLL locked", (RCC->CR & RCC_CR_PLLRDY) != 0U,
                           "CR=0x%08lX", RCC->CR);
  passed &= selfTest_Check("PLLSAI locked (LTDC)",
                           (RCC->CR & RCC_CR_PLLSAIRDY) != 0U,
                           "CR=0x%08lX", RCC->CR);
  passed &= selfTest_Check("over-drive active",
                           (PWR->CSR1 & PWR_CSR1_ODSWRDY) != 0U,
                           "CSR1=0x%08lX", PWR->CSR1);
  passed &= selfTest_Check("PLL source is HSE",
                           __HAL_RCC_GET_PLL_OSCSOURCE() == RCC_PLLSOURCE_HSE,
                           "0x%08lX", __HAL_RCC_GET_PLL_OSCSOURCE());

  const uint32_t sysclk = HAL_RCC_GetSysClockFreq();
  passed &= selfTest_Check("SYSCLK", sysclk == SELF_TEST_SYSCLK_HZ, "%lu Hz",
                           sysclk);
  passed &= selfTest_Check("SystemCoreClock",
                           SystemCoreClock == SELF_TEST_SYSCLK_HZ, "%lu Hz",
                           SystemCoreClock);
  passed &= selfTest_Check("HCLK", HAL_RCC_GetHCLKFreq() == SELF_TEST_SYSCLK_HZ,
                           "%lu Hz", HAL_RCC_GetHCLKFreq());
  passed &= selfTest_Check("PCLK1", HAL_RCC_GetPCLK1Freq() == SELF_TEST_PCLK1_HZ,
                           "%lu Hz", HAL_RCC_GetPCLK1Freq());
  passed &= selfTest_Check("PCLK2", HAL_RCC_GetPCLK2Freq() == SELF_TEST_PCLK2_HZ,
                           "%lu Hz", HAL_RCC_GetPCLK2Freq());

  /* SysTick period against the core cycle counter: 100 HAL ticks should take
     SYSCLK / 10 cycles. Both derive from SYSCLK, so this verifies the 1 kHz
     time base configuration rather than the crystal itself. */
  selfTest_CycleCounterStart();
  const uint32_t tick0 = HAL_GetTick();
  while (HAL_GetTick() == tick0)
  {
  }
  const uint32_t tickStart = HAL_GetTick();
  const uint32_t cycleStart = DWT->CYCCNT;
  while ((HAL_GetTick() - tickStart) < 100U)
  {
  }
  const uint32_t cycles = DWT->CYCCNT - cycleStart;
  const uint32_t expected = SystemCoreClock / 10U;
  const uint32_t error = (cycles > expected) ? (cycles - expected) :
                                               (expected - cycles);
  passed &= selfTest_Check("SysTick 100 ms", error < (expected / 200U),
                           "%lu cycles", cycles);

  return passed;
}

static bool test_Usart(const SelfTest_Context_TypeDef *context)
{
  UART_HandleTypeDef *uart = context->Uart;
  if (uart == NULL)
  {
    return selfTest_Check("handle", false, "%lu", 0UL);
  }

  bool passed = true;
  passed &= selfTest_Check("TX state ready",
                           uart->gState == HAL_UART_STATE_READY,
                           "0x%02lX", (unsigned long)uart->gState);
  passed &= selfTest_Check("RX armed (console)",
                           uart->RxState == HAL_UART_STATE_BUSY_RX,
                           "0x%02lX", (unsigned long)uart->RxState);
  passed &= selfTest_Check("TE/RE/UE enabled",
                           (uart->Instance->CR1 &
                            (USART_CR1_TE | USART_CR1_RE | USART_CR1_UE)) ==
                           (USART_CR1_TE | USART_CR1_RE | USART_CR1_UE),
                           "CR1=0x%08lX", uart->Instance->CR1);

  const uint32_t brr = uart->Instance->BRR;
  const uint32_t baud = (brr != 0U) ? (HAL_RCC_GetPCLK2Freq() / brr) : 0U;
  const uint32_t error = (baud > SELF_TEST_UART_BAUD) ?
                         (baud - SELF_TEST_UART_BAUD) :
                         (SELF_TEST_UART_BAUD - baud);
  passed &= selfTest_Check("baud rate within 1%",
                           error < (SELF_TEST_UART_BAUD / 100U), "%lu baud",
                           baud);
  return passed;
}

static bool test_Crc(const SelfTest_Context_TypeDef *context)
{
  CRC_HandleTypeDef *crc = context->Crc;
  if ((crc == NULL) || (HAL_CRC_GetState(crc) != HAL_CRC_STATE_READY))
  {
    return selfTest_Check("state ready", false, "%lu",
                          (crc != NULL) ? HAL_CRC_GetState(crc) : 0UL);
  }

  /* Default STM32 CRC (poly 0x04C11DB7, init 0xFFFFFFFF, no reflection, no
     final XOR) is CRC-32/MPEG-2, whose check value for "123456789" is
     0x0376E6E7. */
  static const uint8_t check[] = "123456789";
  bool passed = true;
  const uint32_t first = HAL_CRC_Calculate(crc, (uint32_t *)check, 9U);
  passed &= selfTest_Check("CRC-32/MPEG-2 \"123456789\"", first == 0x0376E6E7U,
                           "0x%08lX", first);

  static const uint8_t zeros[4] = {0U, 0U, 0U, 0U};
  const uint32_t zeroCrc = HAL_CRC_Calculate(crc, (uint32_t *)zeros, 4U);
  passed &= selfTest_Check("CRC-32/MPEG-2 4 x 0x00", zeroCrc == 0xC704DD7BU,
                           "0x%08lX", zeroCrc);

  const uint32_t again = HAL_CRC_Calculate(crc, (uint32_t *)check, 9U);
  passed &= selfTest_Check("repeatable after reset", again == first,
                           "0x%08lX", again);
  return passed;
}

/** Probe every 7-bit address; return the number of responders. */
static uint32_t selfTest_I2cScan(I2C_HandleTypeDef *bus, uint8_t mustFind,
                                 bool *found)
{
  uint32_t count = 0U;
  *found = false;
  printf("    scan:");
  for (uint16_t address = SELF_TEST_I2C_FIRST_ADDR;
       address <= SELF_TEST_I2C_LAST_ADDR; address++)
  {
    if (HAL_I2C_IsDeviceReady(bus, (uint16_t)(address << 1), 1U,
                              SELF_TEST_I2C_TIMEOUT_MS) == HAL_OK)
    {
      printf(" 0x%02X", address);
      count++;
      if (address == mustFind)
      {
        *found = true;
      }
    }
  }
  printf("%s\r\n", (count == 0U) ? " none" : "");
  return count;
}

static bool test_I2c1(const SelfTest_Context_TypeDef *context)
{
  I2C_HandleTypeDef *bus = context->I2c1;
  if ((bus == NULL) || (HAL_I2C_GetState(bus) != HAL_I2C_STATE_READY))
  {
    return selfTest_Check("state ready", false, "%lu",
                          (bus != NULL) ? HAL_I2C_GetState(bus) : 0UL);
  }

  /* The Arduino/extension bus has no on-board devices: the scan only proves
     the controller completes transfers and the bus is not stuck. */
  bool unused;
  const uint32_t count = selfTest_I2cScan(bus, 0U, &unused);
  bool passed = selfTest_Check("devices (informational)", true, "%lu", count);
  passed &= selfTest_Check("bus idle after scan",
                           HAL_I2C_GetState(bus) == HAL_I2C_STATE_READY,
                           "ISR=0x%08lX", bus->Instance->ISR);
  return passed;
}

static bool test_I2c4(const SelfTest_Context_TypeDef *context)
{
  I2C_HandleTypeDef *bus = context->I2c4;
  if ((bus == NULL) || (HAL_I2C_GetState(bus) != HAL_I2C_STATE_READY))
  {
    return selfTest_Check("state ready", false, "%lu",
                          (bus != NULL) ? HAL_I2C_GetState(bus) : 0UL);
  }

  bool touchFound;
  const uint32_t count = selfTest_I2cScan(bus, FT6206_I2C_ADDR, &touchFound);
  bool passed = selfTest_Check("FT6206 at 0x2A", touchFound, "%lu devices",
                               count);
  passed &= selfTest_Check("bus idle after scan",
                           HAL_I2C_GetState(bus) == HAL_I2C_STATE_READY,
                           "ISR=0x%08lX", bus->Instance->ISR);
  return passed;
}

static bool test_Touch(const SelfTest_Context_TypeDef *context)
{
  TouchScreen_TypeDef *touch = context->Touch;
  if ((touch == NULL) || (touch->BusHandler == NULL))
  {
    return selfTest_Check("device", false, "%lu", 0UL);
  }

  bool passed = true;
  passed &= selfTest_Check("driver state usable",
                           (touch->State != TOUCH_LOCKED) &&
                           (touch->State != TOUCH_DISABLED),
                           "state %lu", (unsigned long)touch->State);

  uint8_t info[8] = {0};  /* 0xA1..0xA8 */
  const bool infoRead =
      HAL_I2C_Mem_Read(touch->BusHandler, touch->BusAddr, FT6x02_LIB_VER_H,
                       I2C_MEMADD_SIZE_8BIT, info, sizeof(info), 30U) == HAL_OK;
  passed &= selfTest_Check("ID registers read", infoRead, "%lu", 0UL);
  passed &= selfTest_Check("FocalTech ID 0x11",
                           infoRead && (info[7] == FT6x02_FOCALTECH_ID_VALUE),
                           "0x%02lX", info[7]);
  (void)selfTest_Check("library version", true, "0x%04lX",
                       ((unsigned long)info[0] << 8) | info[1]);
  (void)selfTest_Check("firmware ID", true, "0x%02lX", info[5]);
  (void)selfTest_Check("interrupt mode (G_MODE)", true, "0x%02lX", info[3]);

  uint8_t status = 0xFFU;
  const bool statusRead =
      HAL_I2C_Mem_Read(touch->BusHandler, touch->BusAddr, FT6x02_TD_STATUS,
                       I2C_MEMADD_SIZE_8BIT, &status, 1U, 30U) == HAL_OK;
  /* Counts above 2 (0x0F when idle) mean "no valid contact". */
  const uint32_t contacts = ((status & 0x0FU) <= 2U) ? (status & 0x0FU) : 0U;
  passed &= selfTest_Check("TD_STATUS read", statusRead, "raw 0x%02lX",
                           (unsigned long)status);
  (void)selfTest_Check("active contacts", true, "%lu", contacts);

  (void)selfTest_Check("INT line (PI13) level", true, "%lu",
                       (unsigned long)HAL_GPIO_ReadPin(LCD_INT_GPIO_Port,
                                                       LCD_INT_Pin));
  return passed;
}

static bool test_Codec(const SelfTest_Context_TypeDef *context)
{
  I2C_HandleTypeDef *bus = context->I2c4;
  if (bus == NULL)
  {
    return selfTest_Check("bus", false, "%lu", 0UL);
  }

  /* WM8994 register 0x0000 (software reset/ID) reads back the part number. */
  uint8_t id[2] = {0U, 0U};
  const bool read = HAL_I2C_Mem_Read(bus, SELF_TEST_WM8994_ADDR, 0x0000U,
                                     I2C_MEMADD_SIZE_16BIT, id, sizeof(id),
                                     30U) == HAL_OK;
  const uint32_t value = ((uint32_t)id[0] << 8) | id[1];
  bool passed = selfTest_Check("WM8994 ACK at 0x1A", read, "%lu", 0UL);
  passed &= selfTest_Check("WM8994 chip ID", read && (value == SELF_TEST_WM8994_ID),
                           "0x%04lX", value);
  return passed;
}

/** PHY object owned by ethernetif.c. */
extern lan8742_Object_t LAN8742;

static bool test_Ethernet(const SelfTest_Context_TypeDef *context)
{
  (void)context;
  Network_StatusTypeDef status;
  (void)Network_GetStatus(&status);

  bool passed = selfTest_Check("lwIP started", status.Initialized, "%lu", 0UL);
  passed &= selfTest_Check("LAN8742 detected", LAN8742.Is_Initialized != 0U,
                           "MDIO address %lu", (unsigned long)LAN8742.DevAddr);

  /* PHY identifier: OUI 0x0007, model 0xC13x for the LAN8742A. */
  uint32_t id1 = 0U;
  uint32_t id2 = 0U;
  const bool read = (LAN8742.Is_Initialized != 0U) &&
      (HAL_ETH_ReadPHYRegister(&heth, LAN8742.DevAddr, 2U, &id1) == HAL_OK) &&
      (HAL_ETH_ReadPHYRegister(&heth, LAN8742.DevAddr, 3U, &id2) == HAL_OK);
  passed &= selfTest_Check("PHY ID 0x0007:0xC13x",
                           read && (id1 == 0x0007U) &&
                           ((id2 & 0xFFF0U) == 0xC130U),
                           "0x%08lX", (id1 << 16) | id2);

  /* Link and DHCP depend on the cable and network: informational only. */
  printf("    ok   %-22s %s\r\n", "link (informational)",
         status.LinkUp ? "up" : "down");
  printf("    ok   %-22s %s\r\n", "DHCP (informational)",
         Network_GetDhcpStateName(status.DhcpState));
  return passed;
}

/** DNS lookup completion shared with test_Dns(); main-loop context only. */
static volatile bool selfTestDnsDone;
static ip_addr_t selfTestDnsAddress;
static bool selfTestDnsFound;

static void selfTest_DnsFound(const char *name, const ip_addr_t *address,
                              void *arg)
{
  (void)name;
  (void)arg;
  selfTestDnsFound = address != NULL;
  if (address != NULL)
  {
    ip_addr_copy(selfTestDnsAddress, *address);
  }
  selfTestDnsDone = true;
}

/** Keep lwIP running while a network test waits (single-threaded). */
static void selfTest_PumpNetwork(void)
{
  MX_LWIP_Process();
}

static bool test_Dns(const SelfTest_Context_TypeDef *context)
{
  (void)context;
  Network_StatusTypeDef status;
  (void)Network_GetStatus(&status);

  bool passed = selfTest_Check("DHCP lease", status.DhcpBound, "%lu", 0UL);
  char server[16];
  (void)ip4addr_ntoa_r(&status.Dns[0], server, sizeof(server));
  passed &= selfTest_Check("DNS server from DHCP",
                           ip4_addr_get_u32(&status.Dns[0]) != 0U, "%lu", 0UL);
  printf("    info DNS 1                  %s\r\n", server);
  if (!passed)
  {
    return false;
  }

  selfTestDnsDone = false;
  selfTestDnsFound = false;
  const uint32_t startedAt = HAL_GetTick();
  const err_t request = dns_gethostbyname(SELF_TEST_DNS_HOSTNAME,
                                          &selfTestDnsAddress,
                                          selfTest_DnsFound, NULL);
  if (request == ERR_OK)
  {
    selfTestDnsDone = true;
    selfTestDnsFound = true;
  }
  while ((request == ERR_INPROGRESS) && !selfTestDnsDone &&
         ((HAL_GetTick() - startedAt) < SELF_TEST_NETWORK_TIMEOUT_MS))
  {
    selfTest_PumpNetwork();
  }

  char address[IPADDR_STRLEN_MAX] = "-";
  if (selfTestDnsFound)
  {
    (void)ipaddr_ntoa_r(&selfTestDnsAddress, address, sizeof(address));
  }
  printf("    info %-22s %s (%lu ms)\r\n", SELF_TEST_DNS_HOSTNAME, address,
         (unsigned long)(HAL_GetTick() - startedAt));
  passed &= selfTest_Check("resolve " SELF_TEST_DNS_HOSTNAME,
                           selfTestDnsDone && selfTestDnsFound,
                           "request %ld", (long)request);
  return passed;
}

static bool test_Ntp(const SelfTest_Context_TypeDef *context)
{
  (void)context;
  Sntp_StatusTypeDef before;
  (void)Sntp_GetStatus(&before);

  bool passed = selfTest_Check("NTP server from DHCP", before.ServerCount > 0U,
                               "%lu offered", (unsigned long)before.ServerCount);
  if (!passed)
  {
    return false;
  }

  Sntp_RequestSync();
  const uint32_t startedAt = HAL_GetTick();
  Sntp_StatusTypeDef after = before;
  while ((after.SyncCount == before.SyncCount) &&
         ((HAL_GetTick() - startedAt) < SELF_TEST_NETWORK_TIMEOUT_MS))
  {
    selfTest_PumpNetwork();
    (void)Sntp_GetStatus(&after);
  }

  passed &= selfTest_Check("fresh synchronization",
                           after.SyncCount > before.SyncCount, "%lu ms",
                           (unsigned long)(HAL_GetTick() - startedAt));
  uint64_t unixMs = 0U;
  const bool haveTime = Sntp_GetUnixTimeMs(&unixMs);
  char utc[24] = "-";
  if (haveTime)
  {
    Sntp_FormatUtc(unixMs, utc, sizeof(utc));
  }
  printf("    info UTC                    %s\r\n", utc);
  passed &= selfTest_Check("time after 2025-01-01",
                           haveTime &&
                           ((unixMs / 1000U) >= SELF_TEST_MIN_UNIX_SECONDS),
                           "%lu s", (unsigned long)(unixMs / 1000U));
  passed &= selfTest_Check("stratum 1..15",
                           (after.Stratum >= 1U) && (after.Stratum <= 15U),
                           "%lu", (unsigned long)after.Stratum);
  (void)selfTest_Check("round trip", true, "%lu ms",
                       (unsigned long)after.RoundTripMs);
  (void)selfTest_Check("clock step", true, "%ld ms", (long)after.LastStepMs);
  return passed;
}

static bool test_SdramProbe(const SelfTest_Context_TypeDef *context)
{
  SDRAM_HandleTypeDef *sdram = context->Sdram;
  uint32_t saved[SELF_TEST_SDRAM_WORDS];
  uint32_t pattern[SELF_TEST_SDRAM_WORDS];
  uint32_t observed[SELF_TEST_SDRAM_WORDS];

  if ((sdram == NULL) ||
      (SDRAM_ReadBuffer(sdram, SELF_TEST_SDRAM_OFFSET, SELF_TEST_SDRAM_WORDS,
                        saved) != HAL_OK))
  {
    return selfTest_Check("save probe area", false, "%lu", 0UL);
  }

  for (uint32_t index = 0U; index < SELF_TEST_SDRAM_WORDS; index++)
  {
    pattern[index] = UINT32_C(0xA5A50000) ^ (index * UINT32_C(0x01010101));
  }

  bool passed =
      (SDRAM_WriteBuffer(sdram, SELF_TEST_SDRAM_OFFSET, SELF_TEST_SDRAM_WORDS,
                         pattern) == HAL_OK) &&
      (SDRAM_ReadBuffer(sdram, SELF_TEST_SDRAM_OFFSET, SELF_TEST_SDRAM_WORDS,
                        observed) == HAL_OK) &&
      (memcmp(pattern, observed, sizeof(pattern)) == 0);

  const bool restored =
      SDRAM_WriteBuffer(sdram, SELF_TEST_SDRAM_OFFSET, SELF_TEST_SDRAM_WORDS,
                        saved) == HAL_OK;

  passed = selfTest_Check("pattern at top 64 B", passed, "offset 0x%06lX",
                          SELF_TEST_SDRAM_OFFSET);
  passed &= selfTest_Check("original data restored", restored, "%lu", 0UL);
  passed &= selfTest_Check("FMC state ready",
                           HAL_SDRAM_GetState(sdram) == HAL_SDRAM_STATE_READY,
                           "%lu", (unsigned long)HAL_SDRAM_GetState(sdram));
  return passed;
}

/** Fill or verify the scratch region with a seed-derived pattern. */
static bool selfTest_SdramPattern(SDRAM_HandleTypeDef *sdram, uint32_t seed,
                                  bool verify, uint32_t *firstBadOffset)
{
  for (uint32_t offset = SELF_TEST_SCRATCH_OFFSET;
       offset < SDRAM_SIZE_BYTES;
       offset += sizeof(selfTestChunk))
  {
    if (verify)
    {
      if (SDRAM_ReadBuffer(sdram, offset, SELF_TEST_CHUNK_WORDS,
                           selfTestChunk) != HAL_OK)
      {
        *firstBadOffset = offset;
        return false;
      }
    }

    for (uint32_t index = 0U; index < SELF_TEST_CHUNK_WORDS; index++)
    {
      const uint32_t address = offset + (index * sizeof(uint32_t));
      const uint32_t expected = address ^ seed;
      if (!verify)
      {
        selfTestChunk[index] = expected;
      }
      else if (selfTestChunk[index] != expected)
      {
        *firstBadOffset = address;
        return false;
      }
    }

    if (!verify &&
        (SDRAM_WriteBuffer(sdram, offset, SELF_TEST_CHUNK_WORDS,
                           selfTestChunk) != HAL_OK))
    {
      *firstBadOffset = offset;
      return false;
    }
  }
  return true;
}

static bool test_SdramFull(const SelfTest_Context_TypeDef *context)
{
  SDRAM_HandleTypeDef *sdram = context->Sdram;
  if (sdram == NULL)
  {
    return selfTest_Check("handle", false, "%lu", 0UL);
  }

  bool passed = true;
  const uint32_t base = SELF_TEST_SCRATCH_OFFSET;

  /* Data bus: walking one across all 32 lines. */
  uint32_t badBit = 0xFFU;
  for (uint32_t bit = 0U; bit < 32U; bit++)
  {
    const uint32_t value = UINT32_C(1) << bit;
    if ((SDRAM_Write32b(sdram, base, value) != HAL_OK) ||
        (SDRAM_Read32b(sdram, base) != value))
    {
      badBit = bit;
      break;
    }
  }
  passed &= selfTest_Check("data bus walking 1", badBit == 0xFFU,
                           "bad bit %lu", badBit);

  /* Byte lanes (NBL0..3) through 8- and 16-bit accesses. */
  bool lanesOk = (SDRAM_Write32b(sdram, base, 0U) == HAL_OK);
  for (uint32_t lane = 0U; lanesOk && (lane < 4U); lane++)
  {
    lanesOk = (SDRAM_Write8b(sdram, base + lane, (uint8_t)(0x11U * (lane + 1U))) ==
               HAL_OK);
  }
  const uint32_t laneWord = SDRAM_Read32b(sdram, base);
  lanesOk = lanesOk && (laneWord == 0x44332211U) &&
            (SDRAM_Write16b(sdram, base + 2U, 0xBEEFU) == HAL_OK) &&
            (SDRAM_Read16b(sdram, base + 2U) == 0xBEEFU) &&
            (SDRAM_Read8b(sdram, base) == 0x11U);
  passed &= selfTest_Check("byte lanes 8/16/32-bit", lanesOk, "0x%08lX",
                           laneWord);

  /* Address bus: distinct values at power-of-two offsets must not alias. */
  uint32_t aliasOffset = 0U;
  for (uint32_t step = 4U; step < SELF_TEST_SCRATCH_BYTES; step <<= 1)
  {
    (void)SDRAM_Write32b(sdram, base + step, 0x55AA0000U | step);
  }
  (void)SDRAM_Write32b(sdram, base, 0xDEADBEEFU);
  for (uint32_t step = 4U; step < SELF_TEST_SCRATCH_BYTES; step <<= 1)
  {
    if (SDRAM_Read32b(sdram, base + step) != (0x55AA0000U | step))
    {
      aliasOffset = step;
      break;
    }
  }
  /* Address line A23 separates the scratch area from the visible framebuffer. */
  const uint32_t frameWord = SDRAM_Read32b(sdram, 0U);
  (void)SDRAM_Write32b(sdram, base, ~frameWord);
  const bool highLineOk = SDRAM_Read32b(sdram, 0U) == frameWord;
  passed &= selfTest_Check("address bus no aliasing",
                           (aliasOffset == 0U) && highLineOk,
                           "alias step 0x%06lX", aliasOffset);

  /* Full scratch fill with address-derived data, then its complement. */
  selfTest_CycleCounterStart();
  uint32_t bad = 0U;
  const uint32_t started = DWT->CYCCNT;
  bool fillOk = selfTest_SdramPattern(sdram, 0x00000000U, false, &bad) &&
                selfTest_SdramPattern(sdram, 0x00000000U, true, &bad) &&
                selfTest_SdramPattern(sdram, 0xFFFFFFFFU, false, &bad) &&
                selfTest_SdramPattern(sdram, 0xFFFFFFFFU, true, &bad);
  const uint32_t elapsedUs = selfTest_CyclesToUs(DWT->CYCCNT - started);
  passed &= selfTest_Check("8 MiB address pattern",
                           fillOk, fillOk ? "%lu us" : "bad at 0x%06lX",
                           fillOk ? elapsedUs : bad);
  if (fillOk && (elapsedUs > 0U))
  {
    /* 4 passes x 8 MiB = 32 MiB moved. */
    (void)selfTest_Check("CPU throughput", true, "%lu KiB/s",
                         (unsigned long)((32ULL * 1024ULL * 1000000ULL) /
                                         elapsedUs));
  }
  return passed;
}

static bool test_Dma2d(const SelfTest_Context_TypeDef *context)
{
  DMA2D_HandleTypeDef *dma2d = context->Dma2d;
  SDRAM_HandleTypeDef *sdram = context->Sdram;
  if ((dma2d == NULL) || (sdram == NULL) ||
      (HAL_DMA2D_GetState(dma2d) != HAL_DMA2D_STATE_READY))
  {
    return selfTest_Check("state ready", false, "%lu",
                          (dma2d != NULL) ? HAL_DMA2D_GetState(dma2d) : 0UL);
  }

  /* One output line, so the shared line-offset register is irrelevant and the
     display's DMA2D configuration is left untouched. */
  static const uint32_t colors[] = { 0xFF00FF00U, 0x12345678U };
  const uint32_t destination = SDRAM_START_ADDR + SELF_TEST_SCRATCH_OFFSET;
  bool passed = true;
  selfTest_CycleCounterStart();

  for (uint32_t pass = 0U; pass < 2U; pass++)
  {
    const uint32_t started = DWT->CYCCNT;
    const bool started_ok =
        HAL_DMA2D_Start(dma2d, colors[pass], destination,
                        SELF_TEST_DMA2D_PIXELS, 1U) == HAL_OK;
    const bool done = started_ok &&
        (HAL_DMA2D_PollForTransfer(dma2d, SELF_TEST_DMA2D_TIMEOUT_MS) == HAL_OK);
    const uint32_t elapsedUs = selfTest_CyclesToUs(DWT->CYCCNT - started);

    uint32_t mismatches = 0U;
    for (uint32_t index = 0U; done && (index < SELF_TEST_DMA2D_PIXELS); index++)
    {
      if (SDRAM_Read32b(sdram, SELF_TEST_SCRATCH_OFFSET +
                        (index * sizeof(uint32_t))) != colors[pass])
      {
        mismatches++;
      }
    }

    char label[32];
    (void)snprintf(label, sizeof(label), "R2M fill 0x%08lX", colors[pass]);
    passed &= selfTest_Check(label, done && (mismatches == 0U),
                             done ? "%lu us" : "error 0x%lX",
                             done ? elapsedUs : dma2d->ErrorCode);
  }

  passed &= selfTest_Check("state ready after",
                           HAL_DMA2D_GetState(dma2d) == HAL_DMA2D_STATE_READY,
                           "%lu", (unsigned long)HAL_DMA2D_GetState(dma2d));
  return passed;
}

static bool test_Ltdc(const SelfTest_Context_TypeDef *context)
{
  LTDC_HandleTypeDef *ltdc = context->Ltdc;
  if (ltdc == NULL)
  {
    return selfTest_Check("handle", false, "%lu", 0UL);
  }

  bool passed = true;
  passed &= selfTest_Check("HAL state ready",
                           HAL_LTDC_GetState(ltdc) == HAL_LTDC_STATE_READY,
                           "error 0x%lX", ltdc->ErrorCode);
  passed &= selfTest_Check("controller enabled",
                           (ltdc->Instance->GCR & LTDC_GCR_LTDCEN) != 0U,
                           "GCR=0x%08lX", ltdc->Instance->GCR);
  passed &= selfTest_Check("layer 1 enabled",
                           (LTDC_Layer1->CR & LTDC_LxCR_LEN) != 0U,
                           "CR=0x%08lX", LTDC_Layer1->CR);
  const uint32_t frame = LTDC_Layer1->CFBAR;
  passed &= selfTest_Check("layer 1 FB in SDRAM",
                           (frame >= SDRAM_START_ADDR) &&
                           (frame < (SDRAM_START_ADDR + SELF_TEST_SCRATCH_OFFSET)),
                           "0x%08lX", frame);

  /* Count vertical wrap-arounds of the current scan line. */
  uint32_t frames = 0U;
  uint32_t lastLine = ltdc->Instance->CPSR & LTDC_CPSR_CYPOS;
  const uint32_t startedAt = HAL_GetTick();
  while ((HAL_GetTick() - startedAt) < SELF_TEST_FRAME_WINDOW_MS)
  {
    const uint32_t line = ltdc->Instance->CPSR & LTDC_CPSR_CYPOS;
    if (line < lastLine)
    {
      frames++;
    }
    lastLine = line;
  }
  const uint32_t rateHz = frames * (1000U / SELF_TEST_FRAME_WINDOW_MS);
  passed &= selfTest_Check("refresh rate 50..70 Hz",
                           (rateHz >= 50U) && (rateHz <= 70U), "%lu Hz", rateHz);
  return passed;
}

static bool test_Dsi(const SelfTest_Context_TypeDef *context)
{
  DSI_HandleTypeDef *dsi = context->Dsi;
  if (dsi == NULL)
  {
    return selfTest_Check("handle", false, "%lu", 0UL);
  }

  bool passed = true;
  passed &= selfTest_Check("HAL state ready",
                           HAL_DSI_GetState(dsi) == HAL_DSI_STATE_READY,
                           "error 0x%lX", dsi->ErrorCode);
  passed &= selfTest_Check("regulator ready",
                           (dsi->Instance->WISR & DSI_WISR_RRS) != 0U,
                           "WISR=0x%08lX", dsi->Instance->WISR);
  passed &= selfTest_Check("D-PHY PLL locked",
                           (dsi->Instance->WISR & DSI_WISR_PLLLS) != 0U,
                           "WISR=0x%08lX", dsi->Instance->WISR);
  passed &= selfTest_Check("host + wrapper enabled",
                           ((dsi->Instance->CR & DSI_CR_EN) != 0U) &&
                           ((dsi->Instance->WCR & DSI_WCR_DSIEN) != 0U),
                           "WCR=0x%08lX", dsi->Instance->WCR);

  /* A DCS NOP exercises the low-power command path without changing state. */
  const bool nopOk = HAL_DSI_ShortWrite(dsi, 0U, DSI_DCS_SHORT_PKT_WRITE_P0,
                                        OTM8009A_CMD_NOP, 0U) == HAL_OK;
  passed &= selfTest_Check("DCS NOP to OTM8009A", nopOk, "error 0x%lX",
                           dsi->ErrorCode);

  /* ISR0/ISR1 are read-to-clear protocol/PHY error flags. */
  const uint32_t isr0 = dsi->Instance->ISR[0];
  const uint32_t isr1 = dsi->Instance->ISR[1];
  (void)selfTest_Check("error flags ISR0", true, "0x%08lX", isr0);
  (void)selfTest_Check("error flags ISR1", true, "0x%08lX", isr1);

  passed &= selfTest_Check("display driver unlocked",
                           (context->Display != NULL) &&
                           (context->Display->Lock == DISABLE),
                           "model %lu",
                           (context->Display != NULL) ?
                           (unsigned long)context->Display->Model : 0UL);
  return passed;
}

static bool test_Display(const SelfTest_Context_TypeDef *context)
{
  Display_TypeDef *display = context->Display;
  if ((display == NULL) || (display->Lock == ENABLE))
  {
    return selfTest_Check("display ready", false, "%lu", 0UL);
  }

  /* Eight vertical color bars, verified by reading the framebuffer back. */
  static const uint32_t bars[8] = {
    0xFFFFFFFFU, 0xFFFFFF00U, 0xFF00FFFFU, 0xFF00FF00U,
    0xFFFF00FFU, 0xFFFF0000U, 0xFF0000FFU, 0xFF000000U,
  };
  const uint16_t barWidth = (uint16_t)(display->Width / 8U);
  const LTDC_LayerCfgTypeDef *layer = display->Layer1;
  bool drawn = true;
  uint32_t mismatches = 0U;

  for (uint16_t bar = 0U; drawn && (bar < 8U); bar++)
  {
    drawn = Display_FillRectangle(display, L1, (uint16_t)(bar * barWidth), 0U,
                                  barWidth, display->Height, bars[bar]) == HAL_OK;
  }
  for (uint16_t bar = 0U; drawn && (bar < 8U); bar++)
  {
    const uint16_t x = (uint16_t)((bar * barWidth) + (barWidth / 2U));
    const uint16_t y = (uint16_t)(display->Height / 2U);
    if (*(__IO uint32_t *)GET_POSITION_ADDRESS(layer, x, y) != bars[bar])
    {
      mismatches++;
    }
  }
  bool passed = selfTest_Check("color bars drawn", drawn, "%lu bars", 8UL);
  passed &= selfTest_Check("framebuffer readback", drawn && (mismatches == 0U),
                           "%lu mismatches", mismatches);

  HAL_Delay(700U);
  passed &= selfTest_Check("background restored",
                           Display_FillLayer(display, L1, display->BgLayer1) ==
                           HAL_OK, "0x%08lX", display->BgLayer1);
  /* Repaint the touch UI, if it is running, over the restored background. */
  Ui_ShowScreen(Ui_GetScreen());
  return passed;
}

static bool test_Gpio(const SelfTest_Context_TypeDef *context)
{
  (void)context;
  bool passed = true;

  static const struct { GPIO_TypeDef *port; uint16_t pin; const char *label; }
  leds[] = {
    { LD_USER1_GPIO_Port, LD_USER1_Pin, "LD_USER1 (PJ13) set/reset" },
    { LD_USER2_GPIO_Port, LD_USER2_Pin, "LD_USER2 (PJ5) set/reset" },
  };
  for (uint32_t index = 0U; index < 2U; index++)
  {
    const GPIO_PinState saved = ((leds[index].port->ODR & leds[index].pin) != 0U) ?
                                GPIO_PIN_SET : GPIO_PIN_RESET;
    HAL_GPIO_WritePin(leds[index].port, leds[index].pin, GPIO_PIN_SET);
    const bool high = (leds[index].port->ODR & leds[index].pin) != 0U;
    HAL_GPIO_WritePin(leds[index].port, leds[index].pin, GPIO_PIN_RESET);
    const bool low = (leds[index].port->ODR & leds[index].pin) == 0U;
    HAL_GPIO_WritePin(leds[index].port, leds[index].pin, saved);
    passed &= selfTest_Check(leds[index].label, high && low, "ODR=0x%04lX",
                             leds[index].port->ODR);
  }

  passed &= selfTest_Check("backlight (PI14) high",
                           HAL_GPIO_ReadPin(LCD_BL_CTRL_GPIO_Port,
                                            LCD_BL_CTRL_Pin) == GPIO_PIN_SET,
                           "%lu", (unsigned long)HAL_GPIO_ReadPin(
                               LCD_BL_CTRL_GPIO_Port, LCD_BL_CTRL_Pin));
  passed &= selfTest_Check("DSI reset (PJ15) released",
                           HAL_GPIO_ReadPin(DSI_RESET_GPIO_Port,
                                            DSI_RESET_Pin) == GPIO_PIN_SET,
                           "%lu", (unsigned long)HAL_GPIO_ReadPin(
                               DSI_RESET_GPIO_Port, DSI_RESET_Pin));
  (void)selfTest_Check("user button (PA0)", true, "%lu",
                       (unsigned long)HAL_GPIO_ReadPin(B_USER_GPIO_Port,
                                                       B_USER_Pin));
  return passed;
}

/* -------------------------------------------------------------------------- */

static const SelfTest_Case_TypeDef selfTestCases[] = {
  { "core",    "device ID, caches, FPU, MPU regions",          true,  true,  test_Core },
  { "clock",   "oscillators, PLLs, bus clocks, SysTick",       true,  true,  test_Clock },
  { "usart",   "USART1 console state and baud rate",           true,  true,  test_Usart },
  { "crc",     "CRC unit known-answer test",                   true,  true,  test_Crc },
  { "i2c1",    "I2C1 bus scan (Arduino connector)",            true,  false, test_I2c1 },
  { "i2c4",    "I2C4 bus scan (touch, codec)",                 true,  false, test_I2c4 },
  { "touch",   "FT6206 identification and status",             true,  false, test_Touch },
  { "codec",   "WM8994 audio codec chip ID over I2C4",         true,  false, test_Codec },
  { "eth",     "Ethernet MAC/LAN8742 PHY identification",      true,  false, test_Ethernet },
  { "sdram",   "non-destructive SDRAM probe (top 64 bytes)",   true,  false, test_SdramProbe },
  { "dma2d",   "DMA2D register-to-memory fill into scratch",   true,  false, test_Dma2d },
  { "ltdc",    "LTDC enable, layer, refresh rate",             true,  false, test_Ltdc },
  { "dsi",     "DSI host PHY/PLL and DCS command path",        true,  false, test_Dsi },
  { "gpio",    "LEDs, backlight, DSI reset, user button",      true,  false, test_Gpio },
  { "dns",     "resolve " SELF_TEST_DNS_HOSTNAME " via DHCP DNS servers", false, false, test_Dns },
  { "ntp",     "SNTP synchronization with DHCP NTP server",    false, false, test_Ntp },
  { "sdramfull", "8 MiB scratch: data/address bus, pattern",   false, false, test_SdramFull },
  { "display", "color bars with framebuffer readback",         false, false, test_Display },
};

#define SELF_TEST_CASE_COUNT (sizeof(selfTestCases) / sizeof(selfTestCases[0]))

static void selfTest_RunCase(const SelfTest_Context_TypeDef *context,
                             const SelfTest_Case_TypeDef *testCase,
                             SelfTest_Result_TypeDef *result)
{
  printf("[TEST] %s - %s\r\n", testCase->Name, testCase->Description);
  const uint32_t startedAt = HAL_GetTick();
  const bool passed = testCase->Run(context);
  const uint32_t elapsedMs = HAL_GetTick() - startedAt;

  if (passed)
  {
    result->Passed++;
  }
  else
  {
    result->Failed++;
  }
  printf("[%s] %s (%lu ms)\r\n", passed ? "PASS" : "FAIL", testCase->Name,
         (unsigned long)elapsedMs);
}

static void selfTest_PrintSummary(const char *title,
                                  const SelfTest_Result_TypeDef *result)
{
  printf("-----------------------------------------------\r\n");
  printf("%s: %u passed, %u failed\r\n\r\n", title, result->Passed,
         result->Failed);
}

SelfTest_Result_TypeDef SelfTest_Run(const SelfTest_Context_TypeDef *context)
{
  SelfTest_Result_TypeDef result = {0U, 0U};
  printf("\r\nSTM32F769I-DISCO power-on self-test\r\n");
  printf("-----------------------------------------------\r\n");

  if (context == NULL)
  {
    result.Failed++;
    printf("[FAIL] test context\r\n");
    return result;
  }

  for (uint32_t index = 0U; index < SELF_TEST_CASE_COUNT; index++)
  {
    if (selfTestCases[index].Post)
    {
      selfTest_RunCase(context, &selfTestCases[index], &result);
    }
  }
  selfTest_PrintSummary("POST result", &result);
  return result;
}

static SelfTest_Result_TypeDef selfTest_RunPostPhase(
    const SelfTest_Context_TypeDef *context, bool early, const char *title)
{
  SelfTest_Result_TypeDef result = {0U, 0U};
  printf("\r\n%s\r\n", title);
  printf("-----------------------------------------------\r\n");
  if (context == NULL)
  {
    result.Failed++;
    printf("[FAIL] test context\r\n");
    return result;
  }
  for (uint32_t index = 0U; index < SELF_TEST_CASE_COUNT; index++)
  {
    if (selfTestCases[index].Post && (selfTestCases[index].Early == early))
    {
      selfTest_RunCase(context, &selfTestCases[index], &result);
    }
  }
  selfTest_PrintSummary(early ? "Early POST result" : "Peripheral POST result",
                        &result);
  return result;
}

SelfTest_Result_TypeDef SelfTest_RunEarly(const SelfTest_Context_TypeDef *context)
{
  return selfTest_RunPostPhase(context, true, "STM32F769I-DISCO early POST");
}

SelfTest_Result_TypeDef SelfTest_RunPeripheral(const SelfTest_Context_TypeDef *context)
{
  return selfTest_RunPostPhase(context, false,
                               "STM32F769I-DISCO peripheral POST");
}

HAL_StatusTypeDef SelfTest_RunByName(const SelfTest_Context_TypeDef *context,
                                     const char *name,
                                     SelfTest_Result_TypeDef *result)
{
  if ((context == NULL) || (name == NULL) || (result == NULL))
  {
    return HAL_ERROR;
  }

  if (strcmp(name, "post") == 0)
  {
    const SelfTest_Result_TypeDef post = SelfTest_Run(context);
    result->Passed += post.Passed;
    result->Failed += post.Failed;
    return HAL_OK;
  }

  const bool all = strcmp(name, "all") == 0;
  bool matched = false;
  for (uint32_t index = 0U; index < SELF_TEST_CASE_COUNT; index++)
  {
    if (all || (strcmp(name, selfTestCases[index].Name) == 0))
    {
      selfTest_RunCase(context, &selfTestCases[index], result);
      matched = true;
    }
  }

  if (all)
  {
    selfTest_PrintSummary("ALL result", result);
  }
  return matched ? HAL_OK : HAL_ERROR;
}

void SelfTest_PrintList(void)
{
  printf("  %-10s %-5s %s\r\n", "name", "POST", "description");
  for (uint32_t index = 0U; index < SELF_TEST_CASE_COUNT; index++)
  {
    printf("  %-10s %-5s %s\r\n", selfTestCases[index].Name,
           selfTestCases[index].Post ? "yes" : "no",
           selfTestCases[index].Description);
  }
  printf("  %-10s %-5s %s\r\n", "post", "", "run the POST suite");
  printf("  %-10s %-5s %s\r\n", "all", "", "run every test, including intrusive");
}
