/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-05-23     sogwms       the first version
 * 2020-02-02     sogwms       refactor to object-oriented and make simplification and ...
 */
#ifndef __NRF24L01_H__
#define __NRF24L01_H__

#include "nrf24l01_port.h"

#define NRF24_DEFAULT_PIPE      NRF24_PIPE_0

enum 
{
    NRF24_PIPE_NONE = 8,
    NRF24_PIPE_0 = 0,
    NRF24_PIPE_1,
    NRF24_PIPE_2,
    NRF24_PIPE_3,
    NRF24_PIPE_4,
    NRF24_PIPE_5,
};

typedef enum
{
    ROLE_NONE = 2,
    ROLE_PTX = 0,
    ROLE_PRX = 1,
} nrf24_role_et;

typedef enum
{
    MODE_POWER_DOWN,
    MODE_STANDBY,
    MODE_TX,
    MODE_RX,
} nrf24_mode_et;

typedef enum
{
    // CRC_NONE = 2,
    CRC_1_BYTE = 0,
    CRC_2_BYTE = 1,
} nrf24_crc_et;

typedef enum
{
    RF_POWER_N18dBm = 0,
    RF_POWER_N12dBm = 0x1,
    RF_POWER_N6dBm  = 0x2,
    RF_POWER_0dBm   = 0x3,
} nrf24_power_et;

typedef enum
{
    ADR_1Mbps = 0,
    ADR_2Mbps = 1,
} nrf24_adr_et;

/* User-oriented configuration */
struct nrf24_cfg
{
    nrf24_role_et role;
    nrf24_power_et power;
    nrf24_crc_et crc;
    nrf24_adr_et adr;
    uint8_t channel :7;    //range: 0 ~ 127 (frequency:)

    int _irq_pin;
    
    uint8_t txaddr[5];

    struct {
        uint8_t bl_enabled;
        uint8_t addr[5];
    } rxpipe0;
    
    struct {
        uint8_t bl_enabled;
        uint8_t addr[5];
    } rxpipe1;

    struct {
        uint8_t bl_enabled;
        uint8_t addr;
    } rxpipe2;
    
    struct {
        uint8_t bl_enabled;
        uint8_t addr;
    } rxpipe3;
    
    struct {
        uint8_t bl_enabled;
        uint8_t addr;
    } rxpipe4;
    
    struct {
        uint8_t bl_enabled;
        uint8_t addr;
    } rxpipe5;
};

typedef struct nrf24_cfg *nrf24_cfg_t;

typedef struct nrf24 *nrf24_t;

struct nrf24_callback
{
    void (*rx_ind)(nrf24_t nrf24, uint8_t *data, uint8_t len, int pipe);
    void (*tx_done)(nrf24_t nrf24, int pipe);
};

struct nrf24
{
    struct hal_nrf24_port halport;
    struct nrf24_cfg cfg;
    struct nrf24_callback cb;

    struct {
        uint8_t activated_features      :1;
        uint8_t using_irq               :1;
    } flags;

    uint8_t status;

    rt_sem_t sem;   // irq
    rt_sem_t send_sem;
};

// int nrf24_update_txaddr(nrf24_t nrf24, uint8_t addr[5]);
// int nrf24_update_rxaddr(nrf24_t nrf24, int pipe, uint8_t addr[5]);

int nrf24_init(nrf24_t nrf24, char *spi_dev_name, int ce_pin, int irq_pin, const struct nrf24_callback *cb, const nrf24_cfg_t cfg);
nrf24_t nrf24_create(char *spi_dev_name, int ce_pin, int irq_pin, const struct nrf24_callback *cb, const nrf24_cfg_t cfg);

int nrf24_default_init(nrf24_t nrf24, char *spi_dev_name, int ce_pin, int irq_pin, const struct nrf24_callback *cb, nrf24_role_et role);
nrf24_t nrf24_default_create(char *spi_dev_name, int ce_pin, int irq_pin, const struct nrf24_callback *cb, nrf24_role_et role);

void nrf24_enter_power_down_mode(nrf24_t nrf24);
void nrf24_enter_power_up_mode(nrf24_t nrf24);

int nrf24_fill_default_config_on(nrf24_cfg_t cfg);
int nrf24_send_data(nrf24_t nrf24, uint8_t *data, uint8_t len, uint8_t pipe);
int nrf24_run(nrf24_t nrf24);

#endif // __NRF24L01_H__
