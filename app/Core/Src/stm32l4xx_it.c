/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32l4xx_it.c
  * @brief   Interrupt Service Routines.
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

/* Includes ------------------------------------------------------------------*/
#include "stm32l4xx_it.h"
#include "main.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#ifndef NDEBUG
#define MAX_FAULT_NUM 5 // store up to this number of simultaneous faults in faultStatus[]

typedef enum
{
    FAULT_STATUS_NONE = 0,
    // MemManage
    FAULT_STATUS_IACCVIOL = SCB_CFSR_IACCVIOL_Msk,
    FAULT_STATUS_DACCVIOL = SCB_CFSR_DACCVIOL_Msk,
    FAULT_STATUS_MUNSTKERR = SCB_CFSR_MUNSTKERR_Msk,
    FAULT_STATUS_MSTKERR = SCB_CFSR_MSTKERR_Msk,
#if (__CORTEX_M > 3U) // not supported on cortex-m3
    FAULT_STATUS_MLSPERR = SCB_CFSR_MLSPERR_Msk,
#endif
    FAULT_STATUS_MMARVALID = SCB_CFSR_MMARVALID_Msk,
    // BusFault
    FAULT_STATUS_IBUSERR = SCB_CFSR_IBUSERR_Msk,
    FAULT_STATUS_PRECISERR = SCB_CFSR_PRECISERR_Msk,
    FAULT_STATUS_IMPRECISERR = SCB_CFSR_IMPRECISERR_Msk,
    FAULT_STATUS_UNSTKERR = SCB_CFSR_UNSTKERR_Msk,
    FAULT_STATUS_STKERR = SCB_CFSR_STKERR_Msk,
#if (__CORTEX_M > 3U) // not supported on cortex-m3
    FAULT_STATUS_LSPERR = SCB_CFSR_LSPERR_Msk,
#endif
    FAULT_STATUS_BFARVALID = SCB_CFSR_BFARVALID_Msk,
    // UsageFault
    FAULT_STATUS_UNDEFINSTR = SCB_CFSR_UNDEFINSTR_Msk,
    FAULT_STATUS_INVSTATE = SCB_CFSR_INVSTATE_Msk,
    FAULT_STATUS_INVPC = SCB_CFSR_INVPC_Msk,
    FAULT_STATUS_NOCP = SCB_CFSR_NOCP_Msk,
    FAULT_STATUS_UNALIGNED = SCB_CFSR_UNALIGNED_Msk,
    FAULT_STATUS_DIVBYZERO = SCB_CFSR_DIVBYZERO_Msk,

    FAULT_STATUS_VECTTBL,
    FAULT_STATUS_DEBUG, // to generate an offset for the following enums
    // DebugFault
    FAULT_STATUS_HALTED = SCB_DFSR_HALTED_Msk + FAULT_STATUS_DEBUG,
    FAULT_STATUS_BKPT = SCB_DFSR_BKPT_Msk + FAULT_STATUS_DEBUG,
    FAULT_STATUS_DWTTRAP = SCB_DFSR_DWTTRAP_Msk + FAULT_STATUS_DEBUG,
    FAULT_STATUS_VCATCH = SCB_DFSR_VCATCH_Msk + FAULT_STATUS_DEBUG,
    FAULT_STATUS_EXTERNAL = SCB_DFSR_EXTERNAL_Msk + FAULT_STATUS_DEBUG
} FaultStatus_t;

typedef struct
{
    uint32_t r0;
    uint32_t r1;
    uint32_t r2;
    uint32_t r3;
    uint32_t r12;
    uint32_t lr;
    uint32_t pc;
    uint32_t psr;
    uint32_t shcsr;
    uint32_t cfsr;
    uint32_t hfsr;
    uint32_t dfsr;
    uint32_t mmfar;
    uint32_t bfar;
    uint32_t afsr;
} StackedRegs_t;

// usage, memory and bus faults will escalate to hardfaults, if they are not enabled in SHCSR register
static void HardFaultDebug(uint32_t *hardFaultArgs)
{
    volatile StackedRegs_t stackedRegs;
    // The stack frame of the fault handler contains the state of the ARM Cortex-M registers at the time that the fault
    // occurred.
    stackedRegs.r0 = hardFaultArgs[0];
    stackedRegs.r1 = hardFaultArgs[1];
    stackedRegs.r2 = hardFaultArgs[2];
    stackedRegs.r3 = hardFaultArgs[3];
    stackedRegs.r12 = hardFaultArgs[4];
    stackedRegs.lr = hardFaultArgs[5];
    stackedRegs.pc = hardFaultArgs[6];
    stackedRegs.psr = hardFaultArgs[7];
    stackedRegs.shcsr = SCB->SHCSR;
    stackedRegs.cfsr = SCB->CFSR;
    stackedRegs.hfsr = SCB->HFSR;
    stackedRegs.dfsr = SCB->DFSR;
    stackedRegs.afsr = SCB->AFSR;

    if (stackedRegs.cfsr & SCB_CFSR_MMARVALID_Msk)
    {
        stackedRegs.mmfar = SCB->MMFAR;
    }
    else
    {
        stackedRegs.mmfar = 0xffffffff; // not valid
    }

    if (stackedRegs.cfsr & SCB_CFSR_BFARVALID_Msk)
    {
        stackedRegs.bfar = SCB->BFAR;
    }
    else
    {
        stackedRegs.bfar = 0xffffffff; // not valid
    }

    [[maybe_unused]] volatile FaultStatus_t faultStatus[MAX_FAULT_NUM];
    for (size_t count = 0; count < MAX_FAULT_NUM; ++count)
    {
        faultStatus[count] = FAULT_STATUS_NONE;
    }

    size_t faultCount = 0;
    // If FORCED bit is set (bit 30), it may be caused by a bus fault, a memory fault or a usage fault.
    // CFSR must be read to find the cause of the fault.
    if (stackedRegs.hfsr & SCB_HFSR_FORCED_Msk)
    {
        // The CFSR indicates the cause of a MemManage fault, BusFault, or UsageFault.
        // MemoryMagmtFault: bit 0-7,
        // BusFault: bit 8-15,
        // UsageFault: bit 16-31
        for (size_t bitPos = 0; bitPos < (sizeof(stackedRegs.cfsr) * 8); ++bitPos) // if multiple fault bits are set
        {
            if (stackedRegs.cfsr & (1U << bitPos))
            {
                faultStatus[faultCount] = (FaultStatus_t)(stackedRegs.cfsr & (1u << bitPos));

                ++faultCount;
                if (faultCount == MAX_FAULT_NUM)
                {
                    break; // fault array is full. In reality should never get here.
                }
            }
        }
    }
    else if (stackedRegs.hfsr & SCB_HFSR_VECTTBL_Msk)
    {
        // Indicates a BusFault on a vector table read during exception processing
        faultStatus[faultCount] = FAULT_STATUS_VECTTBL;
        ++faultCount;
    }
    else if (stackedRegs.hfsr & SCB_HFSR_DEBUGEVT_Msk)
    {
        for (size_t bitPos = 0; bitPos < 5; ++bitPos) // if multiple fault bits are set
        {
            if (stackedRegs.dfsr & (1U << bitPos))
            {
                faultStatus[faultCount] = (FaultStatus_t)((stackedRegs.dfsr & (1U << bitPos)) + FAULT_STATUS_DEBUG);
                ++faultCount;
            }
        }
    }

    __asm("bkpt 1");
}
#endif
/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern DMA_HandleTypeDef hdma_adc1;
extern CAN_HandleTypeDef hcan1;
extern CAN_HandleTypeDef hcan2;
extern DMA_HandleTypeDef hdma_i2c1_rx;
extern DMA_HandleTypeDef hdma_i2c1_tx;
extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;
extern I2C_HandleTypeDef hi2c3;
extern I2C_HandleTypeDef hi2c4;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim7;
extern DMA_HandleTypeDef hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart1_tx;
extern DMA_HandleTypeDef hdma_usart2_rx;
extern DMA_HandleTypeDef hdma_usart2_tx;
extern DMA_HandleTypeDef hdma_usart3_rx;
extern DMA_HandleTypeDef hdma_usart3_tx;
extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern TIM_HandleTypeDef htim6;

/* USER CODE BEGIN EV */

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M4 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
    /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

    /* USER CODE END NonMaskableInt_IRQn 0 */
    /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
    while (1)
    {
    }
    /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
    /* USER CODE BEGIN HardFault_IRQn 0 */
#ifndef NDEBUG
    asm volatile("TST lr, #4    \n" // check if it's from a thread or the main process.
                 "ITE eq        \n"
                 "MRSEQ r0, msp \n" // set first passed parameter as the main stack pointer
                 "MRSNE r0, psp \n" // set first passed parameter as the process (thread) stack pointer
                 "B HardFaultDebug" // Call HardFaultDebug(r0)
    );
#endif
    /* USER CODE END HardFault_IRQn 0 */
    while (1)
    {
        /* USER CODE BEGIN W1_HardFault_IRQn 0 */
        /* USER CODE END W1_HardFault_IRQn 0 */
    }
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
    /* USER CODE BEGIN DebugMonitor_IRQn 0 */

    /* USER CODE END DebugMonitor_IRQn 0 */
    /* USER CODE BEGIN DebugMonitor_IRQn 1 */

    /* USER CODE END DebugMonitor_IRQn 1 */
}

/******************************************************************************/
/* STM32L4xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32l4xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles RCC global interrupt.
  */
void RCC_IRQHandler(void)
{
    /* USER CODE BEGIN RCC_IRQn 0 */

    /* USER CODE END RCC_IRQn 0 */
    /* USER CODE BEGIN RCC_IRQn 1 */

    /* USER CODE END RCC_IRQn 1 */
}

/**
  * @brief This function handles EXTI line2 interrupt.
  */
void EXTI2_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI2_IRQn 0 */

    /* USER CODE END EXTI2_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(IMU_INT_Pin);
    /* USER CODE BEGIN EXTI2_IRQn 1 */

    /* USER CODE END EXTI2_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel1 global interrupt.
  */
void DMA1_Channel1_IRQHandler(void)
{
    /* USER CODE BEGIN DMA1_Channel1_IRQn 0 */

    /* USER CODE END DMA1_Channel1_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_adc1);
    /* USER CODE BEGIN DMA1_Channel1_IRQn 1 */

    /* USER CODE END DMA1_Channel1_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel2 global interrupt.
  */
void DMA1_Channel2_IRQHandler(void)
{
    /* USER CODE BEGIN DMA1_Channel2_IRQn 0 */

    /* USER CODE END DMA1_Channel2_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_usart3_tx);
    /* USER CODE BEGIN DMA1_Channel2_IRQn 1 */

    /* USER CODE END DMA1_Channel2_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel3 global interrupt.
  */
void DMA1_Channel3_IRQHandler(void)
{
    /* USER CODE BEGIN DMA1_Channel3_IRQn 0 */

    /* USER CODE END DMA1_Channel3_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_usart3_rx);
    /* USER CODE BEGIN DMA1_Channel3_IRQn 1 */

    /* USER CODE END DMA1_Channel3_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel4 global interrupt.
  */
void DMA1_Channel4_IRQHandler(void)
{
    /* USER CODE BEGIN DMA1_Channel4_IRQn 0 */

    /* USER CODE END DMA1_Channel4_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_usart1_tx);
    /* USER CODE BEGIN DMA1_Channel4_IRQn 1 */

    /* USER CODE END DMA1_Channel4_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel5 global interrupt.
  */
void DMA1_Channel5_IRQHandler(void)
{
    /* USER CODE BEGIN DMA1_Channel5_IRQn 0 */

    /* USER CODE END DMA1_Channel5_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_usart1_rx);
    /* USER CODE BEGIN DMA1_Channel5_IRQn 1 */

    /* USER CODE END DMA1_Channel5_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel6 global interrupt.
  */
void DMA1_Channel6_IRQHandler(void)
{
    /* USER CODE BEGIN DMA1_Channel6_IRQn 0 */

    /* USER CODE END DMA1_Channel6_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_usart2_rx);
    /* USER CODE BEGIN DMA1_Channel6_IRQn 1 */

    /* USER CODE END DMA1_Channel6_IRQn 1 */
}

/**
  * @brief This function handles DMA1 channel7 global interrupt.
  */
void DMA1_Channel7_IRQHandler(void)
{
    /* USER CODE BEGIN DMA1_Channel7_IRQn 0 */

    /* USER CODE END DMA1_Channel7_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_usart2_tx);
    /* USER CODE BEGIN DMA1_Channel7_IRQn 1 */

    /* USER CODE END DMA1_Channel7_IRQn 1 */
}

/**
  * @brief This function handles CAN1 TX interrupt.
  */
void CAN1_TX_IRQHandler(void)
{
    /* USER CODE BEGIN CAN1_TX_IRQn 0 */

    /* USER CODE END CAN1_TX_IRQn 0 */
    HAL_CAN_IRQHandler(&hcan1);
    /* USER CODE BEGIN CAN1_TX_IRQn 1 */

    /* USER CODE END CAN1_TX_IRQn 1 */
}

/**
  * @brief This function handles CAN1 RX0 interrupt.
  */
void CAN1_RX0_IRQHandler(void)
{
    /* USER CODE BEGIN CAN1_RX0_IRQn 0 */

    /* USER CODE END CAN1_RX0_IRQn 0 */
    HAL_CAN_IRQHandler(&hcan1);
    /* USER CODE BEGIN CAN1_RX0_IRQn 1 */

    /* USER CODE END CAN1_RX0_IRQn 1 */
}

/**
  * @brief This function handles CAN1 RX1 interrupt.
  */
void CAN1_RX1_IRQHandler(void)
{
    /* USER CODE BEGIN CAN1_RX1_IRQn 0 */

    /* USER CODE END CAN1_RX1_IRQn 0 */
    HAL_CAN_IRQHandler(&hcan1);
    /* USER CODE BEGIN CAN1_RX1_IRQn 1 */

    /* USER CODE END CAN1_RX1_IRQn 1 */
}

/**
  * @brief This function handles CAN1 SCE interrupt.
  */
void CAN1_SCE_IRQHandler(void)
{
    /* USER CODE BEGIN CAN1_SCE_IRQn 0 */

    /* USER CODE END CAN1_SCE_IRQn 0 */
    HAL_CAN_IRQHandler(&hcan1);
    /* USER CODE BEGIN CAN1_SCE_IRQn 1 */

    /* USER CODE END CAN1_SCE_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[9:5] interrupts.
  */
void EXTI9_5_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI9_5_IRQn 0 */

    /* USER CODE END EXTI9_5_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(M2_IRQ_Pin);
    /* USER CODE BEGIN EXTI9_5_IRQn 1 */

    /* USER CODE END EXTI9_5_IRQn 1 */
}

/**
  * @brief This function handles TIM3 global interrupt.
  */
void TIM3_IRQHandler(void)
{
    /* USER CODE BEGIN TIM3_IRQn 0 */

    /* USER CODE END TIM3_IRQn 0 */
    HAL_TIM_IRQHandler(&htim3);
    /* USER CODE BEGIN TIM3_IRQn 1 */

    /* USER CODE END TIM3_IRQn 1 */
}

/**
  * @brief This function handles I2C1 error interrupt.
  */
void I2C1_ER_IRQHandler(void)
{
    /* USER CODE BEGIN I2C1_ER_IRQn 0 */

    /* USER CODE END I2C1_ER_IRQn 0 */
    HAL_I2C_ER_IRQHandler(&hi2c1);
    /* USER CODE BEGIN I2C1_ER_IRQn 1 */

    /* USER CODE END I2C1_ER_IRQn 1 */
}

/**
  * @brief This function handles I2C2 error interrupt.
  */
void I2C2_ER_IRQHandler(void)
{
    /* USER CODE BEGIN I2C2_ER_IRQn 0 */

    /* USER CODE END I2C2_ER_IRQn 0 */
    HAL_I2C_ER_IRQHandler(&hi2c2);
    /* USER CODE BEGIN I2C2_ER_IRQn 1 */

    /* USER CODE END I2C2_ER_IRQn 1 */
}

/**
  * @brief This function handles USART1 global interrupt.
  */
void USART1_IRQHandler(void)
{
    /* USER CODE BEGIN USART1_IRQn 0 */

    /* USER CODE END USART1_IRQn 0 */
    HAL_UART_IRQHandler(&huart1);
    /* USER CODE BEGIN USART1_IRQn 1 */

    /* USER CODE END USART1_IRQn 1 */
}

/**
  * @brief This function handles USART2 global interrupt.
  */
void USART2_IRQHandler(void)
{
    /* USER CODE BEGIN USART2_IRQn 0 */

    /* USER CODE END USART2_IRQn 0 */
    HAL_UART_IRQHandler(&huart2);
    /* USER CODE BEGIN USART2_IRQn 1 */

    /* USER CODE END USART2_IRQn 1 */
}

/**
  * @brief This function handles USART3 global interrupt.
  */
void USART3_IRQHandler(void)
{
    /* USER CODE BEGIN USART3_IRQn 0 */

    /* USER CODE END USART3_IRQn 0 */
    HAL_UART_IRQHandler(&huart3);
    /* USER CODE BEGIN USART3_IRQn 1 */

    /* USER CODE END USART3_IRQn 1 */
}

/**
  * @brief This function handles EXTI line[15:10] interrupts.
  */
void EXTI15_10_IRQHandler(void)
{
    /* USER CODE BEGIN EXTI15_10_IRQn 0 */

    /* USER CODE END EXTI15_10_IRQn 0 */
    HAL_GPIO_EXTI_IRQHandler(RF_COEX_Pin);
    /* USER CODE BEGIN EXTI15_10_IRQn 1 */

    /* USER CODE END EXTI15_10_IRQn 1 */
}

/**
  * @brief This function handles TIM6 global interrupt, DAC channel1 and channel2 underrun error interrupts.
  */
void TIM6_DAC_IRQHandler(void)
{
    /* USER CODE BEGIN TIM6_DAC_IRQn 0 */

    /* USER CODE END TIM6_DAC_IRQn 0 */
    HAL_TIM_IRQHandler(&htim6);
    /* USER CODE BEGIN TIM6_DAC_IRQn 1 */

    /* USER CODE END TIM6_DAC_IRQn 1 */
}

/**
  * @brief This function handles TIM7 global interrupt.
  */
void TIM7_IRQHandler(void)
{
    /* USER CODE BEGIN TIM7_IRQn 0 */

    /* USER CODE END TIM7_IRQn 0 */
    HAL_TIM_IRQHandler(&htim7);
    /* USER CODE BEGIN TIM7_IRQn 1 */

    /* USER CODE END TIM7_IRQn 1 */
}

/**
  * @brief This function handles DMA2 channel3 global interrupt.
  */
void DMA2_Channel3_IRQHandler(void)
{
    /* USER CODE BEGIN DMA2_Channel3_IRQn 0 */

    /* USER CODE END DMA2_Channel3_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_spi1_rx);
    /* USER CODE BEGIN DMA2_Channel3_IRQn 1 */

    /* USER CODE END DMA2_Channel3_IRQn 1 */
}

/**
  * @brief This function handles DMA2 channel4 global interrupt.
  */
void DMA2_Channel4_IRQHandler(void)
{
    /* USER CODE BEGIN DMA2_Channel4_IRQn 0 */

    /* USER CODE END DMA2_Channel4_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_spi1_tx);
    /* USER CODE BEGIN DMA2_Channel4_IRQn 1 */

    /* USER CODE END DMA2_Channel4_IRQn 1 */
}

/**
  * @brief This function handles DMA2 channel6 global interrupt.
  */
void DMA2_Channel6_IRQHandler(void)
{
    /* USER CODE BEGIN DMA2_Channel6_IRQn 0 */

    /* USER CODE END DMA2_Channel6_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_i2c1_rx);
    /* USER CODE BEGIN DMA2_Channel6_IRQn 1 */

    /* USER CODE END DMA2_Channel6_IRQn 1 */
}

/**
  * @brief This function handles DMA2 channel7 global interrupt.
  */
void DMA2_Channel7_IRQHandler(void)
{
    /* USER CODE BEGIN DMA2_Channel7_IRQn 0 */

    /* USER CODE END DMA2_Channel7_IRQn 0 */
    HAL_DMA_IRQHandler(&hdma_i2c1_tx);
    /* USER CODE BEGIN DMA2_Channel7_IRQn 1 */

    /* USER CODE END DMA2_Channel7_IRQn 1 */
}

/**
  * @brief This function handles I2C3 error interrupt.
  */
void I2C3_ER_IRQHandler(void)
{
    /* USER CODE BEGIN I2C3_ER_IRQn 0 */

    /* USER CODE END I2C3_ER_IRQn 0 */
    HAL_I2C_ER_IRQHandler(&hi2c3);
    /* USER CODE BEGIN I2C3_ER_IRQn 1 */

    /* USER CODE END I2C3_ER_IRQn 1 */
}

/**
  * @brief This function handles I2C4 error interrupt.
  */
void I2C4_ER_IRQHandler(void)
{
    /* USER CODE BEGIN I2C4_ER_IRQn 0 */

    /* USER CODE END I2C4_ER_IRQn 0 */
    HAL_I2C_ER_IRQHandler(&hi2c4);
    /* USER CODE BEGIN I2C4_ER_IRQn 1 */

    /* USER CODE END I2C4_ER_IRQn 1 */
}

/**
  * @brief This function handles CAN2 TX interrupt.
  */
void CAN2_TX_IRQHandler(void)
{
    /* USER CODE BEGIN CAN2_TX_IRQn 0 */

    /* USER CODE END CAN2_TX_IRQn 0 */
    HAL_CAN_IRQHandler(&hcan2);
    /* USER CODE BEGIN CAN2_TX_IRQn 1 */

    /* USER CODE END CAN2_TX_IRQn 1 */
}

/**
  * @brief This function handles CAN2 RX0 interrupt.
  */
void CAN2_RX0_IRQHandler(void)
{
    /* USER CODE BEGIN CAN2_RX0_IRQn 0 */

    /* USER CODE END CAN2_RX0_IRQn 0 */
    HAL_CAN_IRQHandler(&hcan2);
    /* USER CODE BEGIN CAN2_RX0_IRQn 1 */

    /* USER CODE END CAN2_RX0_IRQn 1 */
}

/**
  * @brief This function handles CAN2 RX1 interrupt.
  */
void CAN2_RX1_IRQHandler(void)
{
    /* USER CODE BEGIN CAN2_RX1_IRQn 0 */

    /* USER CODE END CAN2_RX1_IRQn 0 */
    HAL_CAN_IRQHandler(&hcan2);
    /* USER CODE BEGIN CAN2_RX1_IRQn 1 */

    /* USER CODE END CAN2_RX1_IRQn 1 */
}

/**
  * @brief This function handles CAN2 SCE interrupt.
  */
void CAN2_SCE_IRQHandler(void)
{
    /* USER CODE BEGIN CAN2_SCE_IRQn 0 */

    /* USER CODE END CAN2_SCE_IRQn 0 */
    HAL_CAN_IRQHandler(&hcan2);
    /* USER CODE BEGIN CAN2_SCE_IRQn 1 */

    /* USER CODE END CAN2_SCE_IRQn 1 */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
