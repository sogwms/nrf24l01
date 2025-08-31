#include <stdio.h>
#include <string.h>
#include "./reg.h"

/* helper macro for casting a pointer ...*/
#define AS(t,v) ((t)(v))
#define ASU8P(x) AS(uint8_t *, x)
#define ASU8V(x) (*(ASU8P(x)))
#define PRINT_REG(cfg, reg_name, field) \
    printf("0x%02x, %s, 0x%02x\n", NRF24_REG_##reg_name, "NRF24_REG_"#reg_name, ASU8V(&cfg.field))

void nrf24_regscfg_init_default(nrf24_regs_cfg_t *cfg);

void print_l5arr(uint8_t *arr) 
{
    printf("0x%02x%02x%02x%02x%02x\n", arr[4],arr[3],arr[2],arr[1],arr[0]);
}

int main(void)
{
    nrf24_regs_cfg_t cfg;
    nrf24_regscfg_init_default(&cfg);

    printf("addr, name, val\n");

    PRINT_REG(cfg, CONFIG, config);
    PRINT_REG(cfg, EN_AA, en_aa);
    PRINT_REG(cfg, EN_RXADDR, en_rxaddr);
    PRINT_REG(cfg, SETUP_AW, setup_aw);
    PRINT_REG(cfg, SETUP_RETR, setup_retr);
    PRINT_REG(cfg, RF_CH, rf_ch);
    PRINT_REG(cfg, RF_SETUP, rf_setup);
    
    // PRINT_REG(cfg, RX_ADDR_P0, rx_addr_p0[0]);  //
    // PRINT_REG(cfg, RX_ADDR_P1, rx_addr_p1[0]);
    printf("0x%02x, %s, ", NRF24_REG_RX_ADDR_P0, "NRF24_REG_RX_ADDR_P0");
    print_l5arr(cfg.rx_addr_p0);
    printf("0x%02x, %s, ", NRF24_REG_RX_ADDR_P1, "NRF24_REG_RX_ADDR_P1");
    print_l5arr(cfg.rx_addr_p1);
    PRINT_REG(cfg, RX_ADDR_P2, rx_addr_p2);
    PRINT_REG(cfg, RX_ADDR_P3, rx_addr_p3);
    PRINT_REG(cfg, RX_ADDR_P4, rx_addr_p4);
    PRINT_REG(cfg, RX_ADDR_P5, rx_addr_p5);
    // PRINT_REG(cfg, TX_ADDR, tx_addr[0]);
    printf("0x%02x, %s, ", NRF24_REG_TX_ADDR, "NRF24_REG_TX_ADDR");
    print_l5arr(cfg.tx_addr);
    
    PRINT_REG(cfg, DYNPD, dynpd);
    PRINT_REG(cfg, FEATURE, feature);

    return 0;
}

static void clear_object(void *obj, int size_in_byte)
{
    uint8_t *p = (uint8_t *)obj;
    while (size_in_byte--) *p++ = 0;
}


/** 
 * @brief Set default value
 */
void nrf24_regscfg_init_default(nrf24_regs_cfg_t *cfg)
{
    if (cfg == 0) return;

    clear_object(cfg, sizeof(nrf24_regs_cfg_t));

    /* rx pipe0 enabled, others disabled */
    cfg->en_rxaddr.p0 = 1;

    /* RF Channel: 2 (2402 MHz) */
    cfg->rf_ch.rf_ch = 2;

    /* 2Mbps, 0dBm (the most highest output power) */
    cfg->rf_setup.rf_dr = NRF24_ADR_2Mbps;
    cfg->rf_setup.rf_pwr = NRF24_RF_POWER_0dBm;
    cfg->rf_setup.lna_hcurr = 1; // set LNA High-Current

    /* ARD: 750us, ARC: 9 */
    cfg->setup_retr.ard = 0x2;
    cfg->setup_retr.arc = 9;

    /* Address:
     * - tx-addr, rx_addr_p0: 0xE7E7E7E7E7
     * - rx_addr_p1: 0xC2C2C2C2C2
     * - rx_addr_p2: ----------C3
     * - rx_addr_p3: ----------C4
     * - rx_addr_p4: ----------C5
     * - rx_addr_p5: ----------C6
    */
    cfg->tx_addr[4] = 0xE7;
    cfg->tx_addr[3] = 0xE7;
    cfg->tx_addr[2] = 0xE7;
    cfg->tx_addr[1] = 0xE7;
    cfg->tx_addr[0] = 0xE7;

    cfg->rx_addr_p0[4] = cfg->tx_addr[4];
    cfg->rx_addr_p0[3] = cfg->tx_addr[3];
    cfg->rx_addr_p0[2] = cfg->tx_addr[2];
    cfg->rx_addr_p0[1] = cfg->tx_addr[1];
    cfg->rx_addr_p0[0] = cfg->tx_addr[0];

    cfg->rx_addr_p1[4] = 0xC2;
    cfg->rx_addr_p1[3] = 0xC2;
    cfg->rx_addr_p1[2] = 0xC2;
    cfg->rx_addr_p1[1] = 0xC2;
    cfg->rx_addr_p1[0] = 0xC2;

    cfg->rx_addr_p2 = 0xC3;
    cfg->rx_addr_p3 = 0xC4;
    cfg->rx_addr_p4 = 0xC5;
    cfg->rx_addr_p5 = 0xC6;

    /* all rx pipes auto-ack enabled */
    *ASU8P(&cfg->en_aa) = 0x3F;

    /*********************************/
    /*  Not Required To Change Below Usually */
    /*********************************/
    /* 5-byte address width  */
    cfg->setup_aw.aw = 0x3;

    /* CONFIG Default:
     * - all irq sources enabled (dr,ds,max_rt)
     * - 2-byte CRC
     * - power down
     * - PTX
     */
    cfg->config.en_crc = 1;
    cfg->config.crco = 1;
    cfg->config.prim_rx = 0;

    /* all rx pipes dyn payload length enabled*/
    *ASU8P(&cfg->dynpd) = 0x3F;

    /* all features enabled */
    cfg->feature.en_dpl = 1;        // dynamic payload length
    cfg->feature.en_ack_pay = 1;    // ack with payload
    cfg->feature.en_dyn_ack = 1;    // dynamic ack (which means can control ack existence by tx packet (NRF24_CMD_W_TX_PAYLOAD_NO_ACK or NRF24_CMD_W_TX_PAYLOAD))
}

// /**
//  * @brief Read from chip
//  */
// int nrf24_regscfg_read(nrf24_t *nrf24, nrf24_regs_cfg_t *cfg) 
// {
//     int ret = 0;
//     // Note: the device does not support burst read

//     ret += read_reg(&nrf24->dep, NRF24_REG_CONFIG, ASU8P(&cfg->config));
//     ret += read_reg(&nrf24->dep, NRF24_REG_EN_AA, ASU8P(&cfg->en_aa));
//     ret += read_reg(&nrf24->dep, NRF24_REG_EN_RXADDR, ASU8P(&cfg->en_rxaddr));
//     ret += read_reg(&nrf24->dep, NRF24_REG_SETUP_AW, ASU8P(&cfg->setup_aw));
//     ret += read_reg(&nrf24->dep, NRF24_REG_SETUP_RETR, ASU8P(&cfg->setup_retr));
//     ret += read_reg(&nrf24->dep, NRF24_REG_RF_CH, ASU8P(&cfg->rf_ch));
//     ret += read_reg(&nrf24->dep, NRF24_REG_RF_SETUP, ASU8P(&cfg->rf_setup));
    
//     ret += read_regs(&nrf24->dep, NRF24_REG_TX_ADDR, ASU8P(&cfg->tx_addr[0]), 5);    
//     ret += read_regs(&nrf24->dep, NRF24_REG_RX_ADDR_P0, ASU8P(&cfg->rx_addr_p0[0]), 5);
//     ret += read_regs(&nrf24->dep, NRF24_REG_RX_ADDR_P1, ASU8P(&cfg->rx_addr_p1[0]), 5);
//     ret += read_reg(&nrf24->dep, NRF24_REG_RX_ADDR_P2, ASU8P(&cfg->rx_addr_p2));
//     ret += read_reg(&nrf24->dep, NRF24_REG_RX_ADDR_P3, ASU8P(&cfg->rx_addr_p3));
//     ret += read_reg(&nrf24->dep, NRF24_REG_RX_ADDR_P4, ASU8P(&cfg->rx_addr_p4));
//     ret += read_reg(&nrf24->dep, NRF24_REG_RX_ADDR_P5, ASU8P(&cfg->rx_addr_p5));

//     ret += read_reg(&nrf24->dep, NRF24_REG_DYNPD, ASU8P(&cfg->dynpd));
//     ret += read_reg(&nrf24->dep, NRF24_REG_FEATURE, ASU8P(&cfg->feature));

//     return ret;
// }

// /**
//  * @brief Write to chip
//  */
// int nrf24_regscfg_write(nrf24_t *nrf24, nrf24_regs_cfg_t *cfg)
// {
//     int ret = 0;

//     ret += reg_sync_val(&nrf24->dep, NRF24_REG_CONFIG, ASU8V(&cfg->config));
//     ret += reg_sync_val(&nrf24->dep, NRF24_REG_EN_AA, ASU8V(&cfg->en_aa));
//     ret += reg_sync_val(&nrf24->dep, NRF24_REG_EN_RXADDR, ASU8V(&cfg->en_rxaddr));
//     ret += reg_sync_val(&nrf24->dep, NRF24_REG_SETUP_AW, ASU8V(&cfg->setup_aw));
//     ret += reg_sync_val(&nrf24->dep, NRF24_REG_SETUP_RETR, ASU8V(&cfg->setup_retr));
//     ret += reg_sync_val(&nrf24->dep, NRF24_REG_RF_CH, ASU8V(&cfg->rf_ch));
//     ret += reg_sync_val(&nrf24->dep, NRF24_REG_RF_SETUP, ASU8V(&cfg->rf_setup));

//     ret += write_regs(&nrf24->dep, NRF24_REG_TX_ADDR, ASU8P(&cfg->tx_addr[0]), 5);    
//     ret += write_regs(&nrf24->dep, NRF24_REG_RX_ADDR_P0, ASU8P(&cfg->rx_addr_p0[0]), 5);
//     ret += write_regs(&nrf24->dep, NRF24_REG_RX_ADDR_P1, ASU8P(&cfg->rx_addr_p1[0]), 5);
//     ret += reg_sync_val(&nrf24->dep, NRF24_REG_RX_ADDR_P2, cfg->rx_addr_p2);
//     ret += reg_sync_val(&nrf24->dep, NRF24_REG_RX_ADDR_P3, cfg->rx_addr_p3);
//     ret += reg_sync_val(&nrf24->dep, NRF24_REG_RX_ADDR_P4, cfg->rx_addr_p4);
//     ret += reg_sync_val(&nrf24->dep, NRF24_REG_RX_ADDR_P5, cfg->rx_addr_p5);

//     ret += reg_sync_val(&nrf24->dep, NRF24_REG_DYNPD, ASU8V(&cfg->dynpd));
//     ret += reg_sync_val(&nrf24->dep, NRF24_REG_FEATURE, ASU8V(&cfg->feature));

//     return ret;
// }

// /** 
//  * @brief "write" usercfg
//  */
// void nrf24_regscfg_apply_usercfg(nrf24_regs_cfg_t *rcfg, const nrf24_user_cfg_t *ucfg)
// {
//     rcfg->rf_setup.rf_dr = ucfg->adr;
//     rcfg->rf_setup.rf_pwr = ucfg->rf_power;
//     rcfg->config.prim_rx = ucfg->role;

//     copy(rcfg->tx_addr, ucfg->tx_addr, sizeof(rcfg->tx_addr));
//     copy(rcfg->rx_addr_p0, ucfg->rxpipe0.addr, sizeof(rcfg->rx_addr_p0));
//     copy(rcfg->rx_addr_p1, ucfg->rxpipe1.addr, sizeof(rcfg->rx_addr_p1));
//     rcfg->rx_addr_p2 = ucfg->rxpipe2.addr;
//     rcfg->rx_addr_p3 = ucfg->rxpipe3.addr;
//     rcfg->rx_addr_p4 = ucfg->rxpipe4.addr;
//     rcfg->rx_addr_p5 = ucfg->rxpipe5.addr;

//     rcfg->en_rxaddr.p0 = ucfg->rxpipe0.enable;
//     rcfg->en_rxaddr.p1 = ucfg->rxpipe1.enable;
//     rcfg->en_rxaddr.p2 = ucfg->rxpipe2.enable;
//     rcfg->en_rxaddr.p3 = ucfg->rxpipe3.enable;
//     rcfg->en_rxaddr.p4 = ucfg->rxpipe4.enable;
//     rcfg->en_rxaddr.p5 = ucfg->rxpipe5.enable;
// }

// /** 
//  * @brief "read" usercfg
//  */
// void nrf24_regscfg_extract_usercfg(nrf24_regs_cfg_t *rcfg, nrf24_user_cfg_t *ucfg)
// {
//     ucfg->adr = rcfg->rf_setup.rf_dr;
//     ucfg->rf_power = rcfg->rf_setup.rf_pwr;
//     ucfg->role = rcfg->config.prim_rx;

//     copy(ucfg->tx_addr,rcfg->tx_addr, sizeof(rcfg->tx_addr));
//     copy(ucfg->rxpipe0.addr,rcfg->rx_addr_p0, sizeof(rcfg->rx_addr_p0));
//     copy(ucfg->rxpipe1.addr,rcfg->rx_addr_p1, sizeof(rcfg->rx_addr_p1));
//     ucfg->rxpipe2.addr = rcfg->rx_addr_p2;
//     ucfg->rxpipe3.addr = rcfg->rx_addr_p3;
//     ucfg->rxpipe4.addr = rcfg->rx_addr_p4;
//     ucfg->rxpipe5.addr = rcfg->rx_addr_p5;

//     ucfg->rxpipe0.enable = rcfg->en_rxaddr.p0;
//     ucfg->rxpipe1.enable = rcfg->en_rxaddr.p1;
//     ucfg->rxpipe2.enable = rcfg->en_rxaddr.p2;
//     ucfg->rxpipe3.enable = rcfg->en_rxaddr.p3;
//     ucfg->rxpipe4.enable = rcfg->en_rxaddr.p4;
//     ucfg->rxpipe5.enable = rcfg->en_rxaddr.p5;
// }

