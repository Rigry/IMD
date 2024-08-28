/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "stm32f1xx_hal.h"

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

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_CAN_Pin GPIO_PIN_0
#define LED_CAN_GPIO_Port GPIOC
#define ON_Pin GPIO_PIN_1
#define ON_GPIO_Port GPIOC
#define RELAY_Pin GPIO_PIN_2
#define RELAY_GPIO_Port GPIOC
#define Shunt__Pin GPIO_PIN_4
#define Shunt__GPIO_Port GPIOC
#define Shunt_C5_Pin GPIO_PIN_5
#define Shunt_C5_GPIO_Port GPIOC
#define LED_GREEN_Pin GPIO_PIN_0
#define LED_GREEN_GPIO_Port GPIOB
#define LED_RED_Pin GPIO_PIN_1
#define LED_RED_GPIO_Port GPIOB
#define TEST_Pin GPIO_PIN_10
#define TEST_GPIO_Port GPIOA
#define res_Pin GPIO_PIN_15
#define res_GPIO_Port GPIOA
#define fb_res_Pin GPIO_PIN_10
#define fb_res_GPIO_Port GPIOC
#define fb_norma_Pin GPIO_PIN_11
#define fb_norma_GPIO_Port GPIOC
#define norma_Pin GPIO_PIN_12
#define norma_GPIO_Port GPIOC
#define first_level_Pin GPIO_PIN_2
#define first_level_GPIO_Port GPIOD
#define fb_first_level_Pin GPIO_PIN_3
#define fb_first_level_GPIO_Port GPIOB
#define fb_second_level_Pin GPIO_PIN_4
#define fb_second_level_GPIO_Port GPIOB
#define second_level_Pin GPIO_PIN_5
#define second_level_GPIO_Port GPIOB
#define KZ_plus_Pin GPIO_PIN_6
#define KZ_plus_GPIO_Port GPIOB
#define fb_KZ_plus_Pin GPIO_PIN_7
#define fb_KZ_plus_GPIO_Port GPIOB
#define fb_KZ_minus_Pin GPIO_PIN_8
#define fb_KZ_minus_GPIO_Port GPIOB
#define KZ_minus_Pin GPIO_PIN_9
#define KZ_minus_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
