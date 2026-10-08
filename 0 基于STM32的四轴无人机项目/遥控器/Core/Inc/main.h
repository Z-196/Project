/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#define LED1_Pin GPIO_PIN_13
#define LED1_GPIO_Port GPIOC
#define LED2_Pin GPIO_PIN_14
#define LED2_GPIO_Port GPIOC
#define LED3_Pin GPIO_PIN_15
#define LED3_GPIO_Port GPIOC
#define BAT_EN_Pin GPIO_PIN_2
#define BAT_EN_GPIO_Port GPIOA
#define RIGHT_Y_Pin GPIO_PIN_4
#define RIGHT_Y_GPIO_Port GPIOA
#define RIGHT_X_Pin GPIO_PIN_5
#define RIGHT_X_GPIO_Port GPIOA
#define LEFT_Y_Pin GPIO_PIN_6
#define LEFT_Y_GPIO_Port GPIOA
#define LEFT_X_Pin GPIO_PIN_7
#define LEFT_X_GPIO_Port GPIOA
#define LEFT_IN_Pin GPIO_PIN_0
#define LEFT_IN_GPIO_Port GPIOB
#define RIGHT_IN_Pin GPIO_PIN_1
#define RIGHT_IN_GPIO_Port GPIOB
#define BUZZER_Pin GPIO_PIN_2
#define BUZZER_GPIO_Port GPIOB
#define KEY_POWER_Pin GPIO_PIN_10
#define KEY_POWER_GPIO_Port GPIOB
#define SPI2_EN_Pin GPIO_PIN_11
#define SPI2_EN_GPIO_Port GPIOB
#define SPI2_NSS_Pin GPIO_PIN_12
#define SPI2_NSS_GPIO_Port GPIOB
#define SPI2_SCK_Pin GPIO_PIN_13
#define SPI2_SCK_GPIO_Port GPIOB
#define SPI2_MISO_Pin GPIO_PIN_14
#define SPI2_MISO_GPIO_Port GPIOB
#define SPI2_MOSI_Pin GPIO_PIN_15
#define SPI2_MOSI_GPIO_Port GPIOB
#define SPI2_IRQ_Pin GPIO_PIN_8
#define SPI2_IRQ_GPIO_Port GPIOA
#define KEY8_Pin GPIO_PIN_11
#define KEY8_GPIO_Port GPIOA
#define KEY7_Pin GPIO_PIN_12
#define KEY7_GPIO_Port GPIOA
#define KEY6_Pin GPIO_PIN_15
#define KEY6_GPIO_Port GPIOA
#define KEY5_Pin GPIO_PIN_3
#define KEY5_GPIO_Port GPIOB
#define KEY4_Pin GPIO_PIN_4
#define KEY4_GPIO_Port GPIOB
#define KEY3_Pin GPIO_PIN_5
#define KEY3_GPIO_Port GPIOB
#define KEY2_Pin GPIO_PIN_6
#define KEY2_GPIO_Port GPIOB
#define KEY1_Pin GPIO_PIN_7
#define KEY1_GPIO_Port GPIOB

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
