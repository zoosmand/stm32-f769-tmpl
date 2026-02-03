/**
  ******************************************************************************
  * @file           : common.h
  * @brief          : Header for common.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */
 
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __COMMON_H
#define __COMMON_H

#ifdef __cplusplus
  extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "main.h"


/* Exported types ------------------------------------------------------------*/
    

/* Private typedef -----------------------------------------------------------*/



/* Private defines -----------------------------------------------------------*/

/* Independent Watchdog */
#define IWDG_KEY_RELOAD             0x0000aaaa               /*!< IWDG Reload Counter Enable   */
#define IWDG_KEY_ENABLE             0x0000cccc               /*!< IWDG Peripheral Enable       */
#define IWDG_KEY_WR_ACCESS_ENABLE   0x00005555               /*!< IWDG KR Write Access Enable  */


/* Private macro -------------------------------------------------------------*/

#define SWO_PrintChar(c) ITM_SendChar(c)

#define SWO_PrintString(s) \
  do { const char *p = s; while (*p) ITM_SendChar(*p++); } while (0)


#define FT6206_I2C_ADDR   0x38

typedef struct {
  uint8_t               Width;
  uint8_t               Height;
  uint32_t              Color;
  uint32_t              Bgcolor;
  uint32_t              BytesPerGlif;
  uint8_t*              Font;
} Font_TypeDef;


typedef struct {
  uint8_t               Event;   // 0=down, 1=up, 2=contact
  uint16_t              RawX;
  uint16_t              RawY;
  uint16_t              X;
  uint16_t              Y;
  uint16_t              LastX;
  uint16_t              LastY;
  uint16_t              BounceX;
  uint16_t              BounceY;
  uint8_t               StableCount;
  uint8_t               ReleaseCount;
  uint8_t               Touches;
  uint32_t              Threshold;
  uint32_t              TouchCount;
} TouchContext_TypeDef;

typedef enum {
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

typedef enum {
  TOUCH_ON_DOWN,
  TOUCH_ON_UP,
  TOUCH_ON_MOVE,
  TOUCH_ON_HOLD,
  TOUCH_ON_IDLE,
} TouchEvent_t;

typedef struct {
  uint16_t              Model;
  uint8_t               Orientation;
  TouchContext_TypeDef* Context;
  TouchState_t          State;
  TouchEvent_t          Event;
  uint32_t*             BusHandler;
  uint8_t               BusAddr;
  HAL_StatusTypeDef     (*Callback)(uint32_t*);
} TouchScreen_TypeDef;


/**
 * @brief   Display device type definition struct.
 */
typedef struct {
  FunctionalState       Lock;
  uint16_t              Model;
  LTDC_LayerCfgTypeDef* Layer1;
  LTDC_LayerCfgTypeDef* Layer2;
  TouchScreen_TypeDef*  TouchDev;
  HAL_StatusTypeDef     (*Callback)(uint32_t*);
} Display_TypeDef;




/* Exported macro ------------------------------------------------------------*/
#define FLAG_SET(registry, flag)        SET_BIT(registry, (1 << flag))
#define FLAG_CLR(registry, flag)        CLEAR_BIT(registry, (1 << flag))
#define FLAG_CHECK(registry, flag)      (READ_BIT(registry, (1 << flag)))

#define PIN_H(port, pin)                SET_BIT(port->BSRR, pin)
#define PIN_L(port, pin)                SET_BIT(port->BSRR, (pin << 16))
#define PIN_LEVEL(port, pin)            (READ_BIT(port->IDR, pin))

#define PREG_SET(registry, key)         SET_BIT(registry, (1 << key))
#define PREG_CLR(registry, key)         CLEAR_BIT(registry, (1 << key))
#define PREG_CHECK(registry, key)       (READ_BIT(registry, (1 << key)))


/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);



#ifdef __cplusplus
}
#endif

#endif /* __COMMON_H */