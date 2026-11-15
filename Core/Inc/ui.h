#ifndef UI_H
#define UI_H

#include "common.h"

/** Screens of the touch user interface. */
typedef enum
{
  UI_SCREEN_HOME,         /**< Title, Network and Log buttons, status line. */
  UI_SCREEN_NETWORK,      /**< Ethernet/IP status with a Back button. */
  UI_SCREEN_LOG,          /**< Recent printf() output with a Back button. */
} Ui_ScreenTypeDef;

/**
  * @brief Bind the display and draw the home screen.
  * @param display (Display_TypeDef*) Non-null, unlocked display; the UI owns
  *        drawing on LTDC layer 1 from now on.
  * @retval (HAL_StatusTypeDef) HAL_OK, or HAL_ERROR for an unusable display.
  * @note  Thread/main-loop context only.
  */
HAL_StatusTypeDef Ui_Init(Display_TypeDef *display);

/**
  * @brief Handle one tap at display coordinates.
  * @param x (uint16_t) Horizontal position in pixels, 0 at the left edge.
  * @param y (uint16_t) Vertical position in pixels, 0 at the top edge. The
  *        display driver and FT6206 use a bottom-left origin; callers convert
  *        with y = Height - 1 - touchY.
  * @retval (bool) true when the tap hit a button.
  * @note  Switching screens redraws the whole layer (blocking, ~10 ms).
  */
bool Ui_HandleTap(uint16_t x, uint16_t y);

/**
  * @brief Show a screen directly, as if its button had been tapped.
  * @param screen (Ui_ScreenTypeDef) Screen to show.
  */
void Ui_ShowScreen(Ui_ScreenTypeDef screen);

/**
  * @brief Refresh dynamic content of the visible screen.
  * @note  Non-blocking when nothing changed; status fields refresh every
  *        0.5-1 s and the log at most every 100 ms, drawing only changed
  *        characters. Call continuously from the main loop.
  */
void Ui_Process(void);

/** @brief Currently visible screen. */
Ui_ScreenTypeDef Ui_GetScreen(void);

/** @brief Short constant name of a screen ("home", "network", "log"). */
const char *Ui_GetScreenName(Ui_ScreenTypeDef screen);

#endif /* UI_H */
