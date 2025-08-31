#include <stdint.h>

///<寄存器
#define NRF24_REG_CONFIG                    ((uint8_t)0x00)     // 配置收发状态，CRC校验模式以及收发状态响应方式
#define NRF24_REG_EN_AA                     ((uint8_t)0x01)     // 自动应答功能设置
#define NRF24_REG_EN_RXADDR                 ((uint8_t)0x02)     // 可用信道设置
#define NRF24_REG_SETUP_AW                  ((uint8_t)0x03)     // 收发地址宽度设置
#define NRF24_REG_SETUP_RETR                ((uint8_t)0x04)     // 自动重发功能设置
#define NRF24_REG_RF_CH                     ((uint8_t)0x05)     // 工作频率设置
#define NRF24_REG_RF_SETUP                  ((uint8_t)0x06)     // 发射速率、功耗功能设置
#define NRF24_REG_STATUS                    ((uint8_t)0x07)     // 状态寄存器
#define NRF24_REG_OBSERVE_TX                ((uint8_t)0x08)     // 发送监测功能
#define NRF24_REG_RPD                       ((uint8_t)0x09)     // 接收功率检测           
#define NRF24_REG_RX_ADDR_P0                ((uint8_t)0x0A)     // 频道0接收数据地址
#define NRF24_REG_RX_ADDR_P1                ((uint8_t)0x0B)     // 频道1接收数据地址
#define NRF24_REG_RX_ADDR_P2                ((uint8_t)0x0C)     // 频道2接收数据地址
#define NRF24_REG_RX_ADDR_P3                ((uint8_t)0x0D)     // 频道3接收数据地址
#define NRF24_REG_RX_ADDR_P4                ((uint8_t)0x0E)     // 频道4接收数据地址
#define NRF24_REG_RX_ADDR_P5                ((uint8_t)0x0F)     // 频道5接收数据地址
#define NRF24_REG_TX_ADDR                   ((uint8_t)0x10)     // 发送地址寄存器
#define NRF24_REG_RX_PW_P0                  ((uint8_t)0x11)     // 接收频道0接收数据长度
#define NRF24_REG_RX_PW_P1                  ((uint8_t)0x12)     // 接收频道1接收数据长度
#define NRF24_REG_RX_PW_P2                  ((uint8_t)0x13)     // 接收频道2接收数据长度
#define NRF24_REG_RX_PW_P3                  ((uint8_t)0x14)     // 接收频道3接收数据长度
#define NRF24_REG_RX_PW_P4                  ((uint8_t)0x15)     // 接收频道4接收数据长度
#define NRF24_REG_RX_PW_P5                  ((uint8_t)0x16)     // 接收频道5接收数据长度
#define NRF24_REG_FIFO_STATUS               ((uint8_t)0x17)     // FIFO栈入栈出状态寄存器设置
#define NRF24_REG_DYNPD                     ((uint8_t)0x1C)     // 动态数据包长度
#define NRF24_REG_FEATURE                   ((uint8_t)0x1D)     // 特性寄存器  

typedef struct {
    // 1: PRX
    uint8_t prim_rx     :1;
    // 1: power up
    uint8_t pwr_up      :1;
    uint8_t crco        :1;
    uint8_t en_crc      :1;
    // 0: max_rt will trigger irq
    uint8_t mask_max_rt :1;
    // 0: tx_ds will trigger irq
    uint8_t mask_tx_ds  :1;
    // 0: rx_dr will trigger irq
    uint8_t mask_rx_dr  :1;
    uint8_t __rvsd_7_7  :1;
} reg_config_t;

typedef struct {
    uint8_t p0          :1;
    uint8_t p1          :1;
    uint8_t p2          :1;
    uint8_t p3          :1;
    uint8_t p4          :1;
    uint8_t p5          :1;
    uint8_t __rvsd_7_6  :2;
} reg_en_aa_t;

typedef struct {
    uint8_t p0          :1;
    uint8_t p1          :1;
    uint8_t p2          :1;
    uint8_t p3          :1;
    uint8_t p4          :1;
    uint8_t p5          :1;
    uint8_t __rvsd_7_6  :2;
} reg_en_rxaddr_t;

typedef struct {
    uint8_t aw          :2;
    uint8_t __rvsd_7_2  :6;
} reg_setup_aw_t;

typedef struct {
    // Auto-retransmit count
    uint8_t arc         :4;
    // Auto-retransmit delay
    uint8_t ard         :4;
} reg_setup_retr_t;

typedef struct {
    uint8_t rf_ch       :7;
    uint8_t __rvsd_7_7  :1;
} reg_rf_ch_t;

typedef struct {
    uint8_t lna_hcurr   :1;
    // output tx power
    uint8_t rf_pwr      :2;
    // air-data-rate
    uint8_t rf_dr       :1;     
    uint8_t pll_lock    :1;
    uint8_t __rvsd_7_5  :3;
} reg_rf_setup_t;

typedef struct {
    uint8_t tx_full     :1;
    uint8_t rx_p_no     :3;
    uint8_t max_rt      :1;
    uint8_t tx_ds       :1;
    uint8_t rx_dr       :1;
    uint8_t __rvsd_7_7  :1;
} reg_status_t;

typedef struct {
    uint8_t arc_cnt     :4;
    uint8_t plos_cnt    :4;
} reg_observe_tx_t;

typedef struct {
    uint8_t cd          :1;
    uint8_t __rvsd_7_1  :7;
} reg_cd_t;

typedef struct {
    uint8_t rx_empty    :1;
    uint8_t rx_full     :1;
    uint8_t __rvsd_3_2  :2;
    uint8_t tx_empty    :1;
    uint8_t tx_full     :1;
    uint8_t tx_reuse    :1;
    uint8_t __rvsd_7_7  :1;
} reg_fifo_status_t;

typedef struct {
    uint8_t p0          :1;
    uint8_t p1          :1;
    uint8_t p2          :1;
    uint8_t p3          :1;
    uint8_t p4          :1;
    uint8_t p5          :1;
    uint8_t __rvsd_7_6  :2;
} reg_dynpd_t;

typedef struct {
    uint8_t en_dyn_ack  :1;
    uint8_t en_ack_pay  :1;
    uint8_t en_dpl      :1;
    uint8_t __rvsd_7_3  :5;
} reg_feature_t;

typedef struct 
{
    reg_config_t config;
    reg_en_aa_t en_aa;
    reg_en_rxaddr_t en_rxaddr;
    reg_setup_aw_t setup_aw;
    reg_setup_retr_t setup_retr;
    reg_rf_ch_t rf_ch;
    reg_rf_setup_t rf_setup;

    uint8_t rx_addr_p0[5]; // LSB
    uint8_t rx_addr_p1[5]; // LSB
    uint8_t rx_addr_p2;
    uint8_t rx_addr_p3;
    uint8_t rx_addr_p4;
    uint8_t rx_addr_p5;

    uint8_t tx_addr[5]; // LSB

    reg_dynpd_t dynpd;
    reg_feature_t feature;

}nrf24_regs_cfg_t;

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