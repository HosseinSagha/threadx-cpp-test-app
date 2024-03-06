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

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);
void MX_GPIO_Init(void);
void MX_DMA_Init(void);
void MX_ADC1_Init(void);
void MX_CAN1_Init(void);
void MX_CAN2_Init(void);
void MX_I2C2_Init(void);
void MX_I2C3_Init(void);
void MX_I2C4_Init(void);
void MX_SPI1_Init(void);
void MX_TIM1_Init(void);
void MX_TIM2_Init(void);
void MX_TIM3_Init(void);
void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
void MX_USART3_UART_Init(void);
void MX_RTC_Init(void);
void MX_I2C1_Init(void);
void MX_TIM7_Init(void);

/* USER CODE BEGIN EFP */
void SystemClock_Config(void);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MOD1_I2C_EN_Pin GPIO_PIN_4
#define MOD1_I2C_EN_GPIO_Port GPIOE
#define MOD2_I2C_EN_Pin GPIO_PIN_5
#define MOD2_I2C_EN_GPIO_Port GPIOE
#define BUS_EN1_Pin GPIO_PIN_0
#define BUS_EN1_GPIO_Port GPIOF
#define CHG_EN1_Pin GPIO_PIN_1
#define CHG_EN1_GPIO_Port GPIOF
#define BAT_I2C_SCL_Pin GPIO_PIN_0
#define BAT_I2C_SCL_GPIO_Port GPIOC
#define BAT_I2C_SDA_Pin GPIO_PIN_1
#define BAT_I2C_SDA_GPIO_Port GPIOC
#define ADC3_ID_VAR_Pin GPIO_PIN_2
#define ADC3_ID_VAR_GPIO_Port GPIOC
#define ADC4_ID_CAT_Pin GPIO_PIN_3
#define ADC4_ID_CAT_GPIO_Port GPIOC
#define RS485_TX_Pin GPIO_PIN_0
#define RS485_TX_GPIO_Port GPIOA
#define CLK32K_Pin GPIO_PIN_2
#define CLK32K_GPIO_Port GPIOA
#define ADC8_VIN_MON_Pin GPIO_PIN_3
#define ADC8_VIN_MON_GPIO_Port GPIOA
#define SAL_CAR_Pin GPIO_PIN_5
#define SAL_CAR_GPIO_Port GPIOA
#define ADC11_SAL_FB_Pin GPIO_PIN_6
#define ADC11_SAL_FB_GPIO_Port GPIOA
#define SAL_EN_Pin GPIO_PIN_7
#define SAL_EN_GPIO_Port GPIOA
#define ADC13_DIG_IN3_Pin GPIO_PIN_4
#define ADC13_DIG_IN3_GPIO_Port GPIOC
#define ADC14_DIG_IN4_Pin GPIO_PIN_5
#define ADC14_DIG_IN4_GPIO_Port GPIOC
#define ADC15_DIG_IN1_Pin GPIO_PIN_0
#define ADC15_DIG_IN1_GPIO_Port GPIOB
#define ADC16_DIG_IN2_Pin GPIO_PIN_1
#define ADC16_DIG_IN2_GPIO_Port GPIOB
#define IMU_INT_Pin GPIO_PIN_2
#define IMU_INT_GPIO_Port GPIOB
#define IMU_INT_EXTI_IRQn EXTI2_IRQn
#define LED1_Pin GPIO_PIN_12
#define LED1_GPIO_Port GPIOF
#define LED2_Pin GPIO_PIN_13
#define LED2_GPIO_Port GPIOF
#define LED3_Pin GPIO_PIN_14
#define LED3_GPIO_Port GPIOF
#define LED4_Pin GPIO_PIN_15
#define LED4_GPIO_Port GPIOF
#define WAKE_Pin GPIO_PIN_0
#define WAKE_GPIO_Port GPIOG
#define BT_RST_Pin GPIO_PIN_1
#define BT_RST_GPIO_Port GPIOG
#define M2_IRQ_Pin GPIO_PIN_8
#define M2_IRQ_GPIO_Port GPIOE
#define M2_IRQ_EXTI_IRQn EXTI9_5_IRQn
#define SYNCH_IN_Pin GPIO_PIN_9
#define SYNCH_IN_GPIO_Port GPIOE
#define RF_COEX_Pin GPIO_PIN_10
#define RF_COEX_GPIO_Port GPIOE
#define RF_COEX_EXTI_IRQn EXTI15_10_IRQn
#define SYNCH_OUT_Pin GPIO_PIN_11
#define SYNCH_OUT_GPIO_Port GPIOE
#define FLASH_SPI_nCS_Pin GPIO_PIN_12
#define FLASH_SPI_nCS_GPIO_Port GPIOE
#define M2_SPI_SCK_Pin GPIO_PIN_13
#define M2_SPI_SCK_GPIO_Port GPIOE
#define M2_SPI_MISO_Pin GPIO_PIN_14
#define M2_SPI_MISO_GPIO_Port GPIOE
#define M2_SPI_MOSI_Pin GPIO_PIN_15
#define M2_SPI_MOSI_GPIO_Port GPIOE
#define MOD_I2C_SCL_Pin GPIO_PIN_10
#define MOD_I2C_SCL_GPIO_Port GPIOB
#define MOD_I2C_SDA_Pin GPIO_PIN_11
#define MOD_I2C_SDA_GPIO_Port GPIOB
#define WATCHDOG_Pin GPIO_PIN_12
#define WATCHDOG_GPIO_Port GPIOB
#define MAIN_I2C_SCL_Pin GPIO_PIN_13
#define MAIN_I2C_SCL_GPIO_Port GPIOB
#define MAIN_I2C_SDA_Pin GPIO_PIN_14
#define MAIN_I2C_SDA_GPIO_Port GPIOB
#define BAT_EN_Pin GPIO_PIN_15
#define BAT_EN_GPIO_Port GPIOB
#define BT_TX_Pin GPIO_PIN_8
#define BT_TX_GPIO_Port GPIOD
#define BT_RX_Pin GPIO_PIN_9
#define BT_RX_GPIO_Port GPIOD
#define VREF_EN_Pin GPIO_PIN_10
#define VREF_EN_GPIO_Port GPIOD
#define BT_CTS_Pin GPIO_PIN_11
#define BT_CTS_GPIO_Port GPIOD
#define BT_RTS_Pin GPIO_PIN_12
#define BT_RTS_GPIO_Port GPIOD
#define DIG_OUT1_Pin GPIO_PIN_13
#define DIG_OUT1_GPIO_Port GPIOD
#define DIG_OUT2_Pin GPIO_PIN_14
#define DIG_OUT2_GPIO_Port GPIOD
#define BT_SPI_nCS_Pin GPIO_PIN_15
#define BT_SPI_nCS_GPIO_Port GPIOD
#define BT_EN_Pin GPIO_PIN_4
#define BT_EN_GPIO_Port GPIOG
#define BUS_EN2_Pin GPIO_PIN_6
#define BUS_EN2_GPIO_Port GPIOG
#define PWR_STOP_Pin GPIO_PIN_7
#define PWR_STOP_GPIO_Port GPIOG
#define ID_READ_Pin GPIO_PIN_8
#define ID_READ_GPIO_Port GPIOG
#define SAL_MOD_Pin GPIO_PIN_6
#define SAL_MOD_GPIO_Port GPIOC
#define PDA_TX_Pin GPIO_PIN_9
#define PDA_TX_GPIO_Port GPIOA
#define PDA_RX_Pin GPIO_PIN_10
#define PDA_RX_GPIO_Port GPIOA
#define PDA_TXE_Pin GPIO_PIN_12
#define PDA_TXE_GPIO_Port GPIOA
#define RS485_TXE_Pin GPIO_PIN_15
#define RS485_TXE_GPIO_Port GPIOA
#define NODE_CAN_RX_Pin GPIO_PIN_0
#define NODE_CAN_RX_GPIO_Port GPIOD
#define NODE_CAN_TX_Pin GPIO_PIN_1
#define NODE_CAN_TX_GPIO_Port GPIOD
#define SAL_CAR_FB_Pin GPIO_PIN_2
#define SAL_CAR_FB_GPIO_Port GPIOD
#define BT_I2C_EN_Pin GPIO_PIN_3
#define BT_I2C_EN_GPIO_Port GPIOD
#define NODE_DE_Pin GPIO_PIN_4
#define NODE_DE_GPIO_Port GPIOD
#define NODE_TX_Pin GPIO_PIN_5
#define NODE_TX_GPIO_Port GPIOD
#define NODE_RX_Pin GPIO_PIN_6
#define NODE_RX_GPIO_Port GPIOD
#define RS485_TERM_Pin GPIO_PIN_9
#define RS485_TERM_GPIO_Port GPIOG
#define RS485_nEN_Pin GPIO_PIN_10
#define RS485_nEN_GPIO_Port GPIOG
#define CHG_EN2_Pin GPIO_PIN_11
#define CHG_EN2_GPIO_Port GPIOG
#define VEHICLE_CAN_TERM_Pin GPIO_PIN_13
#define VEHICLE_CAN_TERM_GPIO_Port GPIOG
#define VEHICLE_CAN_EN_Pin GPIO_PIN_14
#define VEHICLE_CAN_EN_GPIO_Port GPIOG
#define SAL_FB_SEL_Pin GPIO_PIN_15
#define SAL_FB_SEL_GPIO_Port GPIOG
#define VEHICLE_CAN_RX_Pin GPIO_PIN_5
#define VEHICLE_CAN_RX_GPIO_Port GPIOB
#define VEHICLE_CAN_TX_Pin GPIO_PIN_6
#define VEHICLE_CAN_TX_GPIO_Port GPIOB
#define M2_I2C_SCL_Pin GPIO_PIN_8
#define M2_I2C_SCL_GPIO_Port GPIOB
#define M2_I2C_SDA_Pin GPIO_PIN_9
#define M2_I2C_SDA_GPIO_Port GPIOB
#define LOWPWR_Pin GPIO_PIN_1
#define LOWPWR_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
