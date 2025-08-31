/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright sogwms
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-26     sogwms       first version       
 */

#ifndef NRF24L01_H
#define NRF24L01_H

#include <stdint.h>
#include "nrf24l01_dep.h"
#include "nrf24l01_reg.h"

/* Routine Result definitions */
typedef enum {
    NRF24_STA_NONE        = 0,
    NRF24_STA_TX_SENT     = (1 << 0),
    NRF24_STA_HAS_RXDATA  = (1 << 1),
    NRF24_STA_TX_FAIL     = (1 << 2),
    NRF24_STA_TX_RX_OK    = NRF24_STA_TX_SENT | NRF24_STA_HAS_RXDATA,
} nrf24_status_enum_t;

typedef enum {
    NRF24_ROLE_PTX = 0,
    NRF24_ROLE_PRX = 1,
} nrf24_role_enum_t;

typedef enum {
    NRF24_ADR_1Mbps = 0,
    NRF24_ADR_2Mbps = 1,
} nrf24_adr_enum_t;

typedef enum
{
    NRF24_RF_POWER_N18dBm = 0,
    NRF24_RF_POWER_N12dBm = 0x1,
    NRF24_RF_POWER_N6dBm  = 0x2,
    NRF24_RF_POWER_0dBm   = 0x3,
} nrf24_rfpower_enum_t;

typedef struct {
    // RX FIFO empty flag 
    uint8_t rx_empty   : 1;  
    // RX FIFO full flag 
    uint8_t rx_full    : 1;  
    uint8_t            : 2;  
    // TX FIFO empty flag 
    uint8_t tx_empty   : 1;  
    // TX FIFO full flag 
    uint8_t tx_full    : 1;  
    uint8_t            : 1;  
    uint8_t            : 1;  
} nrf24_fifosta_t;

typedef struct { 
    // auto retransmit count
    uint8_t arc_cnt : 4;
    // package lost count
    uint8_t plos_cnt : 4;
} nrf24_observe_t;

typedef struct {
    uint8_t enable;
    uint8_t enable_aa; // auto ack
    union {
        uint8_t addr[5]; // for pipe 0-1
        uint8_t addr_lsb; // for pipe 2-5
    };
} nrf24_rxpipe_cfg_t;

typedef struct {
    uint8_t reg;
    uint8_t val;
}nrf24_regval_t;

typedef struct {
    // air-data-rate
    nrf24_adr_enum_t rf_adr;

    // rf output power
    nrf24_rfpower_enum_t rf_power;

    // rf channel (0 ~ 125)
    uint8_t rf_channel;

    // tx addr
    uint8_t tx_addr[5];

    nrf24_rxpipe_cfg_t rxpipes[6];
} nrf24_user_cfg_t;

typedef struct {
    nrf24_dep_t dep; // Note: keep as the first member
    nrf24_role_enum_t role;

    uint8_t ack_pipe; // PRX txfifo target pipe
    uint8_t is_radio_on;
} nrf24_t;


extern nrf24_regval_t nrf24_default_regval_list[];
extern const int nrf24_default_regval_list_num;

/* Using C11 features to enhace checking */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
    _Static_assert(sizeof(nrf24_fifosta_t) == 1, "nrf24_fifosta_t must be 1 byte");
#endif

/********/
/* Edge */
/********/

int nrf24_global_init(void);
int nrf24_init(nrf24_t *nrf24, nrf24_dep_ops_t *ops, void *ops_ctx);
int nrf24_check_device(nrf24_t *nrf24);
int nrf24_setup(nrf24_t *nrf24, nrf24_role_enum_t role);
int nrf24_setup_full(nrf24_t *nrf24, nrf24_role_enum_t role, const nrf24_user_cfg_t *ucfg, const nrf24_regval_t *regvals, int regvals_num);
void nrf24_deinit(nrf24_t *nrf24);

/**********/
/* Config */
/**********/

void nrf24_usercfg_init_default(nrf24_user_cfg_t *ucfg);
int nrf24_usercfg_read(nrf24_t *nrf24, nrf24_user_cfg_t *ucfg);
int nrf24_usercfg_write(nrf24_t *nrf24, const nrf24_user_cfg_t *ucfg);
int nrf24_usercfg_write_directly(nrf24_t *nrf24, const nrf24_user_cfg_t *ucfg);
int nrf24_usercfg_write_diff_directly(nrf24_t *nrf24, const nrf24_user_cfg_t *old, const nrf24_user_cfg_t *new);

int nrf24_role_switch(nrf24_t *nrf24, nrf24_role_enum_t role);
int nrf24_role_switch_directly(nrf24_t *nrf24, nrf24_role_enum_t role);
int nrf24_role_is_prx(nrf24_t *nrf24);
int nrf24_role_is_ptx(nrf24_t *nrf24);

/**********/
/* Switch */
/**********/

void nrf24_power_up(nrf24_t *nrf24);
void nrf24_power_down(nrf24_t *nrf24);
void nrf24_radio_on(nrf24_t *nrf24);
void nrf24_radio_off(nrf24_t *nrf24);

/**********/
/* Status */
/**********/

uint8_t nrf24_read_status(nrf24_t *nrf24);
void nrf24_clear_status(nrf24_t *nrf24, uint8_t sta);
uint8_t nrf24_read_and_clear_status(nrf24_t *nrf24);

void nrf24_clear_all_status(nrf24_t *nrf24);
void nrf24_clear_txfail_flag(nrf24_t *nrf24);
void nrf24_clear_all_fifo(nrf24_t *nrf24);
void nrf24_clear_all(nrf24_t *nrf24);

nrf24_fifosta_t nrf24_read_fifosta(nrf24_t *nrf24);
nrf24_observe_t nrf24_read_observe(nrf24_t *nrf24);

/***********/
/* IO/FIFO */
/***********/

int nrf24_txfifo_has_space(nrf24_t *nrf24);
int nrf24_txfifo_is_empty(nrf24_t *nrf24);
void nrf24_txfifo_flush(nrf24_t *nrf24);
int nrf24_txfifo_prx_write(nrf24_t *nrf24, const uint8_t *data, uint8_t len, uint8_t pipe);
int nrf24_txfifo_ptx_write(nrf24_t *nrf24, const uint8_t *data, uint8_t len);
int nrf24_txfifo_ptx_write_no_ack(nrf24_t *nrf24, const uint8_t *data, uint8_t len);

void nrf24_txfifo_set_prx_ackpipe(nrf24_t *nrf24, uint8_t pipe);
uint8_t nrf24_txfifo_get_prx_ackpipe(nrf24_t *nrf24);
int nrf24_txfifo_write(nrf24_t *nrf24, const uint8_t *data, uint8_t len);

int nrf24_rxfifo_has_data(nrf24_t *nrf24);
int nrf24_rxfifo_is_full(nrf24_t *nrf24);
int nrf24_rxfifo_read(nrf24_t *nrf24, uint8_t *buf, uint8_t *data_len, uint8_t *pipe);
void nrf24_rxfifo_flush(nrf24_t *nrf24);

/***********/
/* Running */
/***********/

nrf24_status_enum_t nrf24_status_routine(nrf24_t *nrf24, uint8_t sta);

/***********/
/* Utils */
/***********/

uint8_t nrf24_read_reg(nrf24_t *nrf24, uint8_t reg);
int nrf24_read_regs(nrf24_t *nrf24, uint8_t reg, uint8_t *vals, uint8_t len);
int nrf24_write_reg(nrf24_t *nrf24, uint8_t reg, uint8_t val);
int nrf24_write_regs(nrf24_t *nrf24, uint8_t reg, uint8_t *vals, uint8_t len);
int nrf24_write_reg_list(nrf24_t *nrf24, const nrf24_regval_t *regvals, int num);

#endif // NRF24L01_H