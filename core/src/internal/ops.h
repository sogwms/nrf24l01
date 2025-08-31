/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright sogwms
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-26     sogwms       first version       
 */

#include "nrf24l01_dep.h"
#include "nrf24l01_reg.h"

static inline int dep_init(nrf24_dep_t *dep) 
{
    if (dep->ops->init != 0) {
        return dep->ops->init(dep->ctx);
    }
    return 0;
}

static inline void dep_deinit(nrf24_dep_t *dep) 
{
    if (dep->ops->deinit != 0) {
        dep->ops->deinit(dep->ctx);
    }
}

static inline void set_ce(nrf24_dep_t *dep, char val) 
{
    if (val)
        dep->ops->set_ce_high(dep->ctx);
    else
        dep->ops->set_ce_low(dep->ctx);
}

static inline int read_reg(nrf24_dep_t *dep, uint8_t reg, uint8_t *val) 
{
    uint8_t cmd;

    cmd = NRF24_CMD_R_REG | reg;
    return dep->ops->spi_send_then_recv(dep->ctx, &cmd, 1, val, 1);
}

static inline int write_reg(nrf24_dep_t *dep, uint8_t reg, uint8_t val) 
{
    uint8_t buf[2];
    buf[0] = NRF24_CMD_W_REG | reg;
    buf[1] = val;
    return dep->ops->spi_send(dep->ctx, buf, 2);
}

static inline int read_regs(nrf24_dep_t *dep, uint8_t reg, uint8_t *val, uint8_t len) 
{
    uint8_t cmd;
    cmd = NRF24_CMD_R_REG | reg;
    return dep->ops->spi_send_then_recv(dep->ctx, &cmd, 1, val, len);
}

static inline int write_regs(nrf24_dep_t *dep, uint8_t reg, const uint8_t *val, uint8_t len) 
{
    uint8_t cmd;
    cmd = NRF24_CMD_W_REG | reg;
    return dep->ops->spi_send_then_send(dep->ctx, &cmd, 1, val, len);
}

static inline int send_cmd_read_rx_payload(nrf24_dep_t *dep, uint8_t *buf, uint8_t len)
{
    uint8_t cmd;
    cmd = NRF24_CMD_R_RX_PAYLOAD;
    return dep->ops->spi_send_then_recv(dep->ctx, &cmd, 1, buf, len);
}

static inline int send_cmd_write_tx_payload(nrf24_dep_t *dep, const uint8_t *buf, uint8_t len)
{
    uint8_t cmd;
    cmd = NRF24_CMD_W_TX_PAYLOAD;
    return dep->ops->spi_send_then_send(dep->ctx, &cmd, 1, buf, len);
}


static inline int send_cmd_flush_tx(nrf24_dep_t *dep)
{
    uint8_t cmd;
    cmd = NRF24_CMD_FLUSH_TX;
    return dep->ops->spi_send(dep->ctx, &cmd, 1);
}

static inline int send_cmd_flush_rx(nrf24_dep_t *dep)
{
    uint8_t cmd;
    cmd = NRF24_CMD_FLUSH_RX;
    return dep->ops->spi_send(dep->ctx, &cmd, 1);
}

static inline int send_cmd_reuse_tx_payload(nrf24_dep_t *dep)
{
    uint8_t cmd;
    cmd = NRF24_CMD_REUSE_TX_PL;
    return dep->ops->spi_send(dep->ctx, &cmd, 1);
}

static inline int send_cmd_activate(nrf24_dep_t *dep)
{
    uint8_t buf[2];
    buf[0] = NRF24_CMD_ACTIVATE;
    buf[1] = 0x73;
    return dep->ops->spi_send(dep->ctx, buf, 2);
}

static inline int send_cmd_read_rx_payload_width(nrf24_dep_t *dep)
{
    uint8_t cmd;
    uint8_t val;

    cmd = NRF24_CMD_R_RX_PL_WID;
    dep->ops->spi_send_then_recv(dep->ctx, &cmd, 1, &val, 1);
    return val;
}

static inline int send_cmd_write_ack_payload(nrf24_dep_t *dep, uint8_t pipe, const uint8_t *data, uint8_t len)
{
    uint8_t cmd;
    cmd = NRF24_CMD_W_ACK_PAYLOAD | pipe;
    return dep->ops->spi_send_then_send(dep->ctx, &cmd, 1, data, len);
}

static inline int send_cmd_write_tx_payload_no_ack(nrf24_dep_t *dep, const uint8_t *data, uint8_t len) 
{
    uint8_t cmd;
    cmd = NRF24_CMD_W_TX_PAYLOAD_NO_ACK;
    return dep->ops->spi_send_then_send(dep->ctx, &cmd, 1, data, len);   
}
