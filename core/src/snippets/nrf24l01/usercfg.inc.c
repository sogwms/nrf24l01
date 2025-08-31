
/**
 * @brief Initialize user config with default values.
 *
 * @param ucfg  Pointer to config struct to initialize.
 */
void nrf24_usercfg_init_default(nrf24_user_cfg_t *ucfg)
{
    clear_object(ucfg, sizeof(*ucfg));

    ucfg->rf_channel = 2;
    ucfg->rf_power = NRF24_RF_POWER_0dBm;
    ucfg->rf_adr = NRF24_ADR_2Mbps;

    /*  */
    ucfg->rxpipes[0].enable = 1;
    // ucfg->rxpipes[1].enable = 0;
    // ucfg->rxpipes[2].enable = 0;
    // ucfg->rxpipes[3].enable = 0;
    // ucfg->rxpipes[4].enable = 0;
    // ucfg->rxpipes[5].enable = 0;

    for (int i = 0; i < 6; i++) {
        ucfg->rxpipes[i].enable_aa = 1;
    }

    /* Address */
    for (int i = 0; i < 5; i++) {
        ucfg->tx_addr[i] = 0xE7;
        ucfg->rxpipes[0].addr[i] = ucfg->tx_addr[i];
        ucfg->rxpipes[1].addr[i] = 0xC2;
    }
    ucfg->rxpipes[2].addr_lsb = 0xC3;
    ucfg->rxpipes[3].addr_lsb = 0xC4;
    ucfg->rxpipes[4].addr_lsb = 0xC5;
    ucfg->rxpipes[5].addr_lsb = 0xC6;
}

/**
 * @brief Reads current user configuration from device.
 *
 * @param nrf24  Pointer to device structure.
 * @param ucfg   Pointer to config struct to be filled.
 *
 * @return 0 on success, non-zero on error.
 * 
 * @note Involves actual I/O; avoid frequent calls.
 */
int nrf24_usercfg_read(nrf24_t *nrf24, nrf24_user_cfg_t *ucfg)
{
    int ret = 0;
    uint8_t enrx;
    uint8_t enaa;
    uint8_t rfch;
    uint8_t rfsetup;

    ret += read_reg(&nrf24->dep, NRF24_REG_EN_RXADDR, &enrx);
    ret += read_reg(&nrf24->dep, NRF24_REG_EN_AA, &enaa);
    ret += read_reg(&nrf24->dep, NRF24_REG_RF_CH, &rfch);
    ret += read_reg(&nrf24->dep, NRF24_REG_RF_SETUP, &rfsetup);

    ucfg->rf_adr = byte_get_bits(rfsetup, REG_RF_SETUP_BITMASK_RF_DR);
    ucfg->rf_power = byte_get_bits(rfsetup, REG_RF_SETUP_BITMASK_RF_PWR);
    ucfg->rf_channel = byte_get_bits(rfch, REG_RF_CH_BITMASK_RF_CH);

    for (int i = 0; i < 6; i++) {
        ucfg->rxpipes[i].enable = enrx & (BITMASK_PIPE_0 << i) ? 1 : 0;
        ucfg->rxpipes[i].enable_aa = enaa & (BITMASK_PIPE_0 << i) ? 1 : 0;
    }

    ret += read_regs(&nrf24->dep, NRF24_REG_TX_ADDR, ASU8P(&ucfg->tx_addr[0]), 5);    
    ret += read_regs(&nrf24->dep, NRF24_REG_RX_ADDR_P0, ASU8P(&ucfg->rxpipes[0].addr[0]), 5);
    ret += read_regs(&nrf24->dep, NRF24_REG_RX_ADDR_P1, ASU8P(&ucfg->rxpipes[1].addr[0]), 5);
    ret += read_reg(&nrf24->dep, NRF24_REG_RX_ADDR_P2, ASU8P(&ucfg->rxpipes[2].addr_lsb));
    ret += read_reg(&nrf24->dep, NRF24_REG_RX_ADDR_P3, ASU8P(&ucfg->rxpipes[3].addr_lsb));
    ret += read_reg(&nrf24->dep, NRF24_REG_RX_ADDR_P4, ASU8P(&ucfg->rxpipes[4].addr_lsb));
    ret += read_reg(&nrf24->dep, NRF24_REG_RX_ADDR_P5, ASU8P(&ucfg->rxpipes[5].addr_lsb));

    return ret;
}

/**
 * @brief Writes user configuration to device.
 *
 * @param nrf24  Pointer to device structure.
 * @param ucfg   Pointer to config struct.
 *
 * @return 0 on success, non-zero on error.
 * 
 * @note Involves actual I/O; avoid frequent calls.
 * @attention Temporarily disables the radio during configuration update.
 */
int nrf24_usercfg_write(nrf24_t *nrf24, const nrf24_user_cfg_t *ucfg)
{
    int ret = 0;

    uint8_t is_radio_on = nrf24->is_radio_on;

    if (is_radio_on) {
        nrf24_radio_off(nrf24);
    }

    ret = nrf24_usercfg_write_directly(nrf24, ucfg);

    if (is_radio_on) {
        nrf24_radio_on(nrf24);
    }

    return ret;
}

/**
 * @brief Writes user configuration directly without any side effects
 */
int nrf24_usercfg_write_directly(nrf24_t *nrf24, const nrf24_user_cfg_t *ucfg)
{
    int ret = 0;
    uint8_t enrx = 0;
    uint8_t enaa = 0;
    uint8_t rfch = 0;
    uint8_t rfsetup = 0;

    /* RF-SETUP REGISTER */
    ret += read_reg(&nrf24->dep, NRF24_REG_RF_SETUP, &rfsetup);
    byte_set_bits(&rfsetup, REG_RF_SETUP_BITMASK_RF_DR, ucfg->rf_adr);
    byte_set_bits(&rfsetup, REG_RF_SETUP_BITMASK_RF_PWR, ucfg->rf_power);
    ret += write_reg(&nrf24->dep, NRF24_REG_RF_SETUP, rfsetup);

    /* RF-CH REGISTER */
    byte_set_bits(&rfch, REG_RF_CH_BITMASK_RF_CH, ucfg->rf_channel);
    ret += write_reg(&nrf24->dep, NRF24_REG_RF_CH, rfch);

    /* EN_RXADDR, EN_AA REGISTER */
    for (int i = 0; i < 6; i++) {
        byte_set_bits(&enrx, (BITMASK_PIPE_0 << i), ucfg->rxpipes[i].enable ? 1 : 0);
        byte_set_bits(&enaa, (BITMASK_PIPE_0 << i), ucfg->rxpipes[i].enable_aa ? 1 : 0);
    }
    ret += write_reg(&nrf24->dep, NRF24_REG_EN_RXADDR, enrx);
    ret += write_reg(&nrf24->dep, NRF24_REG_EN_AA, enaa);

    /* ADDR REGSITERS */
    ret += write_regs(&nrf24->dep, NRF24_REG_TX_ADDR, ASU8P(&ucfg->tx_addr[0]), 5);    
    ret += write_regs(&nrf24->dep, NRF24_REG_RX_ADDR_P0, ASU8P(&ucfg->rxpipes[0].addr[0]), 5);
    ret += write_regs(&nrf24->dep, NRF24_REG_RX_ADDR_P1, ASU8P(&ucfg->rxpipes[1].addr[0]), 5);
    ret += write_reg(&nrf24->dep, NRF24_REG_RX_ADDR_P2, ucfg->rxpipes[2].addr_lsb);
    ret += write_reg(&nrf24->dep, NRF24_REG_RX_ADDR_P3, ucfg->rxpipes[3].addr_lsb);
    ret += write_reg(&nrf24->dep, NRF24_REG_RX_ADDR_P4, ucfg->rxpipes[4].addr_lsb);
    ret += write_reg(&nrf24->dep, NRF24_REG_RX_ADDR_P5, ucfg->rxpipes[5].addr_lsb);

    return ret;
}

int nrf24_usercfg_write_diff_directly(nrf24_t *nrf24, const nrf24_user_cfg_t *old, const nrf24_user_cfg_t *new)
{
    int ret = 0;
    uint8_t enrx = 0;
    uint8_t enaa = 0;
    uint8_t rfch = 0;
    uint8_t rfsetup = 0;
    const nrf24_user_cfg_t *ucfg = new;

    if (old == 0) {
        return nrf24_usercfg_write_directly(nrf24, new);
    }

    /* RF-SETUP REGISTER */
    if (old->rf_adr != new->rf_adr) {
        ret += read_reg(&nrf24->dep, NRF24_REG_RF_SETUP, &rfsetup);
        byte_set_bits(&rfsetup, REG_RF_SETUP_BITMASK_RF_DR, ucfg->rf_adr);
        byte_set_bits(&rfsetup, REG_RF_SETUP_BITMASK_RF_PWR, ucfg->rf_power);
        ret += write_reg(&nrf24->dep, NRF24_REG_RF_SETUP, rfsetup);
    }

    /* RF-CH REGISTER */
    if (old->rf_channel != new->rf_channel) {
        byte_set_bits(&rfch, REG_RF_CH_BITMASK_RF_CH, ucfg->rf_channel);
        ret += write_reg(&nrf24->dep, NRF24_REG_RF_CH, rfch);
    }

    /* EN_RXADDR REGISTER */
    for (int i = 0; i < 6; i++) {
        if (old->rxpipes[i].enable != new->rxpipes[i].enable) {
            for (int i = 0; i < 6; i++) {
                byte_set_bits(&enrx, (BITMASK_PIPE_0 << i), ucfg->rxpipes[i].enable ? 1 : 0);
            }
            ret += write_reg(&nrf24->dep, NRF24_REG_EN_RXADDR, enrx);
            break;
        }
    }

    /* EN_AA REGISTER */
    for (int i = 0; i < 6; i++) {
        if (old->rxpipes[i].enable_aa != new->rxpipes[i].enable_aa) {
            for (int i = 0; i < 6; i++) {
                byte_set_bits(&enaa, (BITMASK_PIPE_0 << i), ucfg->rxpipes[i].enable_aa ? 1 : 0);
            }
            ret += write_reg(&nrf24->dep, NRF24_REG_EN_AA, enaa);
            break;
        }
    }

    /* ADDR REGSITERS */
    if (compare(&old->tx_addr[0], &new->tx_addr[0], 5) != 0) {
        ret += write_regs(&nrf24->dep, NRF24_REG_TX_ADDR, ASU8P(&ucfg->tx_addr[0]), 5);    
    }
    if (compare(&old->rxpipes[0].addr[0], &new->rxpipes[0].addr[0], 5) != 0) {
        ret += write_regs(&nrf24->dep, NRF24_REG_RX_ADDR_P0, ASU8P(&ucfg->rxpipes[0].addr[0]), 5);
    }
    if (compare(&old->rxpipes[1].addr[0], &new->rxpipes[1].addr[0], 5) != 0) {
        ret += write_regs(&nrf24->dep, NRF24_REG_RX_ADDR_P1, ASU8P(&ucfg->rxpipes[1].addr[0]), 5);
    }

    for (int i = 2; i < 6; i++) {
        if (old->rxpipes[i].addr[0] != new->rxpipes[i].addr[0]) {
            ret += write_reg(&nrf24->dep, NRF24_REG_RX_ADDR_P0 + i, ucfg->rxpipes[i].addr_lsb);
        }
    }

    return ret;
}
