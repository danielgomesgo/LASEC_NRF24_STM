/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  ** This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * COPYRIGHT(c) 2017 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f4xx_hal.h"

/* USER CODE BEGIN Includes */
#include "nrf24_hal.h"
#include "nrf24.h"
#include "nrf24_app.h"
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi3;

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
volatile bool new_data;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI3_Init(void);

/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/

/* USER CODE END PFP */
#define RADIO_TX 0
/* USER CODE BEGIN 0 */
volatile bool wait_data;
nrf24_scenario_t scenario;
nrf24_role_t role;

void tx_cbk_func(nRF24_pipe_number_t pipe, uint8_t *buffer, uint8_t length, nrf24_tx_result_t status)
{
	if(scenario == NRF24_APP_SCENARIO_BIDIR_PLD_ACK)
	{
		if((role == NRF24_APP_ROLE_PTX) || (role == NRF24_APP_ROLE_PRX))
		{
			// only checking when last transmission was sent or not, no frame involved
			if(status == NRF24_APP_OK)
			{
				HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_15);
			}
			else
			{
				HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_14);
			}
		}
	}
	else if(scenario == NRF24_APP_SCENARIO_UNIDIR_ACK)
	{
		if(role == NRF24_APP_ROLE_PTX)
		{
			// only checking when last transmission was sent or not, no frame involved
			if(status == NRF24_APP_OK)
			{
				HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_15);
			}
			else
			{
				HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_14);
			}
		}
		else if(role == NRF24_APP_ROLE_PRX)
		{
			// auto ack enabled, no transmissions
		}
	}
}

void rx_cbk_func(nRF24_pipe_number_t pipe, uint8_t *buffer, uint8_t length, nrf24_tx_result_t status)
{
	if(scenario == NRF24_APP_SCENARIO_BIDIR_PLD_ACK)
	{
		if(role == NRF24_APP_ROLE_PTX)
		{
			// Checking PRX answer
			uint8_t n;
			bool result = true;

    		for(n = 0 ; n < 32 ; n++)
    		{
    			if(buffer[n] != 0xFF)
    			{
    				result = false;
    				break;
    			}
    		}

    		wait_data = false;

    		if(result)
    		{
    			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_14);
    		}
    		else
    		{
    			HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_7);
    		}
		}
		else if(role == NRF24_APP_ROLE_PRX)
		{
			uint8_t n;
			// We need to answer a request, status does not matter. Buffer is reused.
    		for(n = 0 ; n < 32 ; n++)
    			buffer[n] = 0xFF;
		}
	}
	else if(scenario == NRF24_APP_SCENARIO_UNIDIR_ACK)
	{
		if(role == NRF24_APP_ROLE_PTX)
		{
			// no data arriving from PRX
		}
		else
		{
			// process your frame here, no ack required and no answer, unidir
   			HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_15);
		}
	}
}


void run(nrf24_scenario_t _scenario, nrf24_role_t _role)
{
	uint8_t n;
	uint8_t buffer[32];

	if(_scenario == NRF24_APP_SCENARIO_BIDIR_PLD_ACK)
	{
	    if(_role == NRF24_APP_ROLE_PTX)
	    {
	    	while(true)
	    	{
	    		uint32_t tmr_out;

	    		for(n = 0 ; n < 32 ; n++)
	    			buffer[n] = 0x00;

	    		wait_data = true;
	    		//data_value = data_value + 1; // expected value

	    		nrf24_app_tx_data(buffer, 32);

	    		tmr_out = 20;
	    		while(wait_data && tmr_out > 0)
	    		{
	    			HAL_Delay(1);
	    			tmr_out--;
	    		}

	    		if(tmr_out == 0)
				{
	    			HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_SET);
	    			HAL_Delay(50);
					HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_RESET);
					HAL_Delay(50);
					HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_SET);
					HAL_Delay(50);
					HAL_GPIO_WritePin(GPIOD,GPIO_PIN_14,GPIO_PIN_RESET);
				}

	    		HAL_Delay(50);
	    	}
	    }
	    else
	    {
			while(true)
			{
				// not to do here, all work is handled in interrupted mode
			}
	    }
	}
	else if(_scenario == NRF24_APP_SCENARIO_UNIDIR_ACK)
	{
	    if(_role == NRF24_APP_ROLE_PTX)
	    {
	    	while(true)
	    	{
	    		for(n = 0 ; n < 32 ; n++)
	    			buffer[n] = 0x00;

	    		nrf24_app_tx_data(buffer, 32);

	    		HAL_Delay(30);
	    	}
	    }
	    else
	    {
			while(true)
			{
				// not to do here, all work is handled in interrupted mode
			}
	    }
	}

}
/* USER CODE END 0 */

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI3_Init();

  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
  /* USER CODE END WHILE */

  /* USER CODE BEGIN 3 */
	  role = NRF24_APP_ROLE_PRX;
	  scenario = NRF24_APP_SCENARIO_BIDIR_PLD_ACK;
	  nrf24_app_init(scenario, role, rx_cbk_func, tx_cbk_func);
	  run(scenario,role);
  }
  /* USER CODE END 3 */

}

/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

    /**Configure the main internal regulator output voltage 
    */
  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 180;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Activate the Over-Drive mode 
    */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Initializes the CPU, AHB and APB busses clocks 
    */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

    /**Configure the Systick interrupt time 
    */
  HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /**Configure the Systick 
    */
  HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

/* SPI3 init function */
static void MX_SPI3_Init(void)
{

  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi3.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi3) != HAL_OK)
  {
    _Error_Handler(__FILE__, __LINE__);
  }

}

/** Configure pins as 
        * Analog 
        * Input 
        * Output
        * EVENT_OUT
        * EXTI
*/
static void MX_GPIO_Init(void)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_14|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, NRF24_CE_Pin|NRF24_CSN_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : PB0 PB14 PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_14|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : NRF24_INT_Pin */
  GPIO_InitStruct.Pin = NRF24_INT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(NRF24_INT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : NRF24_CE_Pin NRF24_CSN_Pin */
  GPIO_InitStruct.Pin = NRF24_CE_Pin|NRF24_CSN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1) 
  {
  }
  /* USER CODE END Error_Handler_Debug */ 
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */

}

#endif

/**
  * @}
  */ 

/**
  * @}
*/ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
