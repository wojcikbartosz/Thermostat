/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "stm32l4xx_hal.h"

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
#define USART_TX_Pin GPIO_PIN_2
#define USART_TX_GPIO_Port GPIOA
#define USART_RX_Pin GPIO_PIN_3
#define USART_RX_GPIO_Port GPIOA
#define LD2_Pin GPIO_PIN_5
#define LD2_GPIO_Port GPIOA
#define MENU_BUTTON_Pin GPIO_PIN_1
#define MENU_BUTTON_GPIO_Port GPIOB
#define MENU_BUTTON_EXTI_IRQn EXTI1_IRQn
#define DEST_TEMP_CHANGE_Pin GPIO_PIN_13
#define DEST_TEMP_CHANGE_GPIO_Port GPIOB
#define DEST_TEMP_CHANGE_EXTI_IRQn EXTI15_10_IRQn
#define TEMP_MINUS_Pin GPIO_PIN_14
#define TEMP_MINUS_GPIO_Port GPIOB
#define TEMP_MINUS_EXTI_IRQn EXTI15_10_IRQn
#define TEMP_PLUS_Pin GPIO_PIN_15
#define TEMP_PLUS_GPIO_Port GPIOB
#define TEMP_PLUS_EXTI_IRQn EXTI15_10_IRQn
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define DAY_INPUT_Pin GPIO_PIN_10
#define DAY_INPUT_GPIO_Port GPIOC
#define DAY_INPUT_EXTI_IRQn EXTI15_10_IRQn
#define MINUTE_INPUT_Pin GPIO_PIN_11
#define MINUTE_INPUT_GPIO_Port GPIOC
#define MINUTE_INPUT_EXTI_IRQn EXTI15_10_IRQn
#define HOUR_INPUT_Pin GPIO_PIN_12
#define HOUR_INPUT_GPIO_Port GPIOC
#define HOUR_INPUT_EXTI_IRQn EXTI15_10_IRQn
#define SWO_Pin GPIO_PIN_3
#define SWO_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
