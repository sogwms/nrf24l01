#ifndef NRF24L01_EXT_H
#define NRF24L01_EXT_H

#define NRF24L01_ENABLE_CUSTOM_STRUCT_DATA
#define NRF24L01_CUSTOM_STRUCT_DATA_T struct nrf24l01_custom_data

#include "nrf24l01_dep_impl.h"

struct nrf24l01_custom_data
{
    struct nrf24_depimpl_ctx depctx;
};

struct nrf24;

int nrf24_init_ins(struct nrf24 *nrf24, char *spi_dev_name, int ce_pin);
#ifdef NRF24L01_SUPPORT_AUTO_SPIDEV
int nrf24_init_ins_with_auto_spidev(struct nrf24 *nrf24, char *spi_dev_name, int ce_pin, char *spi_bus_name, int cs_pin);
#endif 

#endif // 