#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>

#include "stm32f7xx_hal.h"

/** Bitmap-font metadata and pixel colors used by display drawing functions. */
typedef struct
{
  uint8_t Width;          /**< Glyph width in pixels. */
  uint8_t Height;         /**< Glyph height in pixels. */
  uint32_t Color;         /**< Foreground color in the active layer format. */
  uint32_t Bgcolor;       /**< Background color in the active layer format. */
  uint32_t BytesPerGlif;  /**< Bytes occupied by one glyph in Font. */
  const uint8_t *Font;    /**< Non-null first glyph bitmap (ASCII space). */
} Font_TypeDef;

/** Mutable coordinates and debounce counters for one touchscreen device. */
typedef struct
{
  uint8_t Event;          /**< Raw FT6206 event bits from the last sample. */
  uint16_t RawX;          /**< Untransformed controller X coordinate. */
  uint16_t RawY;          /**< Untransformed controller Y coordinate. */
  uint16_t X;             /**< Display-oriented X coordinate in pixels. */
  uint16_t Y;             /**< Display-oriented Y coordinate in pixels. */
  uint16_t LastX;         /**< X coordinate handled by the application. */
  uint16_t LastY;         /**< Y coordinate handled by the application. */
  uint16_t BounceX;       /**< Candidate X coordinate during debounce. */
  uint16_t BounceY;       /**< Candidate Y coordinate during debounce. */
  uint8_t StableCount;    /**< Consecutive stable samples. */
  uint8_t ReleaseCount;   /**< Consecutive release-state samples. */
  uint8_t Touches;        /**< Active contacts reported by the controller. */
  uint32_t Threshold;     /**< HAL tick deadline used by debounce logic. */
  uint32_t TouchCount;    /**< Samples accumulated for hold detection. */
} TouchContext_TypeDef;

/** Internal touchscreen state-machine states. */
typedef enum
{
  TOUCH_IDLE,
  TOUCH_DOWN,
  TOUCH_HOLD,
  TOUCH_RELEASE,
  TOUCH_UP,
  TOUCH_DEBOUNCE,
  TOUCH_ACTIVE,
  TOUCH_LOCKED,
  TOUCH_DISABLED,
} TouchState_t;

/** Touch events exposed to the application for one processed sample. */
typedef enum
{
  TOUCH_ON_DOWN,
  TOUCH_ON_UP,
  TOUCH_ON_MOVE,
  TOUCH_ON_HOLD,
  TOUCH_ON_IDLE,
} TouchEvent_t;

/** FT6206 device state and its owning I2C bus. */
typedef struct
{
  uint16_t Model;                 /**< Controller model number. */
  uint8_t Orientation;            /**< Display coordinate transform selector. */
  TouchContext_TypeDef *Context;  /**< Non-null persistent sampling context. */
  TouchState_t State;             /**< Current debounce/gesture state. */
  TouchEvent_t Event;             /**< Event produced by the last sample. */
  I2C_HandleTypeDef *BusHandler;  /**< Non-null initialized HAL I2C handle. */
  uint8_t BusAddr;                /**< HAL-format 8-bit I2C address. */
} TouchScreen_TypeDef;

/** LTDC hardware layer index. */
typedef enum
{
  L1 = 0,
  L2 = 1,
} LTCDLayer_t;

/** Display geometry, framebuffer layers, and peripheral ownership. */
typedef struct
{
  FunctionalState Lock;           /**< ENABLE while initialization has failed or is in progress. */
  uint16_t Model;                  /**< Display-controller model number. */
  LTDC_LayerCfgTypeDef *Layer1;    /**< Primary LTDC layer configuration. */
  LTDC_LayerCfgTypeDef *Layer2;    /**< Secondary LTDC layer configuration. */
  uint32_t BgLayer1;               /**< Primary-layer ARGB background color. */
  uint32_t BgLayer2;               /**< Secondary-layer ARGB background color. */
  DSI_HandleTypeDef *DSIDevHandler;       /**< DSI host owned by this display. */
  DMA2D_HandleTypeDef *DMADevHandler;     /**< DMA2D engine used for drawing. */
  SDRAM_HandleTypeDef *SDRAMDevHandler;   /**< SDRAM containing framebuffers. */
  LTDC_HandleTypeDef *LTDCDevHandler;     /**< LTDC driving the DSI stream. */
  uint16_t Width;                  /**< Logical display width in pixels. */
  uint16_t Height;                 /**< Logical display height in pixels. */
  uint32_t ColorSchema;            /**< OTM8009A DSI pixel-format value. */
} Display_TypeDef;

#endif /* COMMON_H */
