/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright sogwms
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-26     sogwms       first version       
 */

#ifndef _NRF24_REG_DEF_H_
#define _NRF24_REG_DEF_H_

#include <stdint.h>

typedef union {
    uint8_t raw;
    struct {
        // 1: PRX
        uint8_t prim_rx : 1;
        // 1: power up
        uint8_t pwr_up : 1;
        uint8_t crco : 1;
        uint8_t en_crc : 1;
        // 0: max_rt will trigger irq
        uint8_t mask_max_rt : 1;
        // 0: tx_ds will trigger irq
        uint8_t mask_tx_ds : 1;
        // 0: rx_dr will trigger irq
        uint8_t mask_rx_dr : 1;
        uint8_t __rvsd_7_7 : 1;
    };
} reg_config_t;

typedef union {
    uint8_t raw;
    struct {
        uint8_t p0 : 1;
        uint8_t p1 : 1;
        uint8_t p2 : 1;
        uint8_t p3 : 1;
        uint8_t p4 : 1;
        uint8_t p5 : 1;
        uint8_t __rvsd_7_6 : 2;
    };
} reg_en_aa_t;

typedef union {
    uint8_t raw;
    struct {
        uint8_t p0 : 1;
        uint8_t p1 : 1;
        uint8_t p2 : 1;
        uint8_t p3 : 1;
        uint8_t p4 : 1;
        uint8_t p5 : 1;
        uint8_t __rvsd_7_6 : 2;
    };
} reg_en_rxaddr_t;

typedef union {
    uint8_t raw;
    struct {
        uint8_t aw : 2;
        uint8_t __rvsd_7_2 : 6;
    };
} reg_setup_aw_t;

typedef union {
    uint8_t raw;
    struct {
        // Auto-retransmit count
        uint8_t arc : 4;
        // Auto-retransmit delay
        uint8_t ard : 4;
    };
} reg_setup_retr_t;

typedef union {
    uint8_t raw;
    struct {
        uint8_t rf_ch : 7;
        uint8_t __rvsd_7_7 : 1;
    };
} reg_rf_ch_t;

typedef union {
    uint8_t raw;
    struct {
        uint8_t lna_hcurr : 1;
        // output tx power
        uint8_t rf_pwr : 2;
        // air-data-rate
        uint8_t rf_dr : 1;
        uint8_t pll_lock : 1;
        uint8_t __rvsd_7_5 : 3;
    };
} reg_rf_setup_t;

typedef union {
    uint8_t raw;
    struct {
        uint8_t tx_full : 1;
        uint8_t rx_p_no : 3;
        uint8_t max_rt : 1;
        uint8_t tx_ds : 1;
        uint8_t rx_dr : 1;
        uint8_t __rvsd_7_7 : 1;
    };
} reg_status_t;

typedef union {
    uint8_t raw;
    struct {
        uint8_t arc_cnt : 4;
        uint8_t plos_cnt : 4;
    };
} reg_observe_tx_t;

typedef union {
    uint8_t raw;
    struct {
        uint8_t cd : 1;
        uint8_t __rvsd_7_1 : 7;
    };
} reg_cd_t;

typedef union {
    uint8_t raw;
    struct {
        uint8_t rx_empty : 1;
        uint8_t rx_full : 1;
        uint8_t __rvsd_3_2 : 2;
        uint8_t tx_empty : 1;
        uint8_t tx_full : 1;
        uint8_t tx_reuse : 1;
        uint8_t __rvsd_7_7 : 1;
    };
} reg_fifo_status_t;

typedef union {
    uint8_t raw;
    struct {
        uint8_t p0 : 1;
        uint8_t p1 : 1;
        uint8_t p2 : 1;
        uint8_t p3 : 1;
        uint8_t p4 : 1;
        uint8_t p5 : 1;
        uint8_t __rvsd_7_6 : 2;
    };
} reg_dynpd_t;

typedef struct {
    int8_t raw;
    struct {
        uint8_t en_dyn_ack : 1;
        uint8_t en_ack_pay : 1;
        uint8_t en_dpl : 1;
        uint8_t __rvsd_7_3 : 5;
    };
} reg_feature_t;

typedef struct {
    reg_status_t status;
    reg_fifo_status_t fifo_status;
    reg_observe_tx_t observe_tx;
} nrf24_regs_sta_t;

#endif