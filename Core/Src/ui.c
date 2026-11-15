#include "ui.h"

#include "fonts.h"
#include "log_buffer.h"
#include "lwip.h"
#include "otm8009a.h"

#include <stdio.h>
#include <string.h>

/* Colors (ARGB8888, opaque). */
#define UI_COLOR_BACKGROUND     0xFF0B1320U
#define UI_COLOR_TITLE_BAR      0xFF1B3358U
#define UI_COLOR_TEXT           0xFFFFFFFFU
#define UI_COLOR_LABEL          0xFF9FB3C8U
#define UI_COLOR_OK             0xFF4CD964U
#define UI_COLOR_WARNING        0xFFFFB020U
#define UI_COLOR_BUTTON         0xFF2D6CDFU
#define UI_COLOR_BUTTON_BORDER  0xFFFFFFFFU
#define UI_COLOR_LOG_BACKGROUND 0xFF000000U
#define UI_COLOR_LOG_TEXT       0xFFB8F5B8U

/* Layout in pixels for the 800 x 480 landscape display. */
#define UI_TITLE_BAR_HEIGHT     56U
#define UI_TAP_MARGIN           8U
#define UI_BORDER_WIDTH         3U

#define UI_LOG_ORIGIN_X         4U
#define UI_LOG_ORIGIN_Y         64U
#define UI_LOG_CELL_WIDTH       12U    /* font_dot_10x14 cell. */
#define UI_LOG_CELL_HEIGHT      16U
#define UI_LOG_ROWS             26U
#define UI_LOG_COLUMNS          LOG_BUFFER_COLUMNS

#define UI_NET_FIRST_ROW_Y      80U
#define UI_NET_ROW_STEP         36U
#define UI_NET_LABEL_X          60U
#define UI_NET_VALUE_X          320U
#define UI_NET_VALUE_COLUMNS    26U
#define UI_NET_ROWS             9U

#define UI_HOME_STATUS_Y        420U
#define UI_HOME_STATUS_COLUMNS  44U

/* Refresh periods in milliseconds. */
#define UI_STATUS_PERIOD_MS     1000U
#define UI_NETWORK_PERIOD_MS    500U
#define UI_LOG_PERIOD_MS        100U

/** One rectangular touch button and the screen it opens. */
typedef struct
{
  uint16_t x;
  uint16_t y;
  uint16_t width;
  uint16_t height;
  const char *label;
  const Font_TypeDef *font;
  Ui_ScreenTypeDef target;
} UiButton_TypeDef;

/* Glyph geometry of the bitmap fonts (cell size includes spacing). */
static const Font_TypeDef uiFontSmall = {
  .Width = 12U, .Height = 16U, .BytesPerGlif = 24U,
  .Font = (const uint8_t *)font_dot_10x14,
};
static const Font_TypeDef uiFontMedium = {
  .Width = 18U, .Height = 24U, .BytesPerGlif = 54U,
  .Font = (const uint8_t *)font_dot_15x21,
};
static const Font_TypeDef uiFontLarge = {
  .Width = 24U, .Height = 32U, .BytesPerGlif = 96U,
  .Font = (const uint8_t *)font_dot_20x28,
};

static const UiButton_TypeDef uiHomeButtons[] = {
  { 90U, 150U, 280U, 150U, "NETWORK", &uiFontLarge, UI_SCREEN_NETWORK },
  { 430U, 150U, 280U, 150U, "LOG", &uiFontLarge, UI_SCREEN_LOG },
};
static const UiButton_TypeDef uiBackButton = {
  10U, 8U, 130U, 40U, "< BACK", &uiFontMedium, UI_SCREEN_HOME,
};

static const char *const uiNetworkLabels[UI_NET_ROWS] = {
  "Link", "Speed", "DHCP", "IP address", "Netmask", "Gateway", "DNS",
  "MAC address", "Uptime",
};

static Display_TypeDef *uiDisplay;
static Ui_ScreenTypeDef uiScreen = UI_SCREEN_HOME;
static uint32_t uiLastRefresh;
static uint32_t uiLogSequence;
static char uiLogShown[UI_LOG_ROWS][UI_LOG_COLUMNS];
static char uiNetworkShown[UI_NET_ROWS][UI_NET_VALUE_COLUMNS + 1U];

/* The display driver's origin is the bottom-left corner (glyphs grow
   upward). The UI lays out in conventional top-left coordinates and converts
   here, at the single drawing boundary. */

/** Display (bottom-origin) row of the lowest line of a box whose top is yTop. */
static uint16_t ui_ToDisplayY(uint16_t yTop, uint16_t height)
{
  return (uint16_t)(uiDisplay->Height - yTop - height);
}

/** Fill a box given by its top-left corner. */
static void ui_FillBox(uint16_t x, uint16_t yTop, uint16_t width,
                       uint16_t height, uint32_t color)
{
  (void)Display_FillRectangle(uiDisplay, L1, x, ui_ToDisplayY(yTop, height),
                              width, height, color);
}

/** Draw a string with its top-left corner at (x, yTop), padded with spaces to
    padColumns cells. */
static void ui_DrawText(uint16_t x, uint16_t yTop, const Font_TypeDef *font,
                        uint32_t color, uint32_t background, const char *text,
                        uint16_t padColumns)
{
  uint16_t y = ui_ToDisplayY(yTop, font->Height);
  Font_TypeDef glyphs = *font;
  glyphs.Color = color;
  glyphs.Bgcolor = background;

  uint16_t columns = 0U;
  for (; (text != NULL) && (*text != '\0'); text++, columns++)
  {
    const uint8_t ch = ((*text >= ' ') && (*text <= '~')) ? (uint8_t)*text : '?';
    if (Display_DrawSymbol(uiDisplay, L1, &x, &y, &glyphs, ch) != HAL_OK)
    {
      return;
    }
  }
  for (; columns < padColumns; columns++)
  {
    if (Display_DrawSymbol(uiDisplay, L1, &x, &y, &glyphs, ' ') != HAL_OK)
    {
      return;
    }
  }
}

static uint16_t ui_TextWidth(const Font_TypeDef *font, const char *text)
{
  return (uint16_t)(strlen(text) * font->Width);
}

static void ui_DrawButton(const UiButton_TypeDef *button)
{
  const uint16_t right = (uint16_t)(button->x + button->width - UI_BORDER_WIDTH);
  const uint16_t bottom = (uint16_t)(button->y + button->height - UI_BORDER_WIDTH);
  ui_FillBox(button->x, button->y, button->width, button->height,
             UI_COLOR_BUTTON);
  ui_FillBox(button->x, button->y, button->width, UI_BORDER_WIDTH,
             UI_COLOR_BUTTON_BORDER);
  ui_FillBox(button->x, bottom, button->width, UI_BORDER_WIDTH,
             UI_COLOR_BUTTON_BORDER);
  ui_FillBox(button->x, button->y, UI_BORDER_WIDTH, button->height,
             UI_COLOR_BUTTON_BORDER);
  ui_FillBox(right, button->y, UI_BORDER_WIDTH, button->height,
             UI_COLOR_BUTTON_BORDER);
  const uint16_t textX = (uint16_t)(button->x +
      ((button->width - ui_TextWidth(button->font, button->label)) / 2U));
  const uint16_t textY = (uint16_t)(button->y +
      ((button->height - button->font->Height) / 2U));
  ui_DrawText(textX, textY, button->font, UI_COLOR_TEXT, UI_COLOR_BUTTON,
              button->label, 0U);
}

static bool ui_IsHit(const UiButton_TypeDef *button, uint16_t x, uint16_t y)
{
  return ((x + UI_TAP_MARGIN) >= button->x) &&
         (x <= (button->x + button->width + UI_TAP_MARGIN)) &&
         ((y + UI_TAP_MARGIN) >= button->y) &&
         (y <= (button->y + button->height + UI_TAP_MARGIN));
}

/** Clear the layer and draw a title bar, optionally with a Back button. */
static void ui_DrawFrame(const char *title, bool backButton)
{
  (void)Display_FillLayer(uiDisplay, L1, UI_COLOR_BACKGROUND);
  ui_FillBox(0U, 0U, uiDisplay->Width, UI_TITLE_BAR_HEIGHT, UI_COLOR_TITLE_BAR);

  const uint16_t left = backButton ?
      (uint16_t)(uiBackButton.x + uiBackButton.width) : 0U;
  const uint16_t space = (uint16_t)(uiDisplay->Width - left);
  const uint16_t titleX = (uint16_t)(left +
      ((space - ui_TextWidth(&uiFontLarge, title)) / 2U));
  ui_DrawText(titleX, 12U, &uiFontLarge, UI_COLOR_TEXT, UI_COLOR_TITLE_BAR,
              title, 0U);

  if (backButton)
  {
    ui_DrawButton(&uiBackButton);
  }
}

static void ui_FormatUptime(char *text, size_t size)
{
  const uint32_t seconds = HAL_GetTick() / 1000U;
  (void)snprintf(text, size, "%lu:%02lu:%02lu",
                 (unsigned long)(seconds / 3600U),
                 (unsigned long)((seconds / 60U) % 60U),
                 (unsigned long)(seconds % 60U));
}

/* ---- Home ---------------------------------------------------------------- */

static void ui_RefreshHome(void)
{
  Network_StatusTypeDef status;
  char uptime[16];
  char address[16] = "no address";
  char line[64];

  (void)Network_GetStatus(&status);
  if (ip4_addr_get_u32(&status.Address) != 0U)
  {
    (void)ip4addr_ntoa_r(&status.Address, address, sizeof(address));
  }
  ui_FormatUptime(uptime, sizeof(uptime));
  (void)snprintf(line, sizeof(line), "IP %s  link %s  up %s", address,
                 status.LinkUp ? "up" : "down", uptime);

  const uint16_t x = (uint16_t)((uiDisplay->Width -
      (UI_HOME_STATUS_COLUMNS * uiFontMedium.Width)) / 2U);
  ui_DrawText(x, UI_HOME_STATUS_Y, &uiFontMedium,
              status.LinkUp ? UI_COLOR_OK : UI_COLOR_WARNING,
              UI_COLOR_BACKGROUND, line, UI_HOME_STATUS_COLUMNS);
}

static void ui_DrawHome(void)
{
  ui_DrawFrame("STM32F769I-DISCO", false);
  for (uint32_t index = 0U; index < 2U; index++)
  {
    ui_DrawButton(&uiHomeButtons[index]);
  }
  ui_RefreshHome();
}

/* ---- Network ------------------------------------------------------------- */

static void ui_RefreshNetwork(bool force)
{
  Network_StatusTypeDef status;
  char values[UI_NET_ROWS][UI_NET_VALUE_COLUMNS + 1U];
  (void)Network_GetStatus(&status);

  const size_t size = sizeof(values[0]);
  (void)snprintf(values[0], size, "%s", !status.Initialized ? "not started" :
                 (status.LinkUp ? "up" : "down (check cable)"));
  if (status.LinkUp)
  {
    (void)snprintf(values[1], size, "%lu Mbit/s %s duplex",
                   (unsigned long)status.SpeedMbps,
                   status.FullDuplex ? "full" : "half");
  }
  else
  {
    (void)snprintf(values[1], size, "-");
  }
  (void)snprintf(values[2], size, "%s",
                 Network_GetDhcpStateName(status.DhcpState));
  (void)ip4addr_ntoa_r(&status.Address, values[3], (int)size);
  (void)ip4addr_ntoa_r(&status.Netmask, values[4], (int)size);
  (void)ip4addr_ntoa_r(&status.Gateway, values[5], (int)size);
#if LWIP_DNS
  (void)ip4addr_ntoa_r(&status.Dns, values[6], (int)size);
#else
  (void)snprintf(values[6], size, "n/a (LWIP_DNS off)");
#endif
  (void)snprintf(values[7], size, "%02X:%02X:%02X:%02X:%02X:%02X",
                 status.Mac[0], status.Mac[1], status.Mac[2], status.Mac[3],
                 status.Mac[4], status.Mac[5]);
  ui_FormatUptime(values[8], size);

  for (uint32_t row = 0U; row < UI_NET_ROWS; row++)
  {
    if (!force && (strcmp(values[row], uiNetworkShown[row]) == 0))
    {
      continue;
    }
    const bool good = (row == 0U) ? status.LinkUp :
                      ((row == 2U) ? status.DhcpBound : true);
    ui_DrawText(UI_NET_VALUE_X,
                (uint16_t)(UI_NET_FIRST_ROW_Y + (row * UI_NET_ROW_STEP)),
                &uiFontMedium, good ? UI_COLOR_TEXT : UI_COLOR_WARNING,
                UI_COLOR_BACKGROUND, values[row], UI_NET_VALUE_COLUMNS);
    memcpy(uiNetworkShown[row], values[row], sizeof(uiNetworkShown[row]));
  }
}

static void ui_DrawNetwork(void)
{
  ui_DrawFrame("Network", true);
  for (uint32_t row = 0U; row < UI_NET_ROWS; row++)
  {
    ui_DrawText(UI_NET_LABEL_X,
                (uint16_t)(UI_NET_FIRST_ROW_Y + (row * UI_NET_ROW_STEP)),
                &uiFontMedium, UI_COLOR_LABEL, UI_COLOR_BACKGROUND,
                uiNetworkLabels[row], 0U);
  }
  ui_RefreshNetwork(true);
}

/* ---- Log ----------------------------------------------------------------- */

/** Draw only the characters that differ from what is already on screen. */
static void ui_RefreshLog(void)
{
  Font_TypeDef glyphs = uiFontSmall;
  glyphs.Color = UI_COLOR_LOG_TEXT;
  glyphs.Bgcolor = UI_COLOR_LOG_BACKGROUND;

  uiLogSequence = LogBuffer_GetSequence();
  const uint32_t available = LogBuffer_GetLineCount();

  for (uint32_t row = 0U; row < UI_LOG_ROWS; row++)
  {
    /* The bottom row shows the line being written (age 0). */
    const uint32_t age = UI_LOG_ROWS - 1U - row;
    uint16_t length = 0U;
    const char *text = (age < available) ? LogBuffer_GetLine(age, &length) : NULL;

    for (uint16_t column = 0U; column < UI_LOG_COLUMNS; column++)
    {
      const char ch = ((text != NULL) && (column < length)) ? text[column] : ' ';
      if (uiLogShown[row][column] == ch)
      {
        continue;
      }
      uint16_t x = (uint16_t)(UI_LOG_ORIGIN_X + (column * UI_LOG_CELL_WIDTH));
      uint16_t y = ui_ToDisplayY((uint16_t)(UI_LOG_ORIGIN_Y +
                                            (row * UI_LOG_CELL_HEIGHT)),
                                 UI_LOG_CELL_HEIGHT);
      if (Display_DrawSymbol(uiDisplay, L1, &x, &y, &glyphs, (uint8_t)ch) ==
          HAL_OK)
      {
        uiLogShown[row][column] = ch;
      }
    }
  }
}

static void ui_DrawLog(void)
{
  ui_DrawFrame("printf() log", true);
  ui_FillBox(0U, UI_TITLE_BAR_HEIGHT, uiDisplay->Width,
             (uint16_t)(uiDisplay->Height - UI_TITLE_BAR_HEIGHT),
             UI_COLOR_LOG_BACKGROUND);
  memset(uiLogShown, ' ', sizeof(uiLogShown));
  ui_RefreshLog();
}

/* ---- Public interface ---------------------------------------------------- */

HAL_StatusTypeDef Ui_Init(Display_TypeDef *display)
{
  if ((display == NULL) || (display->Lock == ENABLE))
  {
    return HAL_ERROR;
  }
  uiDisplay = display;
  Ui_ShowScreen(UI_SCREEN_HOME);
  return HAL_OK;
}

void Ui_ShowScreen(Ui_ScreenTypeDef screen)
{
  if (uiDisplay == NULL)
  {
    return;
  }

  uiScreen = screen;
  /* Logged before drawing so the Log screen already contains this line. */
  printf("UI: %s screen\r\n", Ui_GetScreenName(screen));
  switch (screen)
  {
    case UI_SCREEN_NETWORK:
      ui_DrawNetwork();
      break;
    case UI_SCREEN_LOG:
      ui_DrawLog();
      break;
    case UI_SCREEN_HOME:
    default:
      ui_DrawHome();
      break;
  }
  uiLastRefresh = HAL_GetTick();
}

bool Ui_HandleTap(uint16_t x, uint16_t y)
{
  if (uiDisplay == NULL)
  {
    return false;
  }

  if (uiScreen == UI_SCREEN_HOME)
  {
    for (uint32_t index = 0U; index < 2U; index++)
    {
      if (ui_IsHit(&uiHomeButtons[index], x, y))
      {
        Ui_ShowScreen(uiHomeButtons[index].target);
        return true;
      }
    }
  }
  else if (ui_IsHit(&uiBackButton, x, y))
  {
    Ui_ShowScreen(uiBackButton.target);
    return true;
  }
  return false;
}

void Ui_Process(void)
{
  if (uiDisplay == NULL)
  {
    return;
  }

  const uint32_t elapsed = HAL_GetTick() - uiLastRefresh;
  switch (uiScreen)
  {
    case UI_SCREEN_HOME:
      if (elapsed >= UI_STATUS_PERIOD_MS)
      {
        uiLastRefresh = HAL_GetTick();
        ui_RefreshHome();
      }
      break;
    case UI_SCREEN_NETWORK:
      if (elapsed >= UI_NETWORK_PERIOD_MS)
      {
        uiLastRefresh = HAL_GetTick();
        ui_RefreshNetwork(false);
      }
      break;
    case UI_SCREEN_LOG:
      if ((elapsed >= UI_LOG_PERIOD_MS) &&
          (LogBuffer_GetSequence() != uiLogSequence))
      {
        uiLastRefresh = HAL_GetTick();
        ui_RefreshLog();
      }
      break;
    default:
      break;
  }
}

Ui_ScreenTypeDef Ui_GetScreen(void)
{
  return uiScreen;
}

const char *Ui_GetScreenName(Ui_ScreenTypeDef screen)
{
  switch (screen)
  {
    case UI_SCREEN_HOME:    return "home";
    case UI_SCREEN_NETWORK: return "network";
    case UI_SCREEN_LOG:     return "log";
    default:                return "unknown";
  }
}
