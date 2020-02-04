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

// note: 地址宽度:5

/* Includes --------------------------------------------------------------------------------*/
#include <rtthread.h>
#include "nrf24l01.h"

#define DBG_SECTION_NAME  "nrf24l01"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

#ifdef PKG_NRF24L01_ENABLING_DEBUG
#ifdef PKG_NRF24L01_USING_INFO_REPORT
    #define NRF24_USING_INFO_REPORT
#endif
#ifdef PKG_NRF24L01_USING_SHELL_CMD
    #define NRF24_USING_SHELL_CMD
#endif
#endif

///<命令映射  
#define NRF24CMD_R_REG          0x00  // 读寄存器
#define NRF24CMD_W_REG          0x20  // 写寄存器
#define NRF24CMD_R_RX_PAYLOAD   0x61  // 读接收缓冲区
#define NRF24CMD_W_TX_PAYLOAD   0xA0  // 写发送缓冲区
#define NRF24CMD_FLUSH_TX       0xE1  // 清空发送FIFO
#define NRF24CMD_FLUSH_RX       0xE2  // 清空接收FIFO
#define NRF24CMD_REUSE_TX_PL    0xE3  // PTX模式下使用，重装载发送缓冲区
#define NRF24CMD_ACTIVATE       0x50  // 使能命令，后接数据 0x73
#define NRF24CMD_R_RX_PL_WID    0x60  // 读顶层接收FIFO大小
#define NRF24CMD_W_ACK_PAYLOAD  0xA8  // RX模式下使用，写应答发送缓冲区
///<寄存器映射
#define NRF24REG_CONFIG         0x00  // 配置收发状态，CRC校验模式以及收发状态响应方式
#define NRF24REG_EN_AA          0x01  // 自动应答功能设置
#define NRF24REG_EN_RXADDR      0x02  // 可用信道设置
#define NRF24REG_SETUP_AW       0x03  // 收发地址宽度设置
#define NRF24REG_SETUP_RETR     0x04  // 自动重发功能设置
#define NRF24REG_RF_CH          0x05  // 工作频率设置
#define NRF24REG_RF_SETUP       0x06  // 发射速率、功耗功能设置
#define NRF24REG_STATUS         0x07  // 状态寄存器
#define NRF24REG_OBSERVE_TX     0x08  // 发送监测功能
#define NRF24REG_RPD            0x09  // 接收功率检测           
#define NRF24REG_RX_ADDR_P0     0x0A  // 频道0接收数据地址
#define NRF24REG_RX_ADDR_P1     0x0B  // 频道1接收数据地址
#define NRF24REG_RX_ADDR_P2     0x0C  // 频道2接收数据地址
#define NRF24REG_RX_ADDR_P3     0x0D  // 频道3接收数据地址
#define NRF24REG_RX_ADDR_P4     0x0E  // 频道4接收数据地址
#define NRF24REG_RX_ADDR_P5     0x0F  // 频道5接收数据地址
#define NRF24REG_TX_ADDR        0x10  // 发送地址寄存器
#define NRF24REG_RX_PW_P0       0x11  // 接收频道0接收数据长度
#define NRF24REG_RX_PW_P1       0x12  // 接收频道1接收数据长度
#define NRF24REG_RX_PW_P2       0x13  // 接收频道2接收数据长度
#define NRF24REG_RX_PW_P3       0x14  // 接收频道3接收数据长度
#define NRF24REG_RX_PW_P4       0x15  // 接收频道4接收数据长度
#define NRF24REG_RX_PW_P5       0x16  // 接收频道5接收数据长度
#define NRF24REG_FIFO_STATUS    0x17  // FIFO栈入栈出状态寄存器设置
#define NRF24REG_DYNPD          0x1C  // 动态数据包长度
#define NRF24REG_FEATURE        0x1D  // 特点寄存器  
///<寄存器功能位掩码部分映射
//CONFIG
#define NRF24BITMASK_RX_DR      ((uint8_t)(1<<6))  // 接收完成中断使能位
#define NRF24BITMASK_TX_DS      ((uint8_t)(1<<5))  // 发送完成中断使能位
#define NRF24BITMASK_MAX_RT     ((uint8_t)(1<<4))  // 达最大重发次数中断使能位
#define NRF24BITMASK_EN_CRC     ((uint8_t)(1<<3))  // CRC使能位
#define NRF24BITMASK_CRCO       ((uint8_t)(1<<2))  // CRC编码方式 （1B or 2B）
#define NRF24BITMASK_PWR_UP     ((uint8_t)(1<<1))  // 上（掉）电
#define NRF24BITMASK_PRIM_RX    ((uint8_t)(1))     // PR（T）X
//SETUP_AW
#define NRF24BITMASK_AW         ((uint8_t)(0x03))  // RX/TX地址宽度
//SETUP_RETR
#define NRF24BITMASK_ARD        ((uint8_t)(0xF0))  // 重发延时
#define NRF24BITMASK_ARC        ((uint8_t)(0x0F))  // 重发最大次数
//RF_CH
#define NRF24BITMASK_RF_CH      ((uint8_t)(0x7F))  // 射频频道
//RF_SETUP
#define NRF24BITMASK_RF_DR      ((uint8_t)(1<<3))  // 空中速率
#define NRF24BITMASK_RF_PWR     ((uint8_t)(0x06))  // 发射功率
//STATUS
#define NRF24BITMASK_RX_DR      ((uint8_t)(1<<6))  // 接收完成标志位
#define NRF24BITMASK_TX_DS      ((uint8_t)(1<<5))  // 发送完成标志位
#define NRF24BITMASK_MAX_RT     ((uint8_t)(1<<4))  // 最大重发次数标志位
#define NRF24BITMASK_RX_P_NO    ((uint8_t)(0x0E))  // RX_FIFO状态标志区位
#define NRF24BITMASK_TX_FULL    ((uint8_t)(1))     // TX_FIFO满标志位
//OBSERVE_TX
#define NRF24BITMASK_PLOS_CNT   ((uint8_t)(0xF0))  // 丢包计数
#define NRF24BITMASK_ARC_CNT    ((uint8_t)(0x0F))  // 重发计数
//CD
#define NRF24BITMASK_CD         ((uint8_t)(1))     // 载波检测标志位
//通用掩码，RX_PW_P[0::5] 掩码相同
#define NRF24BITMASK_RX_PW_P_   ((uint8_t)(0x3F))  // 数据管道RX-Payload中的字节数
//FIFO_STATUS
#define NRF24BITMASK_TX_REUSE   ((uint8_t)(1<<6))  // 
#define NRF24BITMASK_TX_FULL2    ((uint8_t)(1<<5)) // 
#define NRF24BITMASK_TX_EMPTY   ((uint8_t)(1<<4))  // 
#define NRF24BITMASK_RX_RXFULL  ((uint8_t)(1<<1))  // 
#define NRF24BITMASK_RX_EMPTY   ((uint8_t)(1))     // 
//FEATURE
#define NRF24BITMASK_EN_DPL     ((uint8_t)(1<<2))  // 动态长度使能位
#define NRF24BITMASK_EN_ACK_PAY ((uint8_t)(1<<1))  // Payload with ACK 使能位
#define NRF24BITMASK_EN_DYN_ACK ((uint8_t)(1))     // W_TX_PAYLOAD_NOACK 命令使能位
//通用掩码，适用于多个寄存器： EN_AA, EN_RXADDR, DYNPD
#define NRF24BITMASK_PIPE_0     ((uint8_t)(1))     // 
#define NRF24BITMASK_PIPE_1     ((uint8_t)(1<<1))  // 
#define NRF24BITMASK_PIPE_2     ((uint8_t)(1<<2))  // 
#define NRF24BITMASK_PIPE_3     ((uint8_t)(1<<3))  // 
#define NRF24BITMASK_PIPE_4     ((uint8_t)(1<<4))  // 
#define NRF24BITMASK_PIPE_5     ((uint8_t)(1<<5))  // 

struct nrf24_onchip_cfg
{
    struct {
        uint8_t prim_rx     :1;
        uint8_t pwr_up      :1;
        uint8_t crco        :1;
        uint8_t en_crc      :1;
        uint8_t mask_max_rt :1;
        uint8_t mask_tx_ds  :1;
        uint8_t mask_rx_dr  :1;
    } config;

    struct {
        uint8_t p0          :1;
        uint8_t p1          :1;
        uint8_t p2          :1;
        uint8_t p3          :1;
        uint8_t p4          :1;
        uint8_t p5          :1;
    } en_aa;

    struct {
        uint8_t p0          :1;
        uint8_t p1          :1;
        uint8_t p2          :1;
        uint8_t p3          :1;
        uint8_t p4          :1;
        uint8_t p5          :1;
    } en_rxaddr;

    struct {
        uint8_t aw          :2;
    } setup_aw;

    struct {
        uint8_t arc         :4;
        uint8_t ard         :4;
    } setup_retr;

    struct {
        uint8_t rf_ch       :7;
    } rf_ch;

    struct {
        uint8_t lna_hcurr   :1;
        uint8_t rf_pwr      :2;
        uint8_t rf_dr       :1;
        uint8_t pll_lock    :1;
    } rf_setup;

    struct {
        uint8_t p0          :1;
        uint8_t p1          :1;
        uint8_t p2          :1;
        uint8_t p3          :1;
        uint8_t p4          :1;
        uint8_t p5          :1;
    } dynpd;

    struct {
        uint8_t en_dyn_ack  :1;
        uint8_t en_ack_pay  :1;
        uint8_t en_dpl      :1;
    } feature;

    uint8_t rx_addr_p0[5];
    uint8_t rx_addr_p1[5];
    uint8_t rx_addr_p2;
    uint8_t rx_addr_p3;
    uint8_t rx_addr_p4;
    uint8_t rx_addr_p5;

    uint8_t tx_addr[5];

}ALIGN(1);

#ifdef NRF24_USING_SHELL_CMD
    nrf24_t g_debug_nrf24 = RT_NULL;
#endif

/** S PORT-ORIENTED */
/** |||||| **/

static uint8_t __read_reg(nrf24_t nrf24, uint8_t reg)
{
    uint8_t tmp, rtmp = 0;

    tmp = NRF24CMD_R_REG | reg;
    NRF24_HALPORT_SEND_THEN_RECV(&tmp, 1, &rtmp, 1);
    return rtmp;
}

static void __write_reg(nrf24_t nrf24, uint8_t reg, uint8_t data)
{
    uint8_t tmp[2];

    tmp[0] = NRF24CMD_W_REG | reg;
    tmp[1] = data;
    NRF24_HALPORT_WRITE(&tmp[0], 2);
}

// /**
//  * @brief Set bit(s) to 1 based on bit-mask
//  * @param[in] mask: bit mask.   e.g 0x81 stands for bit7 and bit0
//  */
// static void __set_reg_bits(nrf24_t nrf24, uint8_t reg, uint8_t mask)
// {
//     uint8_t tmp;

//     tmp = __read_reg(nrf24, reg);
//     tmp |= mask;
//     __write_reg(nrf24, reg, tmp);
// }

// static void __reset_reg_bits(nrf24_t nrf24, uint8_t reg, uint8_t mask)
// {
//     uint8_t tmp;

//     tmp = __read_reg(nrf24, reg);
//     tmp &= ~mask;
//     __write_reg(nrf24, reg, tmp);
// }

/**
 * @brief Treat the specified continuous bit as a whole and then set its value
 */
static void __write_reg_bits(nrf24_t nrf24, uint8_t reg, uint8_t mask, uint8_t value)
{
    uint8_t tmp, tidx;

    for (tidx = 0; tidx < 8; tidx++)
    {
        if (mask & (1 << tidx))
            break;
    }
    tmp = ~mask & __read_reg(nrf24, reg);
    tmp |= mask & (value << tidx);
    __write_reg(nrf24, reg, tmp);
}

/** E PORT-ORIENTED*/

/** S SCALPEL */
/** |||||| **/

static uint8_t read_status(nrf24_t nrf24)
{
    return __read_reg(nrf24, NRF24REG_STATUS);
}

// bit: RX_DR, TX_DS, MAX_RT
static void clear_status(nrf24_t nrf24, uint8_t bitmask)
{
    __write_reg(nrf24, NRF24REG_STATUS, bitmask);
}

static void clear_observe_tx(nrf24_t nrf24)
{
    __write_reg(nrf24, NRF24REG_OBSERVE_TX, 0);
}

static uint8_t read_top_rxfifo_width(nrf24_t nrf24)
{
    uint8_t tmp = NRF24CMD_R_RX_PL_WID;

    NRF24_HALPORT_SEND_THEN_RECV(&tmp, 1, &tmp, 1);
    return tmp;
}

void nrf24_enter_power_down_mode(nrf24_t nrf24)
{
    __write_reg_bits(nrf24, NRF24REG_CONFIG, NRF24BITMASK_PWR_UP, 0);
}

void nrf24_enter_power_up_mode(nrf24_t nrf24)
{
    __write_reg_bits(nrf24, NRF24REG_CONFIG, NRF24BITMASK_PWR_UP, 1);
}

// static void enabled_irq(nrf24_t nrf24, uint8_t bitmask)
// {
//     if (!((bitmask == NRF24BITMASK_RX_DR) || (bitmask == NRF24BITMASK_TX_DS) || (bitmask == NRF24BITMASK_MAX_RT)))
//         return;

//     __reset_reg_bits(nrf24, NRF24REG_CONFIG, bitmask);
// }

// static void disable_irq(nrf24_t nrf24, uint8_t bitmask)
// {
//     if (!((bitmask == NRF24BITMASK_RX_DR) || (bitmask == NRF24BITMASK_TX_DS) || (bitmask == NRF24BITMASK_MAX_RT)))
//         return;

//     __set_reg_bits(nrf24, NRF24REG_CONFIG, bitmask);
// }


static void write_tx_payload(nrf24_t nrf24, const uint8_t *buf, uint8_t len)
{
    uint8_t tmp = NRF24CMD_W_TX_PAYLOAD;

    NRF24_HALPORT_SEND_THEN_SEND(&tmp, 1, buf, len);
}

static void write_ack_payload(nrf24_t nrf24, uint8_t pipe, const uint8_t *buf, uint8_t len)
{
    uint8_t tmp;

    if (pipe > 5)
        return;

    tmp = NRF24CMD_W_ACK_PAYLOAD | pipe;
    NRF24_HALPORT_SEND_THEN_SEND(&tmp, 1, buf, len);
}

// static int read_rx_payload_width(nrf24_t nrf24, uint8_t pipe)
// {
//     if (pipe > 5)
//         return 0;

//     uint8_t len = __read_reg(nrf24, NRF24REG_RX_PW_P0+pipe);

//     return len;
// }

static void read_rx_payload(nrf24_t nrf24, uint8_t *buf, uint8_t len)
{
    uint8_t tcmd;

    if ((len > 32) || (len == 0))
        return;

    tcmd = NRF24CMD_R_RX_PAYLOAD;
    NRF24_HALPORT_SEND_THEN_RECV(&tcmd, 1, buf, len);
}

static void flush_tx_fifo(nrf24_t nrf24)
{
    uint8_t tmp = NRF24CMD_FLUSH_TX;

    NRF24_HALPORT_WRITE(&tmp, 1);
}

static void flush_rx_fifo(nrf24_t nrf24)
{
    uint8_t tmp = NRF24CMD_FLUSH_RX;

    NRF24_HALPORT_WRITE(&tmp, 1);
}

/** E SCALPEL */

static void ensure_rww_features_activated(nrf24_t nrf24)
{
    if (!nrf24->flags.activated_features)
    {
        uint8_t tmp[2] = {NRF24CMD_ACTIVATE, 0x73};
        NRF24_HALPORT_WRITE(tmp, 2);
        nrf24->flags.activated_features = RT_TRUE;
    }
}

static int update_onchip_config(nrf24_t nrf24, const struct nrf24_onchip_cfg *ccfg)
{
    uint8_t tmp;

    nrf24_enter_power_down_mode(nrf24);
    ensure_rww_features_activated(nrf24);

    __write_reg(nrf24, NRF24REG_EN_AA,      *((uint8_t *)&ccfg->en_aa));
    __write_reg(nrf24, NRF24REG_EN_RXADDR,  *((uint8_t *)&ccfg->en_rxaddr));
    __write_reg(nrf24, NRF24REG_SETUP_AW,   *((uint8_t *)&ccfg->setup_aw));
    __write_reg(nrf24, NRF24REG_SETUP_RETR, *((uint8_t *)&ccfg->setup_retr));
    __write_reg(nrf24, NRF24REG_RF_CH,      *((uint8_t *)&ccfg->rf_ch));
    __write_reg(nrf24, NRF24REG_RF_SETUP,   *((uint8_t *)&ccfg->rf_setup));
    __write_reg(nrf24, NRF24REG_DYNPD,      *((uint8_t *)&ccfg->dynpd));
    __write_reg(nrf24, NRF24REG_FEATURE,    *((uint8_t *)&ccfg->feature));

    tmp = NRF24CMD_W_REG | NRF24REG_TX_ADDR;
    NRF24_HALPORT_SEND_THEN_SEND(&tmp, 1, ccfg->tx_addr, 5);
    tmp = NRF24CMD_W_REG | NRF24REG_RX_ADDR_P0;
    NRF24_HALPORT_SEND_THEN_SEND(&tmp, 1, ccfg->rx_addr_p0, 5);
    tmp = NRF24CMD_W_REG | NRF24REG_RX_ADDR_P1;
    NRF24_HALPORT_SEND_THEN_SEND(&tmp, 1, ccfg->rx_addr_p1, 5);
    tmp = NRF24CMD_W_REG | NRF24REG_RX_ADDR_P2;
    NRF24_HALPORT_SEND_THEN_SEND(&tmp, 1, &ccfg->rx_addr_p2, 1);
    tmp = NRF24CMD_W_REG | NRF24REG_RX_ADDR_P3;
    NRF24_HALPORT_SEND_THEN_SEND(&tmp, 1, &ccfg->rx_addr_p3, 1);
    tmp = NRF24CMD_W_REG | NRF24REG_RX_ADDR_P4;
    NRF24_HALPORT_SEND_THEN_SEND(&tmp, 1, &ccfg->rx_addr_p4, 1);
    tmp = NRF24CMD_W_REG | NRF24REG_RX_ADDR_P5;
    NRF24_HALPORT_SEND_THEN_SEND(&tmp, 1, &ccfg->rx_addr_p5, 1);

    __write_reg(nrf24, NRF24REG_CONFIG,     *((uint8_t *)&ccfg->config));

    return RT_EOK;
}

static int read_onchip_config(nrf24_t nrf24, struct nrf24_onchip_cfg *ccfg)
{
    struct nrf24_onchip_cfg real_cfg;
    uint8_t tmp;

    *((uint8_t *)&real_cfg.en_aa)      =  __read_reg(nrf24, NRF24REG_EN_AA);
    *((uint8_t *)&real_cfg.en_rxaddr)  =  __read_reg(nrf24, NRF24REG_EN_RXADDR);
    *((uint8_t *)&real_cfg.setup_aw)   =  __read_reg(nrf24, NRF24REG_SETUP_AW);
    *((uint8_t *)&real_cfg.setup_retr) =  __read_reg(nrf24, NRF24REG_SETUP_RETR);
    *((uint8_t *)&real_cfg.rf_ch)      =  __read_reg(nrf24, NRF24REG_RF_CH);
    *((uint8_t *)&real_cfg.rf_setup)   =  __read_reg(nrf24, NRF24REG_RF_SETUP);
    *((uint8_t *)&real_cfg.dynpd)      =  __read_reg(nrf24, NRF24REG_DYNPD);
    *((uint8_t *)&real_cfg.feature)    =  __read_reg(nrf24, NRF24REG_FEATURE);
    *((uint8_t *)&real_cfg.config)     =  __read_reg(nrf24, NRF24REG_CONFIG);

    tmp = NRF24CMD_R_REG | NRF24REG_TX_ADDR;
    NRF24_HALPORT_SEND_THEN_RECV(&tmp, 1, (uint8_t *)&real_cfg.tx_addr, 5);
    tmp = NRF24CMD_R_REG | NRF24REG_RX_ADDR_P0;
    NRF24_HALPORT_SEND_THEN_RECV(&tmp, 1, (uint8_t *)&real_cfg.rx_addr_p0, 5);
    tmp = NRF24CMD_R_REG | NRF24REG_RX_ADDR_P1;
    NRF24_HALPORT_SEND_THEN_RECV(&tmp, 1, (uint8_t *)&real_cfg.rx_addr_p1, 5);
    tmp = NRF24CMD_R_REG | NRF24REG_RX_ADDR_P2;
    NRF24_HALPORT_SEND_THEN_RECV(&tmp, 1, (uint8_t *)&real_cfg.rx_addr_p2, 1);
    tmp = NRF24CMD_R_REG | NRF24REG_RX_ADDR_P3;
    NRF24_HALPORT_SEND_THEN_RECV(&tmp, 1, (uint8_t *)&real_cfg.rx_addr_p3, 1);
    tmp = NRF24CMD_R_REG | NRF24REG_RX_ADDR_P4;
    NRF24_HALPORT_SEND_THEN_RECV(&tmp, 1, (uint8_t *)&real_cfg.rx_addr_p4, 1);
    tmp = NRF24CMD_R_REG | NRF24REG_RX_ADDR_P5;
    NRF24_HALPORT_SEND_THEN_RECV(&tmp, 1, (uint8_t *)&real_cfg.rx_addr_p5, 1);

    rt_memcpy(ccfg, &real_cfg, sizeof(struct nrf24_onchip_cfg));

    return RT_EOK;
}

static int check_onchip_config(nrf24_t nrf24, const struct nrf24_onchip_cfg *ccfg)
{
    struct nrf24_onchip_cfg real_cfg;

    read_onchip_config(nrf24, &real_cfg);

    if (rt_memcmp(&real_cfg, ccfg, sizeof(struct nrf24_onchip_cfg)) == 0)
        return RT_EOK;
    else
        return RT_ERROR;
}

static int build_onchip_config(struct nrf24_onchip_cfg *ccfg, const struct nrf24_cfg *ucfg)
{
    rt_memset(ccfg, 0, sizeof(struct nrf24_onchip_cfg));

    /* Default config */
    ccfg->setup_retr.ard = 1;   // 500us
    ccfg->setup_retr.arc = 11;  // 11 times
    ccfg->setup_aw.aw = 3;      // 5-byte address width

    ccfg->rf_setup.pll_lock = 0;
    ccfg->rf_setup.lna_hcurr = 1;

    ccfg->en_aa.p0 = 1;
    ccfg->en_aa.p1 = 1;
    ccfg->en_aa.p2 = 1;
    ccfg->en_aa.p3 = 1;
    ccfg->en_aa.p4 = 1;
    ccfg->en_aa.p5 = 1;

    ccfg->dynpd.p0 = 1;
    ccfg->dynpd.p1 = 1;
    ccfg->dynpd.p2 = 1;
    ccfg->dynpd.p3 = 1;
    ccfg->dynpd.p4 = 1;
    ccfg->dynpd.p5 = 1;

    ccfg->feature.en_dyn_ack = 1;
    ccfg->feature.en_ack_pay = 1;
    ccfg->feature.en_dpl = 1;
    /* END Default config*/

    /**/
    if (ucfg->_irq_pin == NRF24_PIN_NONE)
    {
        ccfg->config.mask_rx_dr = 1;
        ccfg->config.mask_tx_ds = 1;
        ccfg->config.mask_max_rt = 1;
    }
    
    ccfg->config.pwr_up = 1;
    ccfg->config.prim_rx = ucfg->role;
    ccfg->config.en_crc = 1;
    ccfg->config.crco = ucfg->crc;

    ccfg->rf_setup.rf_pwr = ucfg->power;
    ccfg->rf_setup.rf_dr = ucfg->adr;
    ccfg->rf_ch.rf_ch = ucfg->channel;

    rt_memcpy(ccfg->tx_addr, ucfg->txaddr, sizeof(ucfg->txaddr));

    ccfg->en_rxaddr.p0 = ucfg->rxpipe0.bl_enabled;
    ccfg->en_rxaddr.p1 = ucfg->rxpipe1.bl_enabled;
    ccfg->en_rxaddr.p2 = ucfg->rxpipe2.bl_enabled;
    ccfg->en_rxaddr.p3 = ucfg->rxpipe3.bl_enabled;
    ccfg->en_rxaddr.p4 = ucfg->rxpipe4.bl_enabled;
    ccfg->en_rxaddr.p5 = ucfg->rxpipe5.bl_enabled;

    rt_memcpy(ccfg->rx_addr_p0, ucfg->rxpipe0.addr, sizeof(ucfg->rxpipe0.addr));
    rt_memcpy(ccfg->rx_addr_p1, ucfg->rxpipe1.addr, sizeof(ucfg->rxpipe1.addr));
    ccfg->rx_addr_p2 = ucfg->rxpipe2.addr;
    ccfg->rx_addr_p3 = ucfg->rxpipe3.addr;
    ccfg->rx_addr_p4 = ucfg->rxpipe4.addr;
    ccfg->rx_addr_p5 = ucfg->rxpipe5.addr;

    return RT_EOK;
}

/**
 * Test the connection with NRF24
 */
static int check_halport(hal_nrf24_port_t halport)
{
    uint8_t addr[5] = {1,2,3,4,5}, backup_addr[5];
    uint8_t tmp;

    RT_ASSERT(halport != RT_NULL);
    RT_ASSERT(halport->ops != RT_NULL);

    tmp = NRF24CMD_R_REG | NRF24REG_RX_ADDR_P1;
    halport->ops->send_then_recv(halport, &tmp, 1, backup_addr, 5);

    tmp = NRF24CMD_W_REG | NRF24REG_RX_ADDR_P1;
    halport->ops->send_then_send(halport, &tmp, 1, addr, 5);

    rt_memset(addr, 0, 5);

    tmp = NRF24CMD_R_REG | NRF24REG_RX_ADDR_P1;
    halport->ops->send_then_recv(halport, &tmp, 1, addr, 5);

    for (int i = 0; i < 5; i++)
    {
        if (addr[i] != i+1)
            return RT_ERROR;
    }

    tmp = NRF24CMD_W_REG | NRF24REG_RX_ADDR_P1;
    halport->ops->send_then_send(halport, &tmp, 1, backup_addr, 5);

    return RT_EOK;
}

/**
 * Set the user-oriented configuration as the default
 */
int nrf24_fill_default_config_on(nrf24_cfg_t cfg)
{
    RT_ASSERT(cfg != RT_NULL);

    rt_memset(cfg, 0, sizeof(struct nrf24_cfg));

    cfg->power = RF_POWER_0dBm;
    cfg->crc = CRC_2_BYTE;
    cfg->adr = ADR_2Mbps;
    cfg->channel = 100;
    cfg->role = ROLE_NONE;

    for (int i = 0; i < 5; i++)
    {
        cfg->txaddr[i] = i;
        cfg->rxpipe0.addr[i] = i;
        cfg->rxpipe1.addr[i] = i+1;
    }
    cfg->rxpipe2.addr = 2;
    cfg->rxpipe3.addr = 3;
    cfg->rxpipe4.addr = 4;
    cfg->rxpipe5.addr = 5;

    cfg->rxpipe0.bl_enabled = RT_TRUE;
    cfg->rxpipe1.bl_enabled = RT_TRUE;

    cfg->rxpipe2.bl_enabled = RT_FALSE;
    cfg->rxpipe3.bl_enabled = RT_FALSE;
    cfg->rxpipe4.bl_enabled = RT_FALSE;
    cfg->rxpipe5.bl_enabled = RT_FALSE;

    return RT_EOK;
}

int nrf24_send_data(nrf24_t nrf24, uint8_t *data, uint8_t len, uint8_t pipe)
{
    if (len > 32)
        return RT_ERROR;

    if (nrf24->cfg.role == ROLE_PTX)
    {
        write_tx_payload(nrf24, data, len);
    }
    else
    {
        write_ack_payload(nrf24, pipe, data, len);
        rt_sem_release(nrf24->send_sem);
    }
    
    return RT_EOK;
}

void __irq_handler(hal_nrf24_port_t halport)
{
    nrf24_t nrf24 = (nrf24_t)halport;

    rt_sem_release(nrf24->sem);
}

/**
 * ? if try to create sem with the existing name, what will happen
 */
int nrf24_init(nrf24_t nrf24, char *spi_dev_name, int ce_pin, int irq_pin, const struct nrf24_callback *cb, const nrf24_cfg_t cfg)
{
    struct nrf24_onchip_cfg onchip_cfg;

    RT_ASSERT(nrf24 != RT_NULL);
    RT_ASSERT(cfg != RT_NULL);

    RT_ASSERT(cb != RT_NULL);

    rt_memset(nrf24, 0, sizeof(struct nrf24));

    nrf24->send_sem = rt_sem_create("nrfsend", 0, RT_IPC_FLAG_FIFO);
    if (nrf24->send_sem == RT_NULL)
    {
        LOG_E("Failed to create sem");
        return RT_ERROR;
    }

    if (irq_pin != NRF24_PIN_NONE)
    {
        nrf24->sem = rt_sem_create("nrfirq", 0, RT_IPC_FLAG_FIFO);
        if (nrf24->sem == RT_NULL)
        {
            LOG_E("Failed to create sem");
            return RT_ERROR;
        }

        nrf24->flags.using_irq = RT_TRUE;
    }
    else
    {
        nrf24->flags.using_irq = RT_FALSE;
    }

    rt_memcpy(&nrf24->cb, cb, sizeof(struct nrf24_callback));
    rt_memcpy(&nrf24->cfg, cfg, sizeof(struct nrf24_cfg));
    nrf24->cfg._irq_pin = irq_pin;

    if (hal_nrf24_port_init(&nrf24->halport, spi_dev_name, ce_pin, irq_pin, __irq_handler) != RT_EOK)
        return RT_ERROR;

    if (check_halport(&nrf24->halport) != RT_EOK)
        return RT_ERROR;

    if (build_onchip_config(&onchip_cfg, &nrf24->cfg) != RT_EOK)
        return RT_ERROR;

    if (update_onchip_config(nrf24, &onchip_cfg) != RT_EOK)
        return RT_ERROR;

    if (check_onchip_config(nrf24, &onchip_cfg) != RT_EOK)
        return RT_ERROR;

    flush_tx_fifo(nrf24);
    flush_rx_fifo(nrf24);
    clear_status(nrf24, NRF24BITMASK_RX_DR | NRF24BITMASK_TX_DS | NRF24BITMASK_MAX_RT);
    clear_observe_tx(nrf24);

    nrf24_enter_power_up_mode(nrf24);
    nrf24->halport.ops->set_ce(&nrf24->halport);

    LOG_I("Successfully initialized");

#ifdef NRF24_USING_SHELL_CMD
    g_debug_nrf24 = nrf24;
#endif

    return RT_EOK;
}

nrf24_t nrf24_create(char *spi_dev_name, int ce_pin, int irq_pin, const struct nrf24_callback *cb, const nrf24_cfg_t cfg)
{
    RT_ASSERT(cfg != RT_NULL);

    nrf24_t new_nrf24 = (nrf24_t)rt_malloc(sizeof(struct nrf24));
    if (new_nrf24 == RT_NULL)
    {
        rt_free(new_nrf24);
        LOG_E("Failed to allocate memory!");
    }
    else
    {
        if (nrf24_init(new_nrf24, spi_dev_name, ce_pin, irq_pin, cb, cfg) != RT_EOK)
        {
            rt_free(new_nrf24);
            new_nrf24 = RT_NULL;
        }
    }

    if (new_nrf24 == RT_NULL)
        LOG_E("Failed to create nrf24 instance");

    return new_nrf24;
}

int nrf24_default_init(nrf24_t nrf24, char *spi_dev_name, int ce_pin, int irq_pin, const struct nrf24_callback *cb, nrf24_role_et role)
{
    struct nrf24_cfg cfg;

    nrf24_fill_default_config_on(&cfg);
    cfg.role = role;
    return nrf24_init(nrf24, spi_dev_name, ce_pin, irq_pin, cb, &cfg);
}

nrf24_t nrf24_default_create(char *spi_dev_name, int ce_pin, int irq_pin, const struct nrf24_callback *cb, nrf24_role_et role)
{
    nrf24_t new_nrf24 = (nrf24_t)rt_malloc(sizeof(struct nrf24));
    if (new_nrf24 == RT_NULL)
    {
        rt_free(new_nrf24);
        LOG_E("Failed to allocate memory!");
    }
    else
    {
        if (nrf24_default_init(new_nrf24, spi_dev_name, ce_pin, irq_pin, cb, role) != RT_EOK)
        {
            rt_free(new_nrf24);
            new_nrf24 = RT_NULL;
        }
    }

    if (new_nrf24 == RT_NULL)
        LOG_E("Failed to create nrf24 instance");

    return new_nrf24;
}

/**
 * check status and inform
 * @param nrf24 pointer of nrf24 instance
 * @return -x:error   0:nothing   1:tx_done   2:rx_done   3:tx_rx_done
 */
int nrf24_run(nrf24_t nrf24)
{
    int rvl = 0;

    if (nrf24->flags.using_irq)
    {
        rt_sem_take(nrf24->sem, RT_WAITING_FOREVER);
    }

    nrf24->status = read_status(nrf24);
    clear_status(nrf24, NRF24BITMASK_RX_DR | NRF24BITMASK_TX_DS);

    uint8_t pipe = (nrf24->status & NRF24BITMASK_RX_P_NO) >> 1;

    if (nrf24->cfg.role == ROLE_PTX)
    {
        if (nrf24->status & NRF24BITMASK_MAX_RT)
        {
            flush_tx_fifo(nrf24);
            clear_status(nrf24, NRF24BITMASK_MAX_RT);
            if(nrf24->cb.tx_done) nrf24->cb.tx_done(nrf24, NRF24_PIPE_NONE);
            return -1;
        }

        if (nrf24->status & NRF24BITMASK_RX_DR)
        {
            uint8_t data[32];
            uint8_t len = read_top_rxfifo_width(nrf24);

            read_rx_payload(nrf24, data, len);
            if (nrf24->cb.rx_ind) nrf24->cb.rx_ind(nrf24, data, len, pipe);

            rvl |= 2;
        }

        if (nrf24->status & NRF24BITMASK_TX_DS)
        {
            if (nrf24->cb.tx_done) nrf24->cb.tx_done(nrf24, pipe);

            rvl |= 1;
        }
    }
    else
    {
        if (pipe <= 5)
        {
            uint8_t data[32];
            uint8_t len = read_top_rxfifo_width(nrf24);

            read_rx_payload(nrf24, data, len);
            if (nrf24->cb.rx_ind) nrf24->cb.rx_ind(nrf24, data, len, pipe);

            rvl |= 2;

            if (rt_sem_trytake(nrf24->send_sem) == RT_EOK)
            {
                if (nrf24->cb.tx_done) nrf24->cb.tx_done(nrf24, pipe);
                rvl |= 1;
            }
        }
    }

    return rvl;
}

/** S DEBUG */
/** |||||| **/

#if defined(NRF24_USING_INFO_REPORT)

void __nrf24_report_config_reg(uint8_t data)
{
    if (data & NRF24BITMASK_PRIM_RX)
        rt_kprintf("PRX mode\r\n");
    else
        rt_kprintf("PTX mode\r\n");
    if (data & NRF24BITMASK_EN_CRC)
    {
        rt_kprintf("crc opened. FCS: ");
        if (data & NRF24BITMASK_CRCO)
            rt_kprintf("2bytes\r\n");
        else
            rt_kprintf("1byte\r\n");
    }
    else
        rt_kprintf("crc closed\r\n");
    if (!(data & (NRF24BITMASK_RX_DR | NRF24BITMASK_TX_DS | NRF24BITMASK_MAX_RT)))
    {
        if (!(data & NRF24BITMASK_RX_DR))
            rt_kprintf("RX irq; ");
        if (!(data & NRF24BITMASK_TX_DS))
            rt_kprintf("TX irq; ");
        if (!(data & NRF24BITMASK_MAX_RT))
            rt_kprintf("MAX_RT irq; ");
        rt_kprintf("opened\r\n");
    }
    else
    {
        rt_kprintf("all irq closed\r\n");
    }
    if (data & NRF24BITMASK_PWR_UP)
        rt_kprintf("power up now\r\n");
    else
        rt_kprintf("power down now\r\n");
}

void __nrf24_report_enaa_reg(uint8_t data)
{
    if (!(data & 0x3F))
    {
        rt_kprintf("all pipe AA closed");
        return;
    }

    if (data & NRF24BITMASK_PIPE_0)
        rt_kprintf("pipe0 ");
    if (data & NRF24BITMASK_PIPE_1)
        rt_kprintf("pipe1 ");
    if (data & NRF24BITMASK_PIPE_2)
        rt_kprintf("pipe2 ");
    if (data & NRF24BITMASK_PIPE_3)
        rt_kprintf("pipe3 ");
    if (data & NRF24BITMASK_PIPE_4)
        rt_kprintf("pipe4 ");
    if (data & NRF24BITMASK_PIPE_5)
        rt_kprintf("pipe5 ");
    rt_kprintf("AA opened\r\n");
}
void __nrf24_report_enrxaddr_reg(uint8_t data)
{
    if (!(data & 0x3F))
    {
        rt_kprintf("all rx-pipe closed");
        return;
    }

    if (data & NRF24BITMASK_PIPE_0)
        rt_kprintf("rx-pipe0 ");
    if (data & NRF24BITMASK_PIPE_1)
        rt_kprintf("rx-pipe1 ");
    if (data & NRF24BITMASK_PIPE_2)
        rt_kprintf("rx-pipe2 ");
    if (data & NRF24BITMASK_PIPE_3)
        rt_kprintf("rx-pipe3 ");
    if (data & NRF24BITMASK_PIPE_4)
        rt_kprintf("rx-pipe4 ");
    if (data & NRF24BITMASK_PIPE_5)
        rt_kprintf("rx-pipe5 ");
    rt_kprintf(" opened\r\n");
}
void __nrf24_report_setupaw_reg(uint8_t data)
{
    rt_kprintf("rx/tx address field width: ");
    switch (data & 0x3)
    {
    case 0:
        rt_kprintf("illegal\r\n");
        break;
    case 1:
        rt_kprintf("3bytes\r\n");
        break;
    case 2:
        rt_kprintf("4bytes\r\n");
        break;
    case 3:
        rt_kprintf("5bytes\r\n");
        break;
    }
}
void __nrf24_report_setupretr_reg(uint8_t data)
{
    rt_kprintf("auto retransmit delay: %dus\r\n", (((data & 0xF0) >> 4) + 1) * 250);
    rt_kprintf("auto retransmit count: up to %d\r\n", (data & 0x0F));
}
void __nrf24_report_rfch_reg(uint8_t data)
{
    rt_kprintf("rf channel: %d\r\n", data & 0x7F);
}
void __nrf24_report_rfsetup_reg(uint8_t data)
{
    rt_kprintf("air data rate: ");
    if (data & NRF24BITMASK_RF_DR)
        rt_kprintf("2Mbps\r\n");
    else
        rt_kprintf("1Mbsp\r\n");

    rt_kprintf("rf power: ");
    switch ((data & NRF24BITMASK_RF_PWR) >> 1)
    {
    case 0:
        rt_kprintf("-18dBm\r\n");
        break;
    case 1:
        rt_kprintf("-12dBm\r\n");
        break;
    case 2:
        rt_kprintf("-6dBm\r\n");
        break;
    case 3:
        rt_kprintf("0dBm\r\n");
        break;
    }
}
void __nrf24_report_status_reg(uint8_t data)
{
    rt_kprintf("status: ");
    if (data & NRF24BITMASK_RX_DR)
        rt_kprintf("new rx data; ");
    if (data & NRF24BITMASK_TX_DS)
        rt_kprintf("last tx ok; ");
    if (data & NRF24BITMASK_MAX_RT)
        rt_kprintf("max-rt error exist; ");

    if (data & NRF24BITMASK_TX_FULL)
        rt_kprintf("tx-fifo is full; ");
    else
        rt_kprintf("tx-fifo is not full; ");

    data = (data & NRF24BITMASK_RX_P_NO) >> 1;
    if (data > 5)
    {
        if (data == 7)
            rt_kprintf("rx-fifo empty; ");
        else
            rt_kprintf("rx-fifo not used?; ");
    }
    else
    {
        rt_kprintf("rx-fifo pipe: %d; ", data);
    }

    rt_kprintf("\r\n");
}
void __nrf24_report_observetx_reg(uint8_t data)
{
    rt_kprintf("lost packets count: %d\r\n", (data & NRF24BITMASK_PLOS_CNT) >> 4);
    rt_kprintf("retransmitted packets count: %d\r\n", data & NRF24BITMASK_ARC_CNT);
}

void __nrf24_report_fifostatus_reg(uint8_t data)
{
    if (data & NRF24BITMASK_TX_REUSE)
        rt_kprintf("tx-reuse opened\r\n");

    if (data & NRF24BITMASK_TX_FULL2)
        rt_kprintf("tx-fifo full\r\n");
    else if (data & NRF24BITMASK_TX_EMPTY)
        rt_kprintf("tx-fifo empty\r\n");
    else
        rt_kprintf("tx-fifo has some data\r\n");

    if (data & NRF24BITMASK_RX_RXFULL)
        rt_kprintf("rx-fifo full\r\n");
    else if (data & NRF24BITMASK_RX_EMPTY)
        rt_kprintf("rx-fifo empty\r\n");
    else
        rt_kprintf("rx-fifo has some data\r\n");
}
void __nrf24_report_dynpd_reg(uint8_t data)
{
    rt_kprintf("dynamic payload length enabled (pipe): ");
    if (!(data & 0x3F))
    {
        rt_kprintf("none\r\n");
        return;
    }

    if (data & NRF24BITMASK_PIPE_0)
        rt_kprintf("pipe0; ");
    if (data & NRF24BITMASK_PIPE_1)
        rt_kprintf("pipe1; ");
    if (data & NRF24BITMASK_PIPE_2)
        rt_kprintf("pipe2; ");
    if (data & NRF24BITMASK_PIPE_3)
        rt_kprintf("pipe3; ");
    if (data & NRF24BITMASK_PIPE_4)
        rt_kprintf("pipe4; ");
    if (data & NRF24BITMASK_PIPE_5)
        rt_kprintf("pipe5; ");
    rt_kprintf("\r\n");
}
void __nrf24_report_feature_reg(uint8_t data)
{
    rt_kprintf("feature enabled conditions: ");
    if (data & NRF24BITMASK_EN_DPL)
        rt_kprintf("dynamic payload length; ");
    if (data & NRF24BITMASK_EN_ACK_PAY)
        rt_kprintf("payload with ack; ");
    if (data & NRF24BITMASK_EN_DYN_ACK)
        rt_kprintf("W_TX_PAYLOAD_NOACK command; ");

    rt_kprintf("\r\n");
}

void __nrf24_report_addr(nrf24_t nrf24)
{
    struct nrf24_onchip_cfg ccfg;
    read_onchip_config(nrf24, &ccfg);
    rt_kprintf("tx-addr:0x%02x%02x%02x%02x%02x\n", 
        ccfg.tx_addr[4],
        ccfg.tx_addr[3],
        ccfg.tx_addr[2],
        ccfg.tx_addr[1],
        ccfg.tx_addr[0]);

    rt_kprintf("rx-addr-p0:0x%02x%02x%02x%02x%02x\n", 
        ccfg.rx_addr_p0[4],
        ccfg.rx_addr_p0[3],
        ccfg.rx_addr_p0[2],
        ccfg.rx_addr_p0[1],
        ccfg.rx_addr_p0[0]);

    rt_kprintf("rx-addr-p1:0x%02x%02x%02x%02x%02x\n", 
        ccfg.rx_addr_p1[4],
        ccfg.rx_addr_p1[3],
        ccfg.rx_addr_p1[2],
        ccfg.rx_addr_p1[1],
        ccfg.rx_addr_p1[0]);

    rt_kprintf("rx-addr-p2:0x%02x\n", ccfg.rx_addr_p2);
    rt_kprintf("rx-addr-p3:0x%02x\n", ccfg.rx_addr_p3);
    rt_kprintf("rx-addr-p4:0x%02x\n", ccfg.rx_addr_p4);
    rt_kprintf("rx-addr-p5:0x%02x\n", ccfg.rx_addr_p5);
}

void nrf24_report(nrf24_t nrf24)
{
    __nrf24_report_config_reg(__read_reg(nrf24, NRF24REG_CONFIG));
    __nrf24_report_enaa_reg(__read_reg(nrf24, NRF24REG_EN_AA));
    __nrf24_report_enrxaddr_reg(__read_reg(nrf24, NRF24REG_EN_RXADDR));
    __nrf24_report_setupaw_reg(__read_reg(nrf24, NRF24REG_SETUP_AW));
    __nrf24_report_setupretr_reg(__read_reg(nrf24, NRF24REG_SETUP_RETR));
    __nrf24_report_rfch_reg(__read_reg(nrf24, NRF24REG_RF_CH));
    __nrf24_report_rfsetup_reg(__read_reg(nrf24, NRF24REG_RF_SETUP));
    __nrf24_report_status_reg(__read_reg(nrf24, NRF24REG_STATUS));
    __nrf24_report_observetx_reg(__read_reg(nrf24, NRF24REG_OBSERVE_TX));

    __nrf24_report_fifostatus_reg(__read_reg(nrf24, NRF24REG_FIFO_STATUS));
    __nrf24_report_dynpd_reg(__read_reg(nrf24, NRF24REG_DYNPD));
    __nrf24_report_feature_reg(__read_reg(nrf24, NRF24REG_FEATURE));
    __nrf24_report_addr(nrf24);
}

#endif // NRF24_USING_INFO_REPORT

#ifdef NRF24_USING_SHELL_CMD
#include <stdlib.h>

static void nrf24(int argc, char **argv)
{
    static nrf24_t instance = RT_NULL;

    if (argc < 2)
    {
        rt_kprintf("Usage: nrf24 [OPTION]\n");
        rt_kprintf("Options:\n");
        rt_kprintf("    init <spi_dev_name> <ce_pin>\n");
        rt_kprintf("    probe\n");
        rt_kprintf("    check_halport\n");
        rt_kprintf("    read <reg>\n");
        rt_kprintf("    write <reg> <byte>\n");
#ifdef NRF24_USING_INFO_REPORT
        rt_kprintf("    report\n");
#endif // NRF24_USING_INFO_REPORT

        return;
    }

#ifdef NRF24_USING_INFO_REPORT
    if (!rt_strcmp(argv[1], "report"))
    {
        nrf24_report(instance);
    }
#endif // NRF24_USING_INFO_REPORT

    if (!rt_strcmp(argv[1], "check_halport"))
    {
        if (check_halport(&instance->halport) == RT_EOK)
            rt_kprintf("OK\n");
        else
            rt_kprintf("ERROR\n");
    }
    else if (!rt_strcmp(argv[1], "probe"))
    {
        if (g_debug_nrf24 != RT_NULL)
        {
            instance = g_debug_nrf24;
            rt_kprintf("OK\n");
        }
        else
            rt_kprintf("ERROR\n");
    }

    if (argc < 3)
    {
        return;
    }
    if (!rt_strcmp(argv[1], "read"))
    {
        uint8_t reg = atoi(argv[2]);
        rt_kprintf("reg:0x%x val: 0x%x\n", reg, __read_reg(instance, reg));
    }

    if (argc < 4)
    {
        return;
    }
    if (!rt_strcmp(argv[1], "write"))
    {
        uint8_t reg = atoi(argv[2]);
        uint8_t data = atoi(argv[3]);
        __write_reg(instance, reg, data);
    }
    else if (!rt_strcmp(argv[1], "init"))
    {
        char *name = argv[2];
        int pin = atoi(argv[3]);
        struct nrf24_callback cb = {0};

        if (instance != RT_NULL)
            return;

        instance = nrf24_default_create(name, pin, NRF24_PIN_NONE, &cb, ROLE_PTX);

        if (instance)
            rt_kprintf("OK\n");
        else
            rt_kprintf("\nFAILED!\n");
    }
}
MSH_CMD_EXPORT(nrf24, nrf24l01);

#endif // NRF24_USING_SHELL_CMD

/** E DEBUG */
