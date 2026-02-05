/**
  ******************************************************************************
  * File Name          : tf6206.h
  * Description        : This file provides definition for the code of 
  *                      touchscreen controller TF6202.
  ******************************************************************************
  * @attention
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FT6206_H
#define __FT6206_H


#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"


extern I2C_HandleTypeDef hi2c4;

#define FT6206_I2C_ADDR   0x2a


/* Exported functions prototypes ---------------------------------------------*/
TouchScreen_TypeDef* FT6206_Init(void);

HAL_StatusTypeDef __attribute__((weak)) TouchScreen_Process(TouchScreen_TypeDef*);


#ifdef __cplusplus
}
#endif
#endif /*__FT6206_H */
