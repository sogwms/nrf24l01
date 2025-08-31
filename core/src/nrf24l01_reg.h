/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright sogwms
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-26     sogwms       first version       
 */

#ifndef NRF24L01_REG_H
#define NRF24L01_REG_H

///<命令
#define NRF24_CMD_R_REG                     ((uint8_t)0x00)     // 读寄存器
#define NRF24_CMD_W_REG                     ((uint8_t)0x20)     // 写寄存器
#define NRF24_CMD_R_RX_PAYLOAD              ((uint8_t)0x61)     // 读接收缓冲区
#define NRF24_CMD_W_TX_PAYLOAD              ((uint8_t)0xA0)     // PTX, 写发送缓冲区
#define NRF24_CMD_FLUSH_TX                  ((uint8_t)0xE1)     // 清空发送FIFO
#define NRF24_CMD_FLUSH_RX                  ((uint8_t)0xE2)     // 清空接收FIFO
#define NRF24_CMD_REUSE_TX_PL               ((uint8_t)0xE3)     // PTX模式下使用, 重用发送FIFO数据
#define NRF24_CMD_ACTIVATE                  ((uint8_t)0x50)     // 
#define NRF24_CMD_R_RX_PL_WID               ((uint8_t)0x60)     // 读顶层接收FIFO大小
#define NRF24_CMD_W_ACK_PAYLOAD             ((uint8_t)0xA8)     // PRX模式下使用，写应答发送缓冲区
#define NRF24_CMD_W_TX_PAYLOAD_NO_ACK       ((uint8_t)0xB0)     // PTX，写发送缓冲区，接收方不发送ACK
#define NRF24_CMD_NOP                       ((uint8_t)0xFF)
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
///<寄存器功能位掩码
// //CONFIG
#define REG_CONFIG_BITMASK_RX_DR            ((uint8_t)(1<<6))   // 接收完成中断使能位
#define REG_CONFIG_BITMASK_TX_DS            ((uint8_t)(1<<5))   // 发送完成中断使能位
#define REG_CONFIG_BITMASK_MAX_RT           ((uint8_t)(1<<4))   // 达最大重发次数中断使能位
#define REG_CONFIG_BITMASK_EN_CRC           ((uint8_t)(1<<3))   // CRC使能位
#define REG_CONFIG_BITMASK_CRCO             ((uint8_t)(1<<2))   // CRC编码方式 （1B or 2B）
#define REG_CONFIG_BITMASK_PWR_UP           ((uint8_t)(1<<1))   // 上（掉）电
#define REG_CONFIG_BITMASK_PRIM_RX          ((uint8_t)(1))      // PR（T）X
//SETUP_AW
#define REG_AW_BITMASK_AW                   ((uint8_t)(0x03))   // RX/TX地址宽度
//SETUP_RETR
#define REG_SETUP_RETR_BITMASK_ARD          ((uint8_t)(0xF0))   // 重发延时
#define REG_SETUP_RETR_BITMASK_ARC          ((uint8_t)(0x0F))   // 重发最大次数
//RF_CH
#define REG_RF_CH_BITMASK_RF_CH             ((uint8_t)(0x7F))   // 射频频道
//RF_SETUP
#define REG_RF_SETUP_BITMASK_RF_DR          ((uint8_t)(1<<3))   // 空中速率
#define REG_RF_SETUP_BITMASK_RF_PWR         ((uint8_t)(0x06))   // 发射功率
//STATUS
#define REG_STATUS_BITMASK_RX_DR            ((uint8_t)(1<<6))   // 接收完成标志位
#define REG_STATUS_BITMASK_TX_DS            ((uint8_t)(1<<5))   // 发送完成标志位
#define REG_STATUS_BITMASK_MAX_RT           ((uint8_t)(1<<4))   // 最大重发次数标志位
#define REG_STATUS_BITMASK_RX_P_NO          ((uint8_t)(0x0E))   // RX_FIFO状态标志区位
#define REG_STATUS_BITMASK_TX_FULL          ((uint8_t)(1))      // TX_FIFO满标志位
//OBSERVE_TX
#define REG_OBSERVE_TX_BITMASK_PLOS_CNT     ((uint8_t)(0xF0))   // 丢包计数
#define REG_OBSERVE_TX_BITMASK_ARC_CNT      ((uint8_t)(0x0F))   // 重发计数
//CD
#define REG_CD_BITMASK_CD                   ((uint8_t)(1))      // 载波检测标志位
//通用掩码，RX_PW_P[0::5] 掩码相同
#define BITMASK_RX_PW_Px                    ((uint8_t)(0x3F))   // 数据管道RX-Payload中的字节数
//FIFO_STATUS
#define REG_FIFO_STATUS_BITMASK_TX_REUSE    ((uint8_t)(1<<6))   // 
#define REG_FIFO_STATUS_BITMASK_TX_FULL     ((uint8_t)(1<<5))   // 
#define REG_FIFO_STATUS_BITMASK_TX_EMPTY    ((uint8_t)(1<<4))   // 
#define REG_FIFO_STATUS_BITMASK_RX_RXFULL   ((uint8_t)(1<<1))   // 
#define REG_FIFO_STATUS_BITMASK_RX_EMPTY    ((uint8_t)(1))      // 
//FEATURE
#define REG_FEATURE_BITMASK_EN_DPL          ((uint8_t)(1<<2))   // 动态长度使能位
#define REG_FEATURE_BITMASK_EN_ACK_PAY      ((uint8_t)(1<<1))   // Payload with ACK 使能位
#define REG_FEATURE_BITMASK_EN_DYN_ACK      ((uint8_t)(1))      // W_TX_PAYLOAD_NOACK 命令使能位
//通用掩码，适用于多个寄存器： EN_AA, EN_RXADDR, DYNPD
#define BITMASK_PIPE_0                      ((uint8_t)(1))     // 
#define BITMASK_PIPE_1                      ((uint8_t)(1<<1))  // 
#define BITMASK_PIPE_2                      ((uint8_t)(1<<2))  // 
#define BITMASK_PIPE_3                      ((uint8_t)(1<<3))  // 
#define BITMASK_PIPE_4                      ((uint8_t)(1<<4))  // 
#define BITMASK_PIPE_5                      ((uint8_t)(1<<5))  // 

#endif // NRF24L01_REG_H
