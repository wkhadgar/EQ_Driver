/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    tim.c
  * @brief   This file provides code for the configuration
  *          of the TIM instances.
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
/* Includes ------------------------------------------------------------------*/
#include "tim.h"

/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;
TIM_HandleTypeDef htim4;

/* TIM2 init function */
void MX_TIM2_Init(void) {
	
	/* USER CODE BEGIN TIM2_Init 0 */
	
	/* USER CODE END TIM2_Init 0 */
	
	TIM_ClockConfigTypeDef sClockSourceConfig = {0};
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};
	
	/* USER CODE BEGIN TIM2_Init 1 */
	
	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 360 - 1;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 28125 - 1;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_4) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */
	
	/* USER CODE END TIM2_Init 2 */
	HAL_TIM_MspPostInit(&htim2);
	
}

/* TIM3 init function */
void MX_TIM3_Init(void) {
	
	/* USER CODE BEGIN TIM3_Init 0 */
	
	/* USER CODE END TIM3_Init 0 */
	
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};
	
	/* USER CODE BEGIN TIM3_Init 1 */
	
	/* USER CODE END TIM3_Init 1 */
	htim3.Instance = TIM3;
	htim3.Init.Prescaler = 360 - 1;
	htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim3.Init.Period = 28125 - 1;
	htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_PWM_Init(&htim3) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM3_Init 2 */
	
	/* USER CODE END TIM3_Init 2 */
	HAL_TIM_MspPostInit(&htim3);
	
}

/* TIM4 init function */
void MX_TIM4_Init(void) {
	
	/* USER CODE BEGIN TIM4_Init 0 */
	
	/* USER CODE END TIM4_Init 0 */
	
	TIM_MasterConfigTypeDef sMasterConfig = {0};
	TIM_OC_InitTypeDef sConfigOC = {0};
	
	/* USER CODE BEGIN TIM4_Init 1 */
	
	/* USER CODE END TIM4_Init 1 */
	htim4.Instance = TIM4;
	htim4.Init.Prescaler = 36000 - 1;
	htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim4.Init.Period = 4000 - 1;
	htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_PWM_Init(&htim4) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_3) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM4_Init 2 */
	
	/* USER CODE END TIM4_Init 2 */
	HAL_TIM_MspPostInit(&htim4);
	
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* tim_baseHandle) {
	
	if (tim_baseHandle->Instance == TIM2) {
		/* USER CODE BEGIN TIM2_MspInit 0 */
		
		/* USER CODE END TIM2_MspInit 0 */
		/* TIM2 clock enable */
		__HAL_RCC_TIM2_CLK_ENABLE();
		
		/* TIM2 interrupt Init */
		HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM2_IRQn);
		/* USER CODE BEGIN TIM2_MspInit 1 */
		
		/* USER CODE END TIM2_MspInit 1 */
	}
}

void HAL_TIM_PWM_MspInit(TIM_HandleTypeDef* tim_pwmHandle) {
	
	if (tim_pwmHandle->Instance == TIM3) {
		/* USER CODE BEGIN TIM3_MspInit 0 */
		
		/* USER CODE END TIM3_MspInit 0 */
		/* TIM3 clock enable */
		__HAL_RCC_TIM3_CLK_ENABLE();
		
		/* TIM3 interrupt Init */
		HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
		HAL_NVIC_EnableIRQ(TIM3_IRQn);
		/* USER CODE BEGIN TIM3_MspInit 1 */
		
		/* USER CODE END TIM3_MspInit 1 */
	} else if (tim_pwmHandle->Instance == TIM4) {
		/* USER CODE BEGIN TIM4_MspInit 0 */
		
		/* USER CODE END TIM4_MspInit 0 */
		/* TIM4 clock enable */
		__HAL_RCC_TIM4_CLK_ENABLE();
		/* USER CODE BEGIN TIM4_MspInit 1 */
		
		/* USER CODE END TIM4_MspInit 1 */
	}
}

void HAL_TIM_MspPostInit(TIM_HandleTypeDef* timHandle) {
	
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	if (timHandle->Instance == TIM2) {
		/* USER CODE BEGIN TIM2_MspPostInit 0 */
		
		/* USER CODE END TIM2_MspPostInit 0 */
		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**TIM2 GPIO Configuration
		PB11     ------> TIM2_CH4
		*/
		GPIO_InitStruct.Pin = M2_STEP_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(M2_STEP_GPIO_Port, &GPIO_InitStruct);
		
		__HAL_AFIO_REMAP_TIM2_PARTIAL_2();
		
		/* USER CODE BEGIN TIM2_MspPostInit 1 */
		
		/* USER CODE END TIM2_MspPostInit 1 */
	} else if (timHandle->Instance == TIM3) {
		/* USER CODE BEGIN TIM3_MspPostInit 0 */
		
		/* USER CODE END TIM3_MspPostInit 0 */
		
		__HAL_RCC_GPIOA_CLK_ENABLE();
		/**TIM3 GPIO Configuration
		PA6     ------> TIM3_CH1
		*/
		GPIO_InitStruct.Pin = M1_STEP_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(M1_STEP_GPIO_Port, &GPIO_InitStruct);
		
		/* USER CODE BEGIN TIM3_MspPostInit 1 */
		
		/* USER CODE END TIM3_MspPostInit 1 */
	} else if (timHandle->Instance == TIM4) {
		/* USER CODE BEGIN TIM4_MspPostInit 0 */
		
		/* USER CODE END TIM4_MspPostInit 0 */
		
		__HAL_RCC_GPIOB_CLK_ENABLE();
		/**TIM4 GPIO Configuration
		PB8     ------> TIM4_CH3
		*/
		GPIO_InitStruct.Pin = LED_PWM_Pin;
		GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(LED_PWM_GPIO_Port, &GPIO_InitStruct);
		
		/* USER CODE BEGIN TIM4_MspPostInit 1 */
		
		/* USER CODE END TIM4_MspPostInit 1 */
	}
	
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* tim_baseHandle) {
	
	if (tim_baseHandle->Instance == TIM2) {
		/* USER CODE BEGIN TIM2_MspDeInit 0 */
		
		/* USER CODE END TIM2_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM2_CLK_DISABLE();
		
		/* TIM2 interrupt Deinit */
		HAL_NVIC_DisableIRQ(TIM2_IRQn);
		/* USER CODE BEGIN TIM2_MspDeInit 1 */
		
		/* USER CODE END TIM2_MspDeInit 1 */
	}
}

void HAL_TIM_PWM_MspDeInit(TIM_HandleTypeDef* tim_pwmHandle) {
	
	if (tim_pwmHandle->Instance == TIM3) {
		/* USER CODE BEGIN TIM3_MspDeInit 0 */
		
		/* USER CODE END TIM3_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM3_CLK_DISABLE();
		
		/* TIM3 interrupt Deinit */
		HAL_NVIC_DisableIRQ(TIM3_IRQn);
		/* USER CODE BEGIN TIM3_MspDeInit 1 */
		
		/* USER CODE END TIM3_MspDeInit 1 */
	} else if (tim_pwmHandle->Instance == TIM4) {
		/* USER CODE BEGIN TIM4_MspDeInit 0 */
		
		/* USER CODE END TIM4_MspDeInit 0 */
		/* Peripheral clock disable */
		__HAL_RCC_TIM4_CLK_DISABLE();
		/* USER CODE BEGIN TIM4_MspDeInit 1 */
		
		/* USER CODE END TIM4_MspDeInit 1 */
	}
}

/* USER CODE BEGIN 1 */

/**
 * @note
 * CLK = 72.000.000 Hz
 * TIM2 Prescaler = 360
 * TIM3 Prescaler = 360
 * TIM4 Prescaler = 360000
 *
 * Resolution TIM2 CLK = 200000 Hz (5us) (M2_STEP)
 * Resolution TIM3 CLK = 200000 Hz (5us) (M1_STEP)
 * Resolution TIM4 CLK = 2000 Hz (500us) (LED)
 */

void led_set_slow_blink(void) {
	TIM4->ARR = (4000 - 1); //Auto Reload Register set to 4000 -> reload every 2 seconds
	TIM4->CCR3 = (2000 - 1); //Amount of ticks while on, 2000 -> 1 seconds
}

void led_set_slow_fast_blink(void) {
	TIM4->ARR = (30000 - 1); //Auto Reload Register set to 30000 -> reload every 15 seconds
	TIM4->CCR3 = (250 - 1); //Amount of ticks while on, 250 -> 0.125 seconds
}

void led_set_fast_blink(void) {
	TIM4->ARR = (1000 - 1); //Auto Reload Register set to 1000 -> reload every 0.5 seconds
	TIM4->CCR3 = (500 - 1); //Amount of ticks while on, 500 -> 0.25 seconds
}

void led_start_blink(void) {
	HAL_TIM_PWM_Start_IT(&htim4, TIM_CHANNEL_3); //Start the output
}

void led_stop_blink(void) {
	HAL_TIM_PWM_Stop_IT(&htim4, TIM_CHANNEL_3); //Stop the output
}

/* USER CODE END 1 */