/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright sogwms
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-26     sogwms       first version       
 */

#include "nrf24l01.h"
#include "nrf24l01_reg.h"
#include "nrf24l01_dep.h"
#include "./internal/def.h"
#include "./internal/ops.h"
#include "./internal/cfg.h"
#include "./internal/log.h"

#ifdef CHECK
#undef CHECK
#endif
#define CHECK NRF24_CHECK

/* return value to readable string */
#define R2S(r) (r == 0 ? "ok" : "fail")

/* helper macro for casting a pointer ...*/
#define AS(t,v) ((t)(v))
#define ASU8P(x) AS(uint8_t *, x)
#define ASU8V(x) (*(ASU8P(x)))

#include "./snippets/nrf24l01/reg.inc.c"
#include "./snippets/nrf24l01/mem.inc.c"
#include "./snippets/nrf24l01/usercfg.inc.c"
#include "./snippets/nrf24l01/fifo.inc.c"

uint8_t nrf24_read_reg(nrf24_t *nrf24, uint8_t reg)
{
    uint8_t val;
    read_reg(&nrf24->dep, reg, &val);
    return val;
}

int nrf24_write_reg(nrf24_t *nrf24, uint8_t reg, uint8_t val)
{
    return write_reg(&nrf24->dep, reg, val);
}

int nrf24_write_regs(nrf24_t *nrf24, uint8_t reg, uint8_t *vals, uint8_t len)
{
    return write_regs(&nrf24->dep, reg, vals, len);
}

int nrf24_read_regs(nrf24_t *nrf24, uint8_t reg, uint8_t *vals, uint8_t len)
{
    return read_regs(&nrf24->dep, reg, vals, len);
}

nrf24_fifosta_t nrf24_read_fifosta(nrf24_t *nrf24)
{
    nrf24_fifosta_t fifosta;
    read_reg(&nrf24->dep, NRF24_REG_FIFO_STATUS, ASU8P(&fifosta));
    return fifosta;
}

nrf24_observe_t nrf24_read_observe(nrf24_t *nrf24)
{
    nrf24_observe_t observe;
    read_reg(&nrf24->dep, NRF24_REG_OBSERVE_TX, ASU8P(&observe));
    return observe;
}

void nrf24_power_up(nrf24_t *nrf24)
{
    reg_modify_bits(&nrf24->dep, NRF24_REG_CONFIG, REG_CONFIG_BITMASK_PWR_UP, 1);
}

void nrf24_power_down(nrf24_t *nrf24)
{
    reg_modify_bits(&nrf24->dep, NRF24_REG_CONFIG, REG_CONFIG_BITMASK_PWR_UP, 0);
}

void nrf24_radio_on(nrf24_t *nrf24)
{
    set_ce(&nrf24->dep, 1);
    nrf24->is_radio_on = 1;
}

void nrf24_radio_off(nrf24_t *nrf24)
{
    set_ce(&nrf24->dep, 0);
    nrf24->is_radio_on = 0;
}

/**
 * @brief Read the current status register value from the device.
 *
 * This function result is intended for other driver functions (e.g. `xxx_routine`, `nrf24_clear_status`).
 *
 * @param nrf24  Pointer to the NRF24 device instance.
 * @return       Current status register value.
 */
uint8_t nrf24_read_status(nrf24_t *nrf24)
{
    uint8_t sta;
    read_reg(&nrf24->dep, NRF24_REG_STATUS, &sta);
    return sta;
}

/**
 * @brief Clear the specified status bits in the status register.
 *
 * @param nrf24  Pointer to the NRF24 device instance.
 * @param sta    Status bits to clear (typically obtained via nrf24_read_status).
 * 
 * @note The MAX_RT status flag is not cleared here, as doing so would resume transmission from the TX FIFO.
 */
void nrf24_clear_status(nrf24_t *nrf24, uint8_t sta)
{
    if ((sta & (REG_STATUS_BITMASK_RX_DR | REG_STATUS_BITMASK_TX_DS))) {
        // clear status (not including MAX_RT)
        write_reg(&nrf24->dep, NRF24_REG_STATUS, sta & ~REG_STATUS_BITMASK_MAX_RT); 
    }
}

/**
 * @brief Read and then clear status flags (not clear max_rt flag)
 * 
 * @note The return value is not intended to be directly used by users
 * 
 * @param nrf24 
 * @return int status value
 */
uint8_t nrf24_read_and_clear_status(nrf24_t *nrf24)
{
    uint8_t sta = nrf24_read_status(nrf24);
    nrf24_clear_status(nrf24, sta);
    return sta;
}

/**
 * @brief Clear all status flags
 */
void nrf24_clear_all_status(nrf24_t *nrf24)
{
    uint8_t tmp;
    // clear status flags
    read_reg(&nrf24->dep, NRF24_REG_STATUS, &tmp);
    write_reg(&nrf24->dep, NRF24_REG_STATUS, tmp);

    // clear plos_cnt
    read_reg(&nrf24->dep, NRF24_REG_RF_CH, &tmp);
    write_reg(&nrf24->dep, NRF24_REG_RF_CH, tmp);
}

/**
 * @brief Used after txfail (max_rt) happened to enable sending
 */
void nrf24_clear_txfail_flag(nrf24_t *nrf24)
{
    write_reg(&nrf24->dep, NRF24_REG_STATUS, REG_STATUS_BITMASK_MAX_RT);
}

/**
 * @brief Clear all data
 */
void nrf24_clear_all_fifo(nrf24_t *nrf24)
{
    send_cmd_flush_tx(&nrf24->dep);
    send_cmd_flush_rx(&nrf24->dep);
}

/**
 * @brief Clear all status and data
 */
void nrf24_clear_all(nrf24_t *nrf24) {
    nrf24_clear_all_fifo(nrf24);
    nrf24_clear_all_status(nrf24);
}

static inline int is_valid_pipeno(uint8_t pipeno)
{
    return pipeno <= 5;
}

/**
 * @brief Switch role
 * 
 * @note Clears all status and data as a side effect.
 */
int nrf24_role_switch(nrf24_t *nrf24, nrf24_role_enum_t role)
{
    if (nrf24->role == role) {
        return 0;
    }

    nrf24->role = role;

    nrf24_clear_all(nrf24);

    return reg_modify_bits(&nrf24->dep, NRF24_REG_CONFIG, REG_CONFIG_BITMASK_PRIM_RX, role);
}

/**
 * @brief Switch role directly
 * 
 * @attention Calling this function only when you know what you are doing.
 */
int nrf24_role_switch_directly(nrf24_t *nrf24, nrf24_role_enum_t role)
{
    if (nrf24->role == role) {
        return 0;
    }

    nrf24->role = role;

    return reg_modify_bits(&nrf24->dep, NRF24_REG_CONFIG, REG_CONFIG_BITMASK_PRIM_RX, role);
}

/// @return return `true` if is ROLE PRX
int nrf24_role_is_prx(nrf24_t *nrf24)
{
    return nrf24->role == NRF24_ROLE_PRX;
}

/// @return return `true` if is ROLE PTX
int nrf24_role_is_ptx(nrf24_t *nrf24)
{
    return nrf24->role == NRF24_ROLE_PTX;
}

/**
 * @brief Checks if the connected device is a valid NRF24L01 and hardware (SPI) communication is functional.
 *
 * This function performs basic register read/write tests to confirm both device identity
 * and stable SPI connectivity.
 *
 * @param nrf24 Pointer to the NRF24L01 device structure
 * @return int 0 on success (valid device and working connection), -1 on failure
 */
int nrf24_check_device(nrf24_t *nrf24) 
{
    int ret = 0;
    uint8_t cmd;
    uint8_t addr_backup[5];
    uint8_t addr[5];
    LOG_V("enter %s", __func__);

    /* Backup the rx address */
    read_regs(&nrf24->dep, NRF24_REG_RX_ADDR_P0, addr_backup, 5);

    /* Set new rx address */
    for (int i = 0; i < 5; i++) {
        addr[i] = addr_backup[i] + i;
    }
    write_regs(&nrf24->dep, NRF24_REG_RX_ADDR_P0, addr, 5);
    LOG_V("set rx address: %02x %02x %02x %02x %02x", addr[0], addr[1], addr[2], addr[3], addr[4]);

    /* Get and verify the rx address */
    for (int i = 0; i < 5; i++) {
        addr[i] = 0;
    }
    read_regs(&nrf24->dep, NRF24_REG_RX_ADDR_P0, addr, 5);
    LOG_V("got rx address: %02x %02x %02x %02x %02x", addr[0], addr[1], addr[2], addr[3], addr[4]);

    for (int i = 0; i < 5; i++) {
        if (addr[i] != addr_backup[i] + i) {
            ret = -1;
            break;
        }
    }

    /* Restore the rx address */
    LOG_V("restore backup rx address: %02x %02x %02x %02x %02x", addr_backup[0], addr_backup[1], addr_backup[2], addr_backup[3], addr_backup[4]);
    write_regs(&nrf24->dep, NRF24_REG_RX_ADDR_P0, addr_backup, 5);

    LOG_V("exit %s [%s]", __func__, R2S(ret));
    return ret;
}

/**
 * @brief Parse Status
 * 
 * @return int  
 *              0: nothing to do; 
 * 
 *              NRF24_STA_TX_SENT: TX complete; 
 * 
 *              NRF24_STA_HAS_RXDATA: RX complete; 
 * 
 *              NRF24_STA_TX_RX_OK: TX and RX complete;
 * 
 *              NRF24_STA_TX_FAIL: TX failed; (PTX only)
 */
nrf24_status_enum_t nrf24_status_routine(nrf24_t *nrf24, uint8_t sta)
{
    int ret = 0;

    /* exit when no events */
    if ((sta & 0x7E) == 0x0E) {
        return ret;
    }

    /* deal events */
    if (sta & REG_STATUS_BITMASK_TX_DS) {
        ret |= NRF24_STA_TX_SENT;
    }

    if ((sta & REG_STATUS_BITMASK_RX_DR) || (is_valid_pipeno(byte_get_bits(sta, REG_STATUS_BITMASK_RX_P_NO)))) {
        ret |= NRF24_STA_HAS_RXDATA;
    }

    if (sta & REG_STATUS_BITMASK_MAX_RT) {
        ret = NRF24_STA_TX_FAIL;
    }

    return ret;
}



/* Configuration:
 *  - PTX mode
 *  - Power down
 *  - channel 2
 *  - 2 bytes crc
 *  - 5 bytes address width
 *  - all pipes feature (auto-ack,..) enabled
 *  - 9 times (max) retransmit && 750us delay
 *  - all irq source enabled
 *  - all features enabled
 * 
 * Note: Gen by utils/regscfg
 */
nrf24_regval_t nrf24_default_regval_list[] = {
    {NRF24_REG_CONFIG, 0x0c},
    {NRF24_REG_EN_AA, 0x3f},
    {NRF24_REG_EN_RXADDR, 0x01},
    {NRF24_REG_SETUP_AW, 0x03},
    {NRF24_REG_SETUP_RETR, 0x29},
    {NRF24_REG_RF_CH, 0x02},
    {NRF24_REG_RF_SETUP, 0x0f},
    {NRF24_REG_RX_ADDR_P2, 0xc3},
    {NRF24_REG_RX_ADDR_P3, 0xc4},
    {NRF24_REG_RX_ADDR_P4, 0xc5},
    {NRF24_REG_RX_ADDR_P5, 0xc6},
    {NRF24_REG_DYNPD, 0x3f},
    {NRF24_REG_FEATURE, 0x07},
};

const int nrf24_default_regval_list_num  = sizeof(nrf24_default_regval_list)/sizeof(nrf24_default_regval_list[0]);

int nrf24_write_reg_list(nrf24_t *nrf24, const nrf24_regval_t *regvals, int num)
{
    int ret = 0;

    for (int i = 0; i < num; i++) {
        ret += nrf24_write_reg(nrf24, regvals[i].reg, regvals[i].val);
    }

    return ret;
}

/**
 * @brief Configure and bring up the NRF24 device.
 *
 * Performs actual communication with the device to configure
 * and bring it into a ready state. 
 *
 * @param nrf24  Pointer to the NRF24 device instance.
 * @param role   Initial role of the device (NRF24_ROLE_PRX or NRF24_ROLE_PTX).
 * @return       0 on success, non-zero on error.
 * 
 * @note It is safe to call this function multiple times,
 */
int nrf24_setup(nrf24_t *nrf24, nrf24_role_enum_t role)
{
    nrf24_user_cfg_t ucfg;
    nrf24_usercfg_init_default(&ucfg);

    return nrf24_setup_full(nrf24, role, &ucfg, nrf24_default_regval_list, sizeof(nrf24_default_regval_list)/sizeof(nrf24_default_regval_list[0])); 
}

int nrf24_setup_full(nrf24_t *nrf24, nrf24_role_enum_t role, const nrf24_user_cfg_t *ucfg, const nrf24_regval_t *regvals, int regvals_num)
{
    int ret = 0;
    LOG_V("enter %s", __func__);
    
    CHECK(nrf24 != 0);

    /* */
    nrf24->role = role;

    /* Check connection */
    ret = nrf24_check_device(nrf24);
    if (ret != 0) {
        LOG_E("check device failed");
        goto __ns_exit;
    }
    LOG_D("check device success");

    /* Do soft reset */
    nrf24_power_down(nrf24);
    nrf24_radio_off(nrf24);
    nrf24_clear_all(nrf24);

    /* Activate RWW */
    // ret += send_cmd_activate(&nrf24->dep);

    /* Do config */
    ret += nrf24_write_reg_list(nrf24, regvals, regvals_num);

    nrf24_usercfg_write_directly(nrf24, ucfg);

    /* Set role */
    reg_modify_bits(&nrf24->dep, NRF24_REG_CONFIG, REG_CONFIG_BITMASK_PRIM_RX, role);

    /* Enable */
    nrf24_power_up(nrf24);
    nrf24_radio_on(nrf24);

__ns_exit:
    if (ret) {
        LOG_E("Device setup [fail]");
    }else {
        LOG_I("Device setup [ok]");
    }

    LOG_V("exit %s [%s]", __func__, R2S(ret));
    return ret;
}


/**
 * @brief Initialize the NRF24 driver instance (host-side only).
 * 
 * Sets up the internal elements, such as binding
 * platform-dependent operations. No communication with the device
 * is performed at this stage.
 * 
 * @param nrf24    Pointer to the NRF24 device instance.
 * @param ops      Pointer to platform-dependent operations.
 * @param ops_ctx  Context for platform operations.
 * @return         0 on success, non-zero on error.
 *
 * @note No actual I/O with the device is performed.
 */
int nrf24_init(nrf24_t *nrf24, nrf24_dep_ops_t *ops, void *ops_ctx)
{ 
    int ret = 0;
    LOG_V("enter %s", __func__);

    CHECK(nrf24 != 0);
    CHECK(ops != 0);

    /* Initialize attributes */
    nrf24->ack_pipe = 0;

    /* Initialize dep */
    nrf24->dep.ops = ops;
    nrf24->dep.ctx = ops_ctx;
    ret = dep_init(&nrf24->dep);
    if (ret != 0) {
        LOG_E("dep init failed");
        return ret;
    }

    LOG_V("exit %s [%s]", __func__, R2S(ret));
    return ret;
}

void nrf24_deinit(nrf24_t *nrf24)
{
    LOG_V("enter %s", __func__);

    if (nrf24 == 0) {
        return;
    }

    if (nrf24->dep.ops == 0) {
        return;
    }

    nrf24_power_down(nrf24);
    nrf24_radio_off(nrf24);

    /* deactivate RWW */
    // send_cmd_activate(&nrf24->dep);

    dep_deinit(&nrf24->dep);

    LOG_D("Device deinit [ok].");

    LOG_V("exit %s", __func__);
}

/**
 * @brief Global initialization of the nRF24L01 driver module.
 *
 * This function performs one-time initialization for the entire NRF24 driver module.
 * It should be called once only
 *
 * @return Always returns 0 (success).
 * 
 * @note This function is currently a no-op
 */
int nrf24_global_init(void)
{
    LOG_I("Version: %s", NRF24L01_VERSION);

#ifdef NRF24L01_Si24R1_DEVICE
    LOG_I("Si24R1 device declared")
#endif

    return 0;
}
