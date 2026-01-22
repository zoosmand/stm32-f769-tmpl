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




/**
 * @brief   Display device type definition struct.
 */
typedef struct {
  FunctionalState       Lock;
  uint16_t              Model;
  LTDC_LayerCfgTypeDef* Layer1;  
  LTDC_LayerCfgTypeDef* Layer2;  
  HAL_StatusTypeDef     (*Callback)(uint32_t*);
} Display_TypeDef;


typedef struct {
  uint8_t               Width;
  uint8_t               Height;
  uint32_t              Color;
  uint32_t              Bgcolor;
  uint32_t              BytesPerGlif;
  uint8_t*              Font;
} Font_TypeDef;




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