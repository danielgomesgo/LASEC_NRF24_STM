#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "nrf24_hal.h"
#include "nrf24.h"

extern SPI_HandleTypeDef nRF24_SPI_PORT;

void EXTI1_IRQHandler(void)
{
//	HAL_GPIO_TogglePin(GPIOD,GPIO_PIN_13);
	nRF24_Interrupt();
	__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_1);
}

void nRF24_GPIO_Interrupt_Set(bool state)
{
	if(state)
		HAL_NVIC_EnableIRQ(EXTI1_IRQn);
	else
		HAL_NVIC_DisableIRQ(EXTI1_IRQn);
}
// Configure the GPIO lines of the nRF24L01 transceiver
// note: IRQ pin must be configured separately
void nRF24_GPIO_Init(void)
{
	// init already done at main.c
	nRF24_CSN_H();
	nRF24_CE_L();
}

// Low level SPI transmit/receive function (hardware depended)
// input:
//   data - value to transmit via SPI
// return: value received from SPI
uint8_t nRF24_LL_RW(uint8_t tx_data)
{
	uint8_t rx_data;
	if(HAL_SPI_TransmitReceive(&nRF24_SPI_PORT,&tx_data,&rx_data,1,100) == HAL_OK)
		return rx_data;
	else
		return 0;
}
