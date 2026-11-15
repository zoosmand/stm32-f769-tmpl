/**
  ******************************************************************************
  * File Name          : ft6206.h
  * Description        : FT6206 capacitive touchscreen controller interface.
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
#include "common.h"

/** I2C4 bus configured by platform initialization for the touch controller. */
extern I2C_HandleTypeDef hi2c4;

/** TOUCH_ACTIVE after the FT6206 INT falling edge (set in the EXTI callback);
    TouchScreen_Process() resets it to TOUCH_IDLE before sampling. */
extern volatile TouchState_t touch_activated_flag;

#define FT6206_I2C_ADDR   0x2a

#define FT6x02_DEV_MODE             0x00
#define FT6x02_GEST_ID              0x01
#define FT6x02_TD_STATUS            0x02
#define FT6x02_P1_XH                0x03
#define FT6x02_P1_XL                0x04
#define FT6x02_P1_YH                0x05
#define FT6x02_P1_YL                0x06
#define FT6x02_P1_WEIGHT            0x07
#define FT6x02_P1_MISC              0x08
#define FT6x02_P2_XH                0x09
#define FT6x02_P2_XL                0x0a
#define FT6x02_P2_YH                0x0b
#define FT6x02_P2_YL                0x0c
#define FT6x02_P2_WEIGHT            0x0d
#define FT6x02_P2_MISC              0x0e
#define FT6x02_TH_GROUP             0x80
#define FT6x02_TH_DIFF              0x85
#define FT6x02_CTRL                 0x86
#define FT6x02_TIMEENTERMONITOR     0x87
#define FT6x02_PERIODACTIVE         0x88
#define FT6x02_PERIODMONITOR        0x89
#define FT6x02_RADIAN_VALUE         0x91
#define FT6x02_OFFSET_LEFT_RIGHT    0x92
#define FT6x02_OFFSET_UP_DOWN       0x93
#define FT6x02_DISTANCE_LEFT_RIJHT  0x94
#define FT6x02_DISTANCE_UP_DOWN     0x95
#define FT6x02_DISTANCE_ZOOM        0x96
#define FT6x02_LIB_VER_H            0xa1
#define FT6x02_LIB_VER_L            0xa2
#define FT6x02_CIPHER               0xa3
#define FT6x02_G_MODE               0xa4
#define FT6x02_PWR_MODE             0xa5
#define FT6x02_FIRMID               0xa6
#define FT6x02_FOCALTECH_ID         0xa8
#define FT6x02_RELEASE_CODE         0xaf
#define FT6x02_STATE                0xbc
#define FT6x02_DEV_DATA             FT6x02_LIB_VER_H
#define FT6x02_DYN_DATA             FT6x02_DEV_MODE

#define FT6x02_GEST_MOVE_UP         0x10
#define FT6x02_GEST_MOVE_RIGHT      0x14
#define FT6x02_GEST_MOVE_DOWN       0x18
#define FT6x02_GEST_MOVE_LEFT       0x1c
#define FT6x02_GEST_MOVE_ZOOM_IN    0x48
#define FT6x02_GEST_MOVE_ZOOM_OUT   0x49
#define FT6x02_GEST_MOVE_NOOP       0x00

#define FT6x02_EVENT_PRESS_DOWN     0b00
#define FT6x02_EVENT_LIFT_UP        0b01
#define FT6x02_EVENT_CONTACT        0b10
#define FT6x02_EVENT_NOOP           0b11

#define FT6x02_FOCALTECH_ID_VALUE   0x11

#define TOUCH_STABLE_COUNT          3   // consecutive reads
#define TOUCH_MOVE_THRESHOLD        3   // pixels
#define TOUCH_RELEASE_COUNT         5   // consecutive reads
#define TOUCH_DEADZONE              3   // pixels
#define TOUCH_RELEASE_THRESHOLD     500 // ms

/**
  * @brief Initialize the FT6206 controller and its falling-edge EXTI input.
  * @retval Persistent device object; inspect State for initialization result.
  * @note Must run after I2C4 and GPIO clocks are initialized. The returned
  *       object has static lifetime and remains owned by this module.
  */
TouchScreen_TypeDef* FT6206_Init(void);

/**
  * @brief Read and advance one touch-controller state-machine sample.
  * @param touch Non-null device returned by FT6206_Init().
  * @retval HAL_OK on a processed sample; HAL_ERROR on invalid/device state or
  *         an I2C transfer failure.
  * @note Call from thread/main-loop context after the EXTI callback signals a
  *       pending sample. The function performs a blocking I2C transaction.
  */
HAL_StatusTypeDef __attribute__((weak)) TouchScreen_Process(
    TouchScreen_TypeDef *touch);

#ifdef __cplusplus
}
#endif
#endif /*__FT6206_H */
