/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright sogwms
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-26     sogwms       first version       
 */

#ifndef NRF24L01_DEP_H
#define NRF24L01_DEP_H

#include <stdint.h>

/* SPI specification */
#ifdef NRF24L01_Si24R1_DEVICE
#define NRF24L01_SPI_MAX_SPEED_HZ 10000000
#else
#define NRF24L01_SPI_MAX_SPEED_HZ  8000000
#endif
#define NRF24L01_SPI_MODE 0

typedef struct nrf24_dep nrf24_dep_t;
typedef struct nrf24_dep_ops nrf24_dep_ops_t;

struct nrf24_dep {
    void *ctx;
    nrf24_dep_ops_t *ops;
};

struct nrf24_dep_ops {
    int (*init)(void *ctx);
    void (*deinit)(void *ctx);
    int (*spi_send)(void *ctx, const uint8_t *buf, uint8_t len);
    int (*spi_send_then_send)(void *ctx, const uint8_t *buf1, uint8_t len1, const uint8_t *buf2, uint8_t len2);
    int (*spi_send_then_recv)(void *ctx, const uint8_t *wbuf, uint8_t wlen, uint8_t *rbuf, uint8_t rlen);
    void (*set_ce_low)(void *ctx);
    void (*set_ce_high)(void *ctx);
};

#endif // NRF24L01_DEP_H
