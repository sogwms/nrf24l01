#ifndef NRF24L01_DEP_IMPL_H
#define NRF24L01_DEP_IMPL_H

#include "nrf24l01.h"

struct nrf24_depimpl_ctx {
    char need_gen_spidev;
    char *spi_bus;
    int cs_pin;
    char *spi_dev_name;
    struct rt_spi_device *spi_dev_handle;
    int ce_pin;
};

void nrf24_depimpl_init_ctx(struct nrf24_depimpl_ctx *ctx, char *spi_dev_name, int ce_pin, char *spi_bus, int cs_pin);
void nrf24_depimpl_init_ctx_with_spidev(struct nrf24_depimpl_ctx *ctx, char *spi_dev_name,int ce_pin);

nrf24_dep_ops_t *nrf24_depimpl_get_ops(void);

#endif