#ifndef CONSOLE_H
#define CONSOLE_H

#include "common.h"
#include "self_test.h"

/**
  * @brief Start interrupt-driven command reception on the console UART.
  * @param uart (UART_HandleTypeDef*) Non-null initialized UART in TX/RX mode
  *        whose IRQ calls HAL_UART_IRQHandler().
  * @param context (SelfTest_Context_TypeDef*) Non-null devices for the test
  *        commands; must have static lifetime.
  * @retval (HAL_StatusTypeDef) HAL_OK when reception is armed; otherwise the
  *         HAL error returned while arming.
  * @note  Call once from thread context after the devices are initialized.
  */
HAL_StatusTypeDef Console_Init(UART_HandleTypeDef *uart,
                               SelfTest_Context_TypeDef *context);

/**
  * @brief Echo received characters and execute any completed command line.
  * @note  Returns immediately when no input is pending. Commands such as
  *        "test" block while they run and print through printf(). Call from
  *        the main loop or a single owning task, never from an interrupt.
  */
void Console_Process(void);

/** @brief Print the command prompt. */
void Console_PrintPrompt(void);

#endif /* CONSOLE_H */
