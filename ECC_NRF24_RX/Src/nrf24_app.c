#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "stm32f4xx_hal.h"
#include "nrf24_hal.h"
#include "nrf24.h"
#include "nrf24_app.h"

const uint8_t MAX_PAYLOAD_LEN = 32;
static const uint8_t nRF24_ADDR[] = { 'U', 'F', 'U' };
nrf24_app_cbk_func nrf24_client_tx_cbk;
nrf24_app_cbk_func nrf24_client_rx_cbk;
nrf24_scenario_t ntf24_scenario;
nrf24_role_t nrf24_role;

typedef struct nrf24_driver_s nrf24_driver_t;

typedef struct nrf24_tx_int_s
{
	void (*cbk)(nrf24_driver_t *driver, nrf24_tx_result_t status);
} nrf24_tx_int_t;

typedef struct nrf24_rx_int_s
{
	void (*cbk)(nrf24_driver_t *driver, uint8_t pipe);
} nrf24_rx_int_t;

typedef struct nrf24_cfg_s
{
	void (*cbk)(void);
} nrf24_cfg_t;

typedef struct nrf24_driver_s
{
	nrf24_tx_int_t tx_int[2];
	nrf24_rx_int_t rx_int[2];
	nrf24_cfg_t cfg[2];
	void (*pre_common_cfg)(void);
	void (*post_common_cfg)(void);
	nrf24_scenario_t scenario;
	nrf24_role_t role;
	nrf24_app_cbk_func client_tx_cbk;
	nrf24_app_cbk_func client_rx_cbk;
} nrf24_driver_t;

static nrf24_driver_t *driver = 0;

#if 0
UART_HandleTypeDef huart3;
#define UART_SendStr(x) HAL_UART_Transmit(&huart3,(uint8_t *)x,strlen((char *)x),100)
#define UART_SendInt(x) { char buf[32]; sprintf(buf,"%d",(int) x); UART_SendStr(buf); }
#define UART_SendBufHex(x,y) { char buf[10]; unsigned int n; for(n = 0 ; n < y ; n++){sprintf(buf,"0x%02X ",(int)x[n]); UART_SendStr(buf);} UART_SendStr("\n"); }
#else
#define UART_SendStr(x)
#define UART_SendInt(x)
#define UART_SendBufHex(x,y)

#endif

static void all_all_tx_int(nrf24_driver_t *driver, nrf24_tx_result_t status)
{
	nrf24_app_cbk_func cbk = driver->client_tx_cbk;

	if(cbk)
		cbk(nRF24_PIPE_INVALID, 0, 0, status);
}

static void unidirack_ptx_rx_int(nrf24_driver_t *driver, uint8_t pipe)
{
	// no data from PRX
}

static void unidirack_prx_rx_int(nrf24_driver_t *driver, uint8_t pipe)
{
	uint8_t len = 0;
	uint8_t buffer[MAX_PAYLOAD_LEN];
	nrf24_app_cbk_func cbk = driver->client_rx_cbk;

	nRF24_CE_L();

	// Pass PTX request to application
	nRF24_ReadPipePayload(pipe, buffer, &len);

	if(cbk)
		cbk(pipe, buffer, MAX_PAYLOAD_LEN, NRF24_APP_OK);

	nRF24_CE_H();
}

static void bidirack_ptx_rx_int(nrf24_driver_t *driver, uint8_t pipe)
{
	uint8_t buffer[MAX_PAYLOAD_LEN];
	nrf24_app_cbk_func cbk = driver->client_rx_cbk;

	nRF24_CE_L();

	// Read answer from PRX RX FIFO and send to application
	nRF24_ReadRxFifoPayload(buffer, MAX_PAYLOAD_LEN);

	if(cbk)
		cbk(nRF24_PIPETX, buffer, MAX_PAYLOAD_LEN, NRF24_APP_OK);

	nRF24_CE_H();
}

static void bidirack_prx_rx_int(nrf24_driver_t *driver, uint8_t pipe)
{
	uint8_t len = 0;
	uint8_t buffer[MAX_PAYLOAD_LEN];
	nrf24_app_cbk_func cbk = driver->client_rx_cbk;

	nRF24_CE_L();

	// Pass PTX request to application and transmit PRX response.
	// Answer must be coded over the same buffer.
	nRF24_ReadPipePayload(pipe, buffer, &len);

	if(cbk)
		cbk(pipe, buffer, MAX_PAYLOAD_LEN, NRF24_APP_OK);

	nRF24_WriteAckPayload(pipe, buffer, len);

	nRF24_CE_H();
}

static void all_all_pre_common_cfg(void)
{
	nRF24_SetRFChannel(40);
	nRF24_SetDataRate(nRF24_DR_1Mbps);
	nRF24_SetCRCScheme(nRF24_CRC_2byte);
	nRF24_SetAddrWidth(3);
	nRF24_SetAutoRetr(nRF24_ARD_500us, 4);
	nRF24_SetTXPower(nRF24_TXPWR_0dBm);
}

static void unidirack_all_pre_common_cfg(void)
{
	all_all_pre_common_cfg();
	nRF24_SetFeatureReg(0);
}

static void bidirack_all_pre_common_cfg(void)
{
	all_all_pre_common_cfg();
	nRF24_SetFeatureReg(nRF24_MASK_EN_DPL | nRF24_MASK_EN_ACK_PAY);
}

static void all_all_post_common_cfg(void)
{
	nRF24_CE_H();
	nRF24_ClearIRQFlags();
	nRF24_SetPowerMode(nRF24_PWR_UP);
	nRF24_GPIO_Interrupt_Set(true);
}

static void all_ptx_cfg(void)
{
	nRF24_SetAddr(nRF24_PIPETX, nRF24_ADDR);
	nRF24_SetAddr(nRF24_PIPE0, nRF24_ADDR);
	nRF24_EnableAA(nRF24_PIPE0);
	nRF24_SetOperationalMode(nRF24_MODE_TX);
}

static void all_prx_cfg(void)
{
	nRF24_SetAddr(nRF24_PIPE0, nRF24_ADDR); // program address for pipe
	nRF24_SetRXPipe(nRF24_PIPE0, nRF24_AA_ON, MAX_PAYLOAD_LEN); // Auto-ACK: enabled, payload length: MAX_PAYLOAD_LEN bytes
	nRF24_SetOperationalMode(nRF24_MODE_RX);
}

void nrf24_app_tx_data_cbk(nrf24_tx_result_t status)
{
	driver->tx_int[driver->role].cbk(driver, status);
}

void nrf24_app_rx_data_cbk(uint8_t pipe)
{
	driver->rx_int[driver->role].cbk(driver, pipe);
}

static nrf24_driver_t driver_set[NRF24_APP_SCENARIO_MAX_NUMBER] =
{
	{   // NRF24_APP_SCENARIO_UNIDIR_ACK
		.tx_int = { [0].cbk = all_all_tx_int,       [1].cbk = all_all_tx_int },
		.rx_int = { [0].cbk = unidirack_ptx_rx_int, [1].cbk = unidirack_prx_rx_int },
		.cfg    = { [0].cbk = all_ptx_cfg,          [1].cbk = all_prx_cfg },
		.pre_common_cfg = unidirack_all_pre_common_cfg,
		.post_common_cfg = all_all_post_common_cfg,
		.scenario = 0,
		.role = 0,
		.client_tx_cbk = 0,
		.client_rx_cbk = 0
	},
	{   // NRF24_APP_SCENARIO_BIDIR_PLD_ACK
		.tx_int = { [0].cbk = all_all_tx_int,       [1].cbk = all_all_tx_int },
		.rx_int = { [0].cbk = bidirack_ptx_rx_int,  [1].cbk = bidirack_prx_rx_int },
		.cfg    = { [0].cbk = all_ptx_cfg,          [1].cbk = all_prx_cfg },
		.pre_common_cfg = bidirack_all_pre_common_cfg,
		.post_common_cfg = all_all_post_common_cfg,
		.scenario = 0,
		.role = 0,
		.client_tx_cbk = 0,
		.client_rx_cbk = 0
	}
};

uint8_t nrf24_app_get_pld_size(void)
{
	return MAX_PAYLOAD_LEN;
}

void nrf24_app_tx_data(uint8_t *pBuf, uint8_t length)
{
	nRF24_CE_L();
	nRF24_WritePayload(pBuf, length);
	nRF24_CE_H(); // CE must be held high at least 10us
}

void nrf24_app_init(nrf24_scenario_t scenario, nrf24_role_t role, nrf24_app_cbk_func tx_cbk, nrf24_app_cbk_func rx_cbk)
{
	driver = &driver_set[scenario];
	driver->scenario = scenario;
	driver->role = role;
	driver->client_tx_cbk = tx_cbk;
	driver->client_rx_cbk = rx_cbk;
	nRF24_GPIO_Init();
	nRF24_Init();

	driver->pre_common_cfg();
	driver->cfg[driver->role].cbk();
	driver->post_common_cfg();
}
