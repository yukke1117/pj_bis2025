/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32u3xx_hal.h"

#include "stm32u3xx_nucleo.h"
#include <stdio.h>

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RCC_OSC32_IN_Pin GPIO_PIN_14
#define RCC_OSC32_IN_GPIO_Port GPIOC
#define RCC_OSC32_OUT_Pin GPIO_PIN_15
#define RCC_OSC32_OUT_GPIO_Port GPIOC
#define ADC_DRDY_Pin GPIO_PIN_15
#define ADC_DRDY_GPIO_Port GPIOB
#define ADC_RESET_Pin GPIO_PIN_6
#define ADC_RESET_GPIO_Port GPIOC
#define ADC_CS_Pin GPIO_PIN_7
#define ADC_CS_GPIO_Port GPIOC
#define LCD_EXTCOMIN_Pin GPIO_PIN_8
#define LCD_EXTCOMIN_GPIO_Port GPIOC
#define LCD_CS_Pin GPIO_PIN_9
#define LCD_CS_GPIO_Port GPIOC
#define DEBUG_JTMS_SWDIO_Pin GPIO_PIN_13
#define DEBUG_JTMS_SWDIO_GPIO_Port GPIOA
#define DEBUG_JTCK_SWCLK_Pin GPIO_PIN_14
#define DEBUG_JTCK_SWCLK_GPIO_Port GPIOA
#define DEBUG_JTDI_Pin GPIO_PIN_15
#define DEBUG_JTDI_GPIO_Port GPIOA
#define LCD_DISP_Pin GPIO_PIN_10
#define LCD_DISP_GPIO_Port GPIOC
#define DEBUG_JTDO_SWO_Pin GPIO_PIN_3
#define DEBUG_JTDO_SWO_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
