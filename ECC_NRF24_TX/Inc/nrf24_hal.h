#ifndef __NRF24_HAL_H
#define __NRF24_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
// Hardware abstraction layer for NRF24L01+ transceiver (hardware depended functions)
// GPIO pins definition
// GPIO pins initialization and control functions
// SPI transmit functions
#include "main.h"
#include "stm32f3xx_hal.h"

// SPI port peripheral
#define nRF24_SPI_PORT             hspi2

// nRF24 GPIO peripherals
#define nRF24_GPIO_PERIPHERALS     (RCC_APB2ENR_IOPBEN)

// CE (chip enable) pin (PB11)
#define nRF24_CE_PORT              NRF24_CE_GPIO_Port
#define nRF24_CE_PIN               NRF24_CE_Pin
#define nRF24_CE_L()               HAL_GPIO_WritePin(nRF24_CE_PORT, nRF24_CE_PIN, GPIO_PIN_RESET)
#define nRF24_CE_H()               HAL_GPIO_WritePin(nRF24_CE_PORT, nRF24_CE_PIN, GPIO_PIN_SET)

// CSN (chip select negative) pin (PB12)
#define nRF24_CSN_PORT             NRF24_CSN_GPIO_Port
#define nRF24_CSN_PIN              NRF24_CSN_Pin
#define nRF24_CSN_L()              HAL_GPIO_WritePin(nRF24_CSN_PORT, nRF24_CSN_PIN, GPIO_PIN_RESET)
#define nRF24_CSN_H()              HAL_GPIO_WritePin(nRF24_CSN_PORT, nRF24_CSN_PIN, GPIO_PIN_SET)

// IRQ pin (PB10)
#define nRF24_IRQ_PORT             NRF24_INT_GPIO_Port
#define nRF24_IRQ_PIN              GPIO_PIN_1


// Function prototypes
void nRF24_GPIO_Init(void);
uint8_t nRF24_LL_RW(uint8_t data);
void nRF24_GPIO_Interrupt_Set(bool state);

#ifdef __cplusplus
}
#endif

#endif // __NRF24_HAL_H
