#include "console.h"

#include "main.h"
#include "otm8009a.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CONSOLE_RX_BUFFER_SIZE  256U   /* Power of two. */
#define CONSOLE_LINE_LENGTH     64U
#define CONSOLE_MAX_ARGUMENTS   4U

/* Reception state shared with the USART1 interrupt.
   Producer: HAL_UART_RxCpltCallback() (ISR) writes consoleRxHead.
   Consumer: Console_Process() (thread) writes consoleRxTail.
   Single-producer/single-consumer with aligned 32-bit indices, so no lock is
   needed on Cortex-M7; the indices are volatile so each side observes the
   other's updates. */
static uint8_t consoleRxBuffer[CONSOLE_RX_BUFFER_SIZE];
static volatile uint32_t consoleRxHead;
static volatile uint32_t consoleRxTail;
static volatile uint32_t consoleRxDropped;
static uint8_t consoleRxByte;

static UART_HandleTypeDef *consoleUart;
static SelfTest_Context_TypeDef *consoleContext;

static char consoleLine[CONSOLE_LINE_LENGTH];
static uint32_t consoleLineLength;
static bool consoleLastWasCr;

/** Arm a one-byte interrupt reception. ISR- and thread-safe with this HAL. */
static HAL_StatusTypeDef console_ArmReception(void)
{
  return HAL_UART_Receive_IT(consoleUart, &consoleRxByte, 1U);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if ((consoleUart == NULL) || (huart != consoleUart))
  {
    return;
  }

  const uint32_t head = consoleRxHead;
  const uint32_t next = (head + 1U) & (CONSOLE_RX_BUFFER_SIZE - 1U);
  if (next != consoleRxTail)
  {
    consoleRxBuffer[head] = consoleRxByte;
    consoleRxHead = next;
  }
  else
  {
    consoleRxDropped++;
  }
  (void)console_ArmReception();
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if ((consoleUart == NULL) || (huart != consoleUart))
  {
    return;
  }
  /* Overrun aborts HAL reception; framing/noise errors leave it running. */
  if (huart->RxState == HAL_UART_STATE_READY)
  {
    (void)console_ArmReception();
  }
}

HAL_StatusTypeDef Console_Init(UART_HandleTypeDef *uart,
                               SelfTest_Context_TypeDef *context)
{
  if ((uart == NULL) || (context == NULL))
  {
    return HAL_ERROR;
  }

  consoleContext = context;
  consoleRxHead = 0U;
  consoleRxTail = 0U;
  consoleLineLength = 0U;
  consoleUart = uart;
  return console_ArmReception();
}

void Console_PrintPrompt(void)
{
  printf("f769> ");
}

/* -------------------------------------------------------------------------- */

static void console_Help(void)
{
  printf("Commands:\r\n");
  printf("  help                      this text\r\n");
  printf("  list                      list peripheral tests\r\n");
  printf("  test <name|post|all>      run test(s); see 'list'\r\n");
  printf("  led <1|2> <on|off|toggle> drive a user LED\r\n");
  printf("  bright <0-255>            OTM8009A display brightness (DCS)\r\n");
  printf("  info                      clocks, uptime, device state\r\n");
  printf("  reset                     system reset\r\n");
}

static void console_Test(uint32_t argc, char *argv[])
{
  if (argc < 2U)
  {
    printf("usage: test <name|post|all>\r\n");
    return;
  }

  for (uint32_t index = 1U; index < argc; index++)
  {
    SelfTest_Result_TypeDef result = {0U, 0U};
    if (SelfTest_RunByName(consoleContext, argv[index], &result) != HAL_OK)
    {
      printf("unknown test '%s'; try 'list'\r\n", argv[index]);
      continue;
    }
    printf("RESULT %s: %u passed, %u failed\r\n", argv[index], result.Passed,
           result.Failed);
  }
}

static void console_Led(uint32_t argc, char *argv[])
{
  if (argc != 3U)
  {
    printf("usage: led <1|2> <on|off|toggle>\r\n");
    return;
  }

  GPIO_TypeDef *port;
  uint16_t pin;
  if (strcmp(argv[1], "1") == 0)
  {
    port = LD_USER1_GPIO_Port;
    pin = LD_USER1_Pin;
  }
  else if (strcmp(argv[1], "2") == 0)
  {
    port = LD_USER2_GPIO_Port;
    pin = LD_USER2_Pin;
  }
  else
  {
    printf("unknown LED '%s'\r\n", argv[1]);
    return;
  }

  if (strcmp(argv[2], "on") == 0)
  {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_SET);
  }
  else if (strcmp(argv[2], "off") == 0)
  {
    HAL_GPIO_WritePin(port, pin, GPIO_PIN_RESET);
  }
  else if (strcmp(argv[2], "toggle") == 0)
  {
    HAL_GPIO_TogglePin(port, pin);
  }
  else
  {
    printf("unknown action '%s'\r\n", argv[2]);
    return;
  }
  printf("LED%s %s\r\n", argv[1],
         ((port->ODR & pin) != 0U) ? "on" : "off");
}

static void console_Brightness(uint32_t argc, char *argv[])
{
  char *end = NULL;
  const unsigned long value = (argc == 2U) ? strtoul(argv[1], &end, 0) : 256UL;
  if ((argc != 2U) || (end == argv[1]) || (*end != '\0') || (value > 255UL))
  {
    printf("usage: bright <0-255>\r\n");
    return;
  }

  Display_TypeDef *display = consoleContext->Display;
  if ((display == NULL) || (display->Lock == ENABLE))
  {
    printf("display not ready\r\n");
    return;
  }
  const HAL_StatusTypeDef status = OTM8009A_SetBrightness(display,
                                                          (uint8_t)value);
  printf("brightness %lu: %s\r\n", value, (status == HAL_OK) ? "ok" : "error");
}

static void console_Info(void)
{
  printf("build      %s %s\r\n", __DATE__, __TIME__);
  printf("SYSCLK     %lu Hz, HCLK %lu Hz, PCLK1 %lu Hz, PCLK2 %lu Hz\r\n",
         HAL_RCC_GetSysClockFreq(), HAL_RCC_GetHCLKFreq(),
         HAL_RCC_GetPCLK1Freq(), HAL_RCC_GetPCLK2Freq());
  printf("uptime     %lu ms\r\n", HAL_GetTick());
  printf("device     ID 0x%03lX rev 0x%04lX\r\n", HAL_GetDEVID(),
         HAL_GetREVID());

  const Display_TypeDef *display = consoleContext->Display;
  if (display != NULL)
  {
    printf("display    OTM%u %ux%u %s\r\n", display->Model, display->Width,
           display->Height, (display->Lock == DISABLE) ? "ready" : "locked");
  }
  const TouchScreen_TypeDef *touch = consoleContext->Touch;
  if (touch != NULL)
  {
    printf("touch      FT%u state %u last x=%u y=%u\r\n", touch->Model,
           (unsigned)touch->State, touch->Context->X, touch->Context->Y);
  }
  printf("console    %lu bytes dropped\r\n", (unsigned long)consoleRxDropped);
}

static void console_Execute(char *line)
{
  char *argv[CONSOLE_MAX_ARGUMENTS];
  uint32_t argc = 0U;
  char *save = NULL;

  for (char *token = strtok_r(line, " \t", &save);
       (token != NULL) && (argc < CONSOLE_MAX_ARGUMENTS);
       token = strtok_r(NULL, " \t", &save))
  {
    argv[argc++] = token;
  }
  if (argc == 0U)
  {
    return;
  }

  if ((strcmp(argv[0], "help") == 0) || (strcmp(argv[0], "?") == 0))
  {
    console_Help();
  }
  else if (strcmp(argv[0], "list") == 0)
  {
    SelfTest_PrintList();
  }
  else if (strcmp(argv[0], "test") == 0)
  {
    console_Test(argc, argv);
  }
  else if (strcmp(argv[0], "led") == 0)
  {
    console_Led(argc, argv);
  }
  else if (strcmp(argv[0], "bright") == 0)
  {
    console_Brightness(argc, argv);
  }
  else if (strcmp(argv[0], "info") == 0)
  {
    console_Info();
  }
  else if (strcmp(argv[0], "reset") == 0)
  {
    printf("resetting\r\n");
    HAL_Delay(10U);
    NVIC_SystemReset();
  }
  else
  {
    printf("unknown command '%s'; type 'help'\r\n", argv[0]);
  }
}

void Console_Process(void)
{
  if (consoleUart == NULL)
  {
    return;
  }

  /* Recover if an overrun aborted reception between interrupts. */
  if (consoleUart->RxState == HAL_UART_STATE_READY)
  {
    (void)console_ArmReception();
  }

  while (consoleRxTail != consoleRxHead)
  {
    const uint32_t tail = consoleRxTail;
    const char ch = (char)consoleRxBuffer[tail];
    consoleRxTail = (tail + 1U) & (CONSOLE_RX_BUFFER_SIZE - 1U);

    if ((ch == '\r') || (ch == '\n'))
    {
      const bool crlfPair = (ch == '\n') && consoleLastWasCr;
      consoleLastWasCr = (ch == '\r');
      if (crlfPair)
      {
        continue;
      }
      printf("\r\n");
      consoleLine[consoleLineLength] = '\0';
      console_Execute(consoleLine);
      consoleLineLength = 0U;
      Console_PrintPrompt();
      continue;
    }
    consoleLastWasCr = false;

    if ((ch == '\b') || (ch == 0x7F))
    {
      if (consoleLineLength > 0U)
      {
        consoleLineLength--;
        printf("\b \b");
      }
    }
    else if ((ch >= ' ') && (ch <= '~') &&
             (consoleLineLength < (CONSOLE_LINE_LENGTH - 1U)))
    {
      consoleLine[consoleLineLength++] = ch;
      printf("%c", ch);
    }
  }
}
