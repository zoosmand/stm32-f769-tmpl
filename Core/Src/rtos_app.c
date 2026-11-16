#include "rtos_app.h"

#include "FreeRTOS.h"
#include "app.h"
#include "console.h"
#include "lwip.h"
#include "main.h"
#include "task.h"

#include <stdio.h>

#define PLATFORM_TASK_STACK_WORDS  1536U
#define PLATFORM_TASK_PRIORITY     (tskIDLE_PRIORITY + 3U)
#define PLATFORM_TASK_PERIOD_MS    1U

static StaticTask_t platformTaskControlBlock;
static StackType_t platformTaskStack[PLATFORM_TASK_STACK_WORDS];
static TaskHandle_t platformTaskHandle;

static void rtos_PlatformTask(void *argument)
{
  (void)argument;
  TickType_t lastWake = xTaskGetTickCount();

  printf("FreeRTOS: scheduler started, tick %lu Hz\r\n",
         (unsigned long)configTICK_RATE_HZ);
  Console_PrintPrompt();

  for (;;)
  {
    Console_Process();
    MX_LWIP_Process();
    App_Process();
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(PLATFORM_TASK_PERIOD_MS));
  }
}

void Rtos_Start(void)
{
  /* heap_4 initializes lazily on its first allocation. Probe and release one
     block so diagnostics report the available heap even though startup tasks
     themselves use static storage. */
  void *heapProbe = pvPortMalloc(1U);
  if (heapProbe == NULL)
  {
    Error_Handler();
  }
  vPortFree(heapProbe);

  platformTaskHandle = xTaskCreateStatic(
      rtos_PlatformTask, "platform", PLATFORM_TASK_STACK_WORDS, NULL,
      PLATFORM_TASK_PRIORITY, platformTaskStack, &platformTaskControlBlock);
  if (platformTaskHandle == NULL)
  {
    Error_Handler();
  }

  vTaskStartScheduler();
  Error_Handler();
}

void Rtos_PrintStatus(void)
{
  const char *state = "not started";
  const BaseType_t scheduler = xTaskGetSchedulerState();
  if (scheduler == taskSCHEDULER_RUNNING)
  {
    state = "running";
  }
  else if (scheduler == taskSCHEDULER_SUSPENDED)
  {
    state = "suspended";
  }

  printf("scheduler  %s\r\n", state);
  printf("tick       %lu ms\r\n", (unsigned long)xTaskGetTickCount());
  printf("heap free  %lu / %lu bytes\r\n",
         (unsigned long)xPortGetFreeHeapSize(),
         (unsigned long)configTOTAL_HEAP_SIZE);
  printf("heap low   %lu bytes\r\n",
         (unsigned long)xPortGetMinimumEverFreeHeapSize());
  if (platformTaskHandle != NULL)
  {
    const UBaseType_t words = uxTaskGetStackHighWaterMark(platformTaskHandle);
    printf("stack low  %lu words (%lu bytes)\r\n", (unsigned long)words,
           (unsigned long)(words * sizeof(StackType_t)));
  }
}

void Rtos_AssertFailed(const char *file, int line)
{
  taskDISABLE_INTERRUPTS();
  printf("FreeRTOS assert: %s:%d\r\n", file, line);
  for (;;)
  {
    if ((CoreDebug->DHCSR & CoreDebug_DHCSR_C_DEBUGEN_Msk) != 0U)
    {
      __BKPT(0);
    }
    __WFI();
  }
}

void vApplicationMallocFailedHook(void)
{
  Rtos_AssertFailed("malloc", 0);
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *taskName)
{
  (void)task;
  (void)taskName;
  Rtos_AssertFailed("stack overflow", 0);
}

void vApplicationGetIdleTaskMemory(StaticTask_t **taskControlBlock,
                                   StackType_t **stackBuffer,
                                   configSTACK_DEPTH_TYPE *stackSize)
{
  static StaticTask_t idleTaskControlBlock;
  static StackType_t idleTaskStack[configMINIMAL_STACK_SIZE];
  *taskControlBlock = &idleTaskControlBlock;
  *stackBuffer = idleTaskStack;
  *stackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationGetTimerTaskMemory(StaticTask_t **taskControlBlock,
                                    StackType_t **stackBuffer,
                                    configSTACK_DEPTH_TYPE *stackSize)
{
  static StaticTask_t timerTaskControlBlock;
  static StackType_t timerTaskStack[configTIMER_TASK_STACK_DEPTH];
  *taskControlBlock = &timerTaskControlBlock;
  *stackBuffer = timerTaskStack;
  *stackSize = configTIMER_TASK_STACK_DEPTH;
}
