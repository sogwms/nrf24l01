
#include "nrf24l01_dep_impl.h"

/**
 * @brief Init NRF24 with pre-created spidev. (no actual io with device)
 * @param nrf24      Device instance.
 * @param spi_dev_name SPI device name
 * @param ce_pin     pin (e.g., rt_pin_get("PA00")).
 * @return 0 on success.
 *
 * @note Assumes spi device is already created.
 */
int nrf24_init_ins(nrf24_t *nrf24, char *spi_dev_name, int ce_pin)
{
    nrf24_depimpl_init_ctx_with_spidev(&nrf24->custom_data.depctx, spi_dev_name, ce_pin);
    return nrf24_init(nrf24, nrf24_depimpl_get_ops(), &nrf24->custom_data.depctx);
}

#ifdef NRF24L01_SUPPORT_AUTO_SPIDEV

/**
 * @brief Init NRF24 with auto-created spidev. (no actual io with device)
 * @param nrf24      Device instance.
 * @param spi_dev_name SPI device name (e.g., "spi10").
 * @param ce_pin     pin
 * @param spi_bus_name    SPI bus name 
 * @param cs_pin     Chip-Select pin
 * @return 0 on success.
 *
 * @note This function auto-creates the SPI device.
 */
int nrf24_init_ins_with_auto_spidev(nrf24_t *nrf24, char *spi_dev_name, int ce_pin, char *spi_bus_name, int cs_pin)
{
    nrf24_depimpl_init_ctx(&nrf24->custom_data.depctx, spi_dev_name, ce_pin, spi_bus_name, cs_pin);
    return nrf24_init(nrf24, nrf24_depimpl_get_ops(), &nrf24->custom_data.depctx);
}

#endif