/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"
#include "app.h"
#include "console.h"
#include "fmc.h"
#include "ft6206.h"
#include "log_buffer.h"
#include "lwip.h"
#include "otm8009a.h"
#include "self_test.h"

#include <stdio.h>
#include <errno.h>

/** SWO bit rate; matches the cortex-debug swoFrequency in launch.json. */
#define ITM_SWO_FREQUENCY_HZ      1800000U
/** Upper bound on polling one ITM stimulus port before a character is dropped. */
#define ITM_PORT_READY_SPINS      10000U
/** Bounded UART transmit time per printf() chunk. */
#define STDOUT_UART_TIMEOUT_MS    100U
/** Bounded wait for one FMC SDRAM command to be accepted. */
#define SDRAM_COMMAND_TIMEOUT_MS  100U
/** SDRAM refresh counter value programmed into FMC_SDRTR. */
#define SDRAM_REFRESH_COUNT       1539U

CRC_HandleTypeDef hcrc;
DMA2D_HandleTypeDef hdma2d;
DSI_HandleTypeDef hdsi;
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c4;
LTDC_HandleTypeDef hltdc;
UART_HandleTypeDef huart1;
SDRAM_HandleTypeDef hsdram1;

void SystemClock_Config(void);
static void MPU_Config(void);
static void ITM_Init(void);
static void MX_GPIO_Init(void);
static void MX_CRC_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_DMA2D_Init(void);
static void MX_DSIHOST_DSI_Init(void);
static void MX_LTDC_Init(void);
static void MX_FMC_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C4_Init(void);

/**
  * @brief Enable ITM stimulus port 0 and route it to the SWO pin (PB3).
  * @note  The TPIU is configured here (asynchronous NRZ, formatter bypassed) so
  *        the ITM FIFO drains at ITM_SWO_FREQUENCY_HZ whether or not a trace
  *        probe is capturing. Without it, ITM_SendChar() never sees a free
  *        stimulus port and blocks forever unless a debugger set up the TPIU.
  *        Must run after SystemClock_Config() because the prescaler depends on
  *        SystemCoreClock.
  */
static void ITM_Init(void)
{
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DBGMCU->CR |= DBGMCU_CR_TRACE_IOEN;

  TPI->CSPSR = 1U;                                   /* 1-bit trace port. */
  TPI->SPPR = 2U;                                    /* Asynchronous NRZ. */
  TPI->ACPR = (SystemCoreClock / ITM_SWO_FREQUENCY_HZ) - 1U;
  TPI->FFCR = 0x100U;                                /* Formatter bypassed. */

  ITM->LAR = 0xC5ACCE55U;                            /* Unlock ITM. */
  ITM->TCR = (1U << ITM_TCR_TraceBusID_Pos) |
             ITM_TCR_ITMENA_Msk |
             ITM_TCR_SYNCENA_Msk |
             ITM_TCR_TSENA_Msk |
             ITM_TCR_SWOENA_Msk;
  ITM->TPR = 0U;
  ITM->TER = 0x1U;                                   /* Stimulus port 0. */
}

/** Write one character to ITM port 0, dropping it if the port stays busy. */
static void itm_WriteChar(char ch)
{
  if (((ITM->TCR & ITM_TCR_ITMENA_Msk) == 0U) || ((ITM->TER & 1U) == 0U))
  {
    return;
  }

  for (uint32_t spins = 0U; spins < ITM_PORT_READY_SPINS; spins++)
  {
    if (ITM->PORT[0U].u32 != 0U)
    {
      ITM->PORT[0U].u8 = (uint8_t)ch;
      return;
    }
  }
}

/**
  * @brief newlib stdout/stderr hook: mirror output to the on-screen log,
  *        ITM port 0, and USART1.
  * @note  Blocking for at most STDOUT_UART_TIMEOUT_MS per call. Thread context
  *        only; not reentrant with respect to interrupt handlers.
  */
int _write(int file, char *ptr, int len)
{
  (void)file;
  if ((ptr == NULL) || (len <= 0))
  {
    return 0;
  }

  /* On-screen log copy; rendered later by the UI in main-loop context. */
  LogBuffer_Write(ptr, (size_t)len);

  for (int i = 0; i < len; i++)
  {
    itm_WriteChar(ptr[i]);
  }

  int sent = 0;
  while (sent < len)
  {
    const uint16_t chunk = (uint16_t)(((len - sent) > UINT16_MAX) ?
                                      UINT16_MAX : (len - sent));
    if (HAL_UART_Transmit(&huart1, (uint8_t *)&ptr[sent], chunk,
                          STDOUT_UART_TIMEOUT_MS) != HAL_OK)
    {
      errno = EIO;
      return (sent > 0) ? sent : -1;
    }
    sent += chunk;
  }
  return len;
}

/**
  * @brief Firmware entry point: order-sensitive platform bring-up, then the
  *        bare-metal processing loop.
  */
int main(void)
{
  static SelfTest_Context_TypeDef selfTestContext = {
    .Crc = &hcrc,
    .Dma2d = &hdma2d,
    .Dsi = &hdsi,
    .I2c1 = &hi2c1,
    .I2c4 = &hi2c4,
    .Ltdc = &hltdc,
    .Uart = &huart1,
    .Sdram = &hsdram1,
  };

  MPU_Config();
  HAL_Init();
  SystemClock_Config();

  SCB_EnableICache();
  SCB_EnableDCache();

  MX_GPIO_Init();
  MX_CRC_Init();
  MX_USART1_UART_Init();
  /* Unbuffered stdout: every printf() reaches USART1/ITM immediately. */
  (void)setvbuf(stdout, NULL, _IONBF, 0);
  if (Console_Init(&huart1, &selfTestContext) != HAL_OK)
  {
    printf("BOOT: console RX unavailable\r\n");
  }
  printf("\r\nBOOT: USART1 ready\r\n");
  (void)SelfTest_RunEarly(&selfTestContext);

  printf("BOOT: initializing I2C\r\n");
  MX_I2C1_Init();
  MX_I2C4_Init();
  ITM_Init();

  printf("BOOT: initializing SDRAM and DMA2D\r\n");
  MX_DMA2D_Init();
  MX_FMC_Init();

  printf("BOOT: initializing display pipeline\r\n");
  MX_DSIHOST_DSI_Init();
  MX_LTDC_Init();
  HAL_DSI_Start(&hdsi);

  Display_TypeDef *display = OTM8009A_Init();
  printf("BOOT: display initialization %s\r\n",
         (display != NULL) && (display->Lock == DISABLE) ? "complete" : "failed");
  TouchScreen_TypeDef *touch = FT6206_Init();
  /* Non-blocking: PHY detection, link and DHCP are polled from the loop. */
  printf("BOOT: initializing Ethernet/lwIP\r\n");
  MX_LWIP_Init();

  selfTestContext.Display = display;
  selfTestContext.Touch = touch;

  /* The early phase already verified the core services used for reporting. */
  (void)SelfTest_RunPeripheral(&selfTestContext);

  if (App_Init(display, touch) != HAL_OK)
  {
    /* Keep the diagnostic console alive; App_Process() stays idle. */
    printf("App_Init failed: display or touchscreen not ready\r\n");
  }
  Console_PrintPrompt();

  while (1)
  {
    Console_Process();
    MX_LWIP_Process();
    App_Process();
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_HSI, RCC_MCODIV_1);
}

/**
  * @brief CRC Initialization Function
  * @param None
  * @retval None
  */
static void MX_CRC_Init(void)
{
  hcrc.Instance = CRC;
  hcrc.Init.DefaultPolynomialUse = DEFAULT_POLYNOMIAL_ENABLE;
  hcrc.Init.DefaultInitValueUse = DEFAULT_INIT_VALUE_ENABLE;
  hcrc.Init.InputDataInversionMode = CRC_INPUTDATA_INVERSION_NONE;
  hcrc.Init.OutputDataInversionMode = CRC_OUTPUTDATA_INVERSION_DISABLE;
  hcrc.InputDataFormat = CRC_INPUTDATA_FORMAT_BYTES;
  if (HAL_CRC_Init(&hcrc) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief DMA2D Initialization Function
  * @param None
  * @retval None
  */
static void MX_DMA2D_Init(void)
{
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_R2M;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_ARGB8888;
  hdma2d.Init.OutputOffset = 0;

  hdma2d.LayerCfg[0].InputOffset = 0;
  hdma2d.LayerCfg[0].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[0].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[0].InputAlpha = 0;
  hdma2d.LayerCfg[0].AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.LayerCfg[0].RedBlueSwap = DMA2D_RB_REGULAR;

  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 0;
  hdma2d.LayerCfg[1].AlphaInverted = DMA2D_REGULAR_ALPHA;
  hdma2d.LayerCfg[1].RedBlueSwap = DMA2D_RB_REGULAR;

  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK) Error_Handler();
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 0) != HAL_OK) Error_Handler();
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK) Error_Handler();
  /* Output address, color and size are programmed by each HAL_DMA2D_Start(). */
}

/**
  * @brief DSIHOST Initialization Function
  * @param None
  * @retval None
  */
static void MX_DSIHOST_DSI_Init(void)
{
  /* Reset */
  HAL_GPIO_WritePin(DSI_RESET_Port, DSI_RESET_Pin, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(DSI_RESET_Port, DSI_RESET_Pin, GPIO_PIN_SET);

  DSI_PLLInitTypeDef PLLInit = {0};
  DSI_VidCfgTypeDef VidCfg = {0};

  hdsi.Instance = DSI;
  hdsi.Init.AutomaticClockLaneControl = DSI_AUTO_CLK_LANE_CTRL_DISABLE;
  hdsi.Init.TXEscapeCkdiv = 4;
  hdsi.Init.NumberOfLanes = DSI_TWO_DATA_LANES;
  PLLInit.PLLNDIV = 100;
  PLLInit.PLLIDF = DSI_PLL_IN_DIV5;
  PLLInit.PLLODF = DSI_PLL_OUT_DIV1;

  if (HAL_DSI_Init(&hdsi, &PLLInit) != HAL_OK)
  {
    Error_Handler();
  }

  VidCfg.VirtualChannelID = 0;
  VidCfg.ColorCoding = DSI_RGB888;
  VidCfg.Mode = DSI_VID_MODE_BURST;
  VidCfg.PacketSize = DISPLAY_HEIGHT;
  VidCfg.NumberOfChunks = 0;
  VidCfg.NullPacketSize = 0x0fff;
  VidCfg.HSPolarity = DSI_HSYNC_ACTIVE_HIGH;
  VidCfg.VSPolarity = DSI_VSYNC_ACTIVE_HIGH;
  VidCfg.DEPolarity = DSI_DATA_ENABLE_ACTIVE_HIGH;
  VidCfg.HorizontalSyncActive = (_HSA_ * LANE_BYTE_CLOCK)/LCD_CLOCK;
  VidCfg.HorizontalBackPorch = (_HBP_ * LANE_BYTE_CLOCK)/LCD_CLOCK;
  VidCfg.HorizontalLine = ((DISPLAY_HEIGHT + _HSA_ + _HBP_ + _HFP_) * LANE_BYTE_CLOCK)/LCD_CLOCK;
  VidCfg.VerticalSyncActive = _VSA_;
  VidCfg.VerticalBackPorch = _VBP_;
  VidCfg.VerticalFrontPorch = _VFP_;
  VidCfg.VerticalActive = DISPLAY_WIDTH;
  VidCfg.LPCommandEnable = DSI_VMCR_LPCE;
  VidCfg.LPLargestPacketSize = 16;
  VidCfg.LPVACTLargestPacketSize = 0;
  VidCfg.LPHorizontalFrontPorchEnable = DSI_VMCR_LPHFPE;
  VidCfg.LPHorizontalBackPorchEnable = DSI_VMCR_LPHBPE;
  VidCfg.LPVerticalActiveEnable = DSI_VMCR_LPVAE;
  VidCfg.LPVerticalFrontPorchEnable = DSI_VMCR_LPVFPE;
  VidCfg.LPVerticalBackPorchEnable = DSI_VMCR_LPVBPE;
  VidCfg.LPVerticalSyncActiveEnable = DSI_VMCR_LPVSAE;
  VidCfg.FrameBTAAcknowledgeEnable = 0;

  if (HAL_DSI_ConfigVideoMode(&hdsi, &VidCfg) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LTDC_Init(void)
{
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AH;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AH;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;

  hltdc.Init.HorizontalSync       = _HSA_ - 1;
  hltdc.Init.VerticalSync         = _VSA_ - 1;

  hltdc.Init.AccumulatedHBP       = _HSA_ + _HBP_ - 1;
  hltdc.Init.AccumulatedVBP       = _VSA_ + _VBP_ - 1;

  hltdc.Init.AccumulatedActiveH   = DISPLAY_WIDTH + _VSA_ + _VBP_ - 1;
  hltdc.Init.AccumulatedActiveW   = DISPLAY_HEIGHT + _HSA_ + _HBP_ - 1;

  hltdc.Init.TotalHeigh           = DISPLAY_WIDTH + _VSA_ + _VBP_ + _VFP_ - 1;
  hltdc.Init.TotalWidth           = DISPLAY_HEIGHT + _HSA_ + _HBP_ + _HFP_ - 1;

  hltdc.Init.Backcolor.Blue = 0xff;
  hltdc.Init.Backcolor.Green = 0xff;
  hltdc.Init.Backcolor.Red = 0xff;

  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_NVIC_SetPriority(LTDC_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(LTDC_IRQn);
  if (HAL_LTDC_ProgramLineEvent(&hltdc, 0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

/* FMC initialization function */
static void MX_FMC_Init(void)
{
  FMC_SDRAM_TimingTypeDef SdramTiming = {0};

  /** Perform the SDRAM1 memory initialization sequence
  */
  hsdram1.Instance                  = FMC_SDRAM_DEVICE;
  /* hsdram1.Init */
  hsdram1.Init.SDBank               = FMC_SDRAM_BANK1;
  hsdram1.Init.ColumnBitsNumber     = FMC_SDRAM_COLUMN_BITS_NUM_8;
  hsdram1.Init.RowBitsNumber        = FMC_SDRAM_ROW_BITS_NUM_12;
  hsdram1.Init.MemoryDataWidth      = FMC_SDRAM_MEM_BUS_WIDTH_32;
  hsdram1.Init.InternalBankNumber   = FMC_SDRAM_INTERN_BANKS_NUM_4;
  hsdram1.Init.CASLatency           = FMC_SDRAM_CAS_LATENCY_3;
  hsdram1.Init.WriteProtection      = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
  hsdram1.Init.SDClockPeriod        = FMC_SDRAM_CLOCK_PERIOD_2;
  hsdram1.Init.ReadBurst            = FMC_SDRAM_RBURST_ENABLE;
  hsdram1.Init.ReadPipeDelay        = FMC_SDRAM_RPIPE_DELAY_0;

  SdramTiming.LoadToActiveDelay     = 1;  // tMRD
  SdramTiming.ExitSelfRefreshDelay  = 6;  // tXSR
  SdramTiming.SelfRefreshTime       = 3;  // tRAS
  SdramTiming.RowCycleDelay         = 6;  // tRC
  SdramTiming.WriteRecoveryTime     = 1;  // tWR
  SdramTiming.RPDelay               = 1;  // tRP
  SdramTiming.RCDDelay              = 1;  // tRCD

  if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK) Error_Handler();

  FMC_SDRAM_CommandTypeDef SdramCommand   = {0};

  SdramCommand.CommandMode                = SDRAM_CMD_CLK_ENABLE;
  SdramCommand.CommandTarget              = SDRAM_CMD_TARGET_BANK1;
  SdramCommand.AutoRefreshNumber          = 1;
  SdramCommand.ModeRegisterDefinition     = 0;
  if (HAL_SDRAM_SendCommand(&hsdram1, &SdramCommand, SDRAM_COMMAND_TIMEOUT_MS) != HAL_OK) Error_Handler();

  /* JEDEC power-up: keep the clock stable for at least 100 us before PALL. */
  HAL_Delay(1);

  SdramCommand.CommandMode                = SDRAM_CMD_PALL;
  if (HAL_SDRAM_SendCommand(&hsdram1, &SdramCommand, SDRAM_COMMAND_TIMEOUT_MS) != HAL_OK) Error_Handler();

  SdramCommand.CommandMode                = SDRAM_CMD_AUTOREFRESH_MODE;
  SdramCommand.AutoRefreshNumber          = 8;
  if (HAL_SDRAM_SendCommand(&hsdram1, &SdramCommand, SDRAM_COMMAND_TIMEOUT_MS) != HAL_OK) Error_Handler();

  SdramCommand.CommandMode                = SDRAM_CMD_LOAD_MODE;
  SdramCommand.AutoRefreshNumber          = 1;
  SdramCommand.ModeRegisterDefinition     = (
      SDRAM_MODEREG_BURST_LENGTH_1
    | SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL
    | SDRAM_MODEREG_CAS_LATENCY_3
    | SDRAM_MODEREG_OPERATING_MODE_STANDARD
    | SDRAM_MODEREG_WRITEBURST_MODE_SINGLE
  );
  if (HAL_SDRAM_SendCommand(&hsdram1, &SdramCommand, SDRAM_COMMAND_TIMEOUT_MS) != HAL_OK) Error_Handler();

  if (HAL_SDRAM_ProgramRefreshRate(&hsdram1, SDRAM_REFRESH_COUNT) != HAL_OK) Error_Handler();
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOJ_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOK_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOJ, LD_USER1_Pin|DSI_RESET_Pin|LD_USER2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : SAI1_FSA_Pin SAI1_SDB_Pin SAI1_SCKA_Pin SAI1_SDA_Pin */
  GPIO_InitStruct.Pin = SAI1_FSA_Pin|SAI1_SDB_Pin|SAI1_SCKA_Pin|SAI1_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SAI1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : QSPI_D2_Pin */
  GPIO_InitStruct.Pin = QSPI_D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(QSPI_D2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARDUINO_SCL_D15_Pin ARDUINO_SDA_D14_Pin */
  GPIO_InitStruct.Pin = ARDUINO_SCL_D15_Pin|ARDUINO_SDA_D14_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : ULPI_D7_Pin ULPI_D6_Pin ULPI_D5_Pin ULPI_D3_Pin
                           ULPI_D2_Pin ULPI_D1_Pin ULPI_D4_Pin */
  GPIO_InitStruct.Pin = ULPI_D7_Pin|ULPI_D6_Pin|ULPI_D5_Pin|ULPI_D3_Pin
                          |ULPI_D2_Pin|ULPI_D1_Pin|ULPI_D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : uSD_D3_Pin uSD_D2_Pin */
  GPIO_InitStruct.Pin = uSD_D3_Pin|uSD_D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_SDMMC2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : uSD_CMD_Pin uSD_CLK_Pin */
  GPIO_InitStruct.Pin = uSD_CMD_Pin|uSD_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_SDMMC2;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : WIFI_RX_Pin */
  GPIO_InitStruct.Pin = WIFI_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
  HAL_GPIO_Init(WIFI_RX_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CEC_Pin */
  GPIO_InitStruct.Pin = CEC_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF4_CEC;
  HAL_GPIO_Init(CEC_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : AUDIO_SDA_Pin */
  GPIO_InitStruct.Pin = AUDIO_SDA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_I2C4;
  HAL_GPIO_Init(AUDIO_SDA_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : QSPI_NCS_Pin */
  GPIO_InitStruct.Pin = QSPI_NCS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_QUADSPI;
  HAL_GPIO_Init(QSPI_NCS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD_USER1_Pin DSI_RESET_Pin LD_USER2_Pin */
  GPIO_InitStruct.Pin = LD_USER1_Pin|DSI_RESET_Pin|LD_USER2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

  /*Configure GPIO pins : Audio_INT_Pin WIFI_RST_Pin ARD_D8_Pin ARD_D7_Pin
                           ARD_D4_Pin ARD_D2_Pin */
  GPIO_InitStruct.Pin = Audio_INT_Pin|WIFI_RST_Pin|ARD_D8_Pin|ARD_D7_Pin
                          |ARD_D4_Pin|ARD_D2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);

  /*Configure GPIO pin : DFSDM_DATIN5_Pin */
  GPIO_InitStruct.Pin = DFSDM_DATIN5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF3_DFSDM1;
  HAL_GPIO_Init(DFSDM_DATIN5_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : QSPI_D1_Pin QSPI_D0_Pin */
  GPIO_InitStruct.Pin = QSPI_D1_Pin|QSPI_D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : ARD_D13_SCK_Pin */
  GPIO_InitStruct.Pin = ARD_D13_SCK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(ARD_D13_SCK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : NC4_Pin NC5_Pin uSD_Detect_Pin LCD_BL_CTRL_Pin */
  GPIO_InitStruct.Pin = NC4_Pin|NC5_Pin|uSD_Detect_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_BL_CTRL_Pin */
  GPIO_InitStruct.Pin = LCD_BL_CTRL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_BL_CTRL_GPIO_Port, &GPIO_InitStruct);

  // After init:
  HAL_GPIO_WritePin(LCD_BL_CTRL_GPIO_Port, LCD_BL_CTRL_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : NC3_Pin NC2_Pin NC1_Pin NC8_Pin
                           NC7_Pin */
  GPIO_InitStruct.Pin = NC3_Pin|NC2_Pin|NC1_Pin|NC8_Pin
                          |NC7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);

  /*Configure GPIO pin : SPDIF_RX_Pin */
  GPIO_InitStruct.Pin = SPDIF_RX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF7_SPDIFRX;
  HAL_GPIO_Init(SPDIF_RX_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : uSD_D1_Pin uSD_D0_Pin */
  GPIO_InitStruct.Pin = uSD_D1_Pin|uSD_D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF11_SDMMC2;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : RMII_RXER_Pin OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = RMII_RXER_Pin|OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pin : DFSDM_CKOUT_Pin */
  GPIO_InitStruct.Pin = DFSDM_CKOUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF3_DFSDM1;
  HAL_GPIO_Init(DFSDM_CKOUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI2_NSS_Pin */
  GPIO_InitStruct.Pin = SPI2_NSS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(SPI2_NSS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : WIFI_TX_Pin */
  GPIO_InitStruct.Pin = WIFI_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_UART5;
  HAL_GPIO_Init(WIFI_TX_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ULPI_DIR_Pin */
  GPIO_InitStruct.Pin = ULPI_DIR_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(ULPI_DIR_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CEC_CLK_Pin */
  GPIO_InitStruct.Pin = CEC_CLK_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(CEC_CLK_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ARD_D5_PWM_Pin */
  GPIO_InitStruct.Pin = ARD_D5_PWM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
  HAL_GPIO_Init(ARD_D5_PWM_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARD_D0_RX_Pin ARDUINO_TX_D1_Pin */
  GPIO_InitStruct.Pin = ARD_D0_RX_Pin|ARDUINO_TX_D1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : ULPI_NXT_Pin */
  GPIO_InitStruct.Pin = ULPI_NXT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(ULPI_NXT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : SAI1_MCLKA_Pin */
  GPIO_InitStruct.Pin = SAI1_MCLKA_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SAI1;
  HAL_GPIO_Init(SAI1_MCLKA_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : EXT_SDA_Pin EXT_SCL_Pin */
  GPIO_InitStruct.Pin = EXT_SDA_Pin|EXT_SCL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : ARD_D6_PWM_Pin */
  GPIO_InitStruct.Pin = ARD_D6_PWM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF3_TIM11;
  HAL_GPIO_Init(ARD_D6_PWM_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ARD_D3_PWM_Pin */
  GPIO_InitStruct.Pin = ARD_D3_PWM_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF3_TIM10;
  HAL_GPIO_Init(ARD_D3_PWM_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARDUINO_A1_Pin ARDUINO_A2_Pin ARDUINO_A3_Pin */
  GPIO_InitStruct.Pin = ARDUINO_A1_Pin|ARDUINO_A2_Pin|ARDUINO_A3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pin : ULPI_STP_Pin */
  GPIO_InitStruct.Pin = ULPI_STP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(ULPI_STP_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : ARD_A2_Pin */
  GPIO_InitStruct.Pin = ARD_A2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ARD_A2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : AUDIO_SCL_Pin */
  GPIO_InitStruct.Pin = AUDIO_SCL_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF4_I2C4;
  HAL_GPIO_Init(AUDIO_SCL_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : QSPI_D3_Pin */
  GPIO_InitStruct.Pin = QSPI_D3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF9_QUADSPI;
  HAL_GPIO_Init(QSPI_D3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : B_USER_Pin */
  GPIO_InitStruct.Pin = B_USER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B_USER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARD_A1_Pin ARD_A0_Pin */
  GPIO_InitStruct.Pin = ARD_A1_Pin|ARD_A0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : SPDIF_TX_Pin */
  GPIO_InitStruct.Pin = SPDIF_TX_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF10_SAI2;
  HAL_GPIO_Init(SPDIF_TX_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PH7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : ULPI_CLK_Pin ULPI_D0_Pin */
  GPIO_InitStruct.Pin = ULPI_CLK_Pin|ULPI_D0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_HS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : ARDUINO_PWM_D6_Pin */
  GPIO_InitStruct.Pin = ARDUINO_PWM_D6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF9_TIM12;
  HAL_GPIO_Init(ARDUINO_PWM_D6_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ARDUINO_MISO_D12_Pin ARDUINO_MOSI_PWM_D11_Pin */
  GPIO_InitStruct.Pin = ARDUINO_MISO_D12_Pin|ARDUINO_MOSI_PWM_D11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : DSI Reset Pin PJ15 */
  GPIO_InitStruct.Pin = DSI_RESET_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(DSI_RESET_GPIO_Port, &GPIO_InitStruct);
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.Timing = 0x20404768;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C4_Init(void)
{
  hi2c4.Instance = I2C4;
  hi2c4.Init.Timing = 0x20404768;
  hi2c4.Init.OwnAddress1 = 0;
  hi2c4.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c4.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c4.Init.OwnAddress2 = 0;
  hi2c4.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c4.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c4.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Analogue filter
  */
  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c4, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Digital filter
  */
  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c4, 0) != HAL_OK)
  {
    Error_Handler();
  }
}

/** Configure non-cacheable MPU regions for Ethernet DMA memory and SDRAM. */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStructETH = {0};
  MPU_Region_InitTypeDef MPU_InitStructDMA = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStructETH.Enable = MPU_REGION_ENABLE;
  MPU_InitStructETH.Number = MPU_REGION_NUMBER0;
  MPU_InitStructETH.BaseAddress = 0x20020000;
  MPU_InitStructETH.Size = MPU_REGION_SIZE_64KB;
  MPU_InitStructETH.SubRegionDisable = 0x0;
  MPU_InitStructETH.TypeExtField = MPU_TEX_LEVEL1;
  MPU_InitStructETH.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStructETH.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStructETH.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStructETH.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStructETH.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStructETH);

  /* External SDRAM: 0xC0000000, 16 MiB, non-cacheable for LTDC/DMA2D. */
  MPU_InitStructDMA.Enable           = MPU_REGION_ENABLE;
  MPU_InitStructDMA.Number           = MPU_REGION_NUMBER1;   // keep REGION0 for ETH
  MPU_InitStructDMA.BaseAddress      = SDRAM_START_ADDR;
  MPU_InitStructDMA.Size             = MPU_REGION_SIZE_16MB;
  MPU_InitStructDMA.SubRegionDisable = 0x00;
  MPU_InitStructDMA.TypeExtField     = MPU_TEX_LEVEL0;
  MPU_InitStructDMA.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStructDMA.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;

  /* Key attributes for LTDC/DMA2D */
  MPU_InitStructDMA.IsShareable      = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStructDMA.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStructDMA.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStructDMA);

  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  __disable_irq();
  HAL_GPIO_WritePin(LD_USER1_GPIO_Port, LD_USER1_Pin, GPIO_PIN_SET);
  while (1)
  {
  }
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}
#endif /* USE_FULL_ASSERT */
