/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright sogwms
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-26     sogwms       first version       
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "nrf24_cmd_core.h"
#include "nrf24_reg_def.h"
#include <nrf24l01.h>
#include <nrf24l01_dep_impl.h>

#define PT_UTILIZE_ALL_FIFOS
#define PT_DEFAULT_DURATION (3)

#define MAX(x, y) (x > y ? x : y)

#ifndef PRINT
#error Missing PRINT definition, Which is like `printf`
#endif

#ifndef TIME_GET_MS
#error Missing definition
#endif

#ifndef TIME_WAIT_MS
#error Missing definition
#endif

#ifndef USER_SUBCMDS
#define USER_SUBCMDS
#endif

static nrf24_t *g_cmd_nrf24 = 0;
static void subcmd_help(int argc, char **argv);
static void print_array(uint8_t *data, uint8_t len) {
    for (int i = 0; i < len; i++) {
        PRINT("%02x ", data[i]);
    }
    PRINT("\n");
}

static void print_array2(uint8_t *data, uint8_t len, char *format, int reverse,
                         int prefix, int newline) {
    if (prefix) {
        PRINT("0x");
    }

    if (reverse) {
        for (int i = len - 1; i >= 0; i--) {
            PRINT(format, data[i]);
        }
    } else {
        for (int i = 0; i < len; i++) {
            PRINT(format, data[i]);
        }
    }

    if (newline) {
        PRINT("\n");
    }
}

static void increment_u8arr_content(uint8_t *buf, int len) {
    int i = 0;
    if (buf[0] == 0xFF && buf[len - 1] == 0xFF) {
        memset(buf, 0, len);
    }

    for (; i < len; i++) {
        if (buf[i] == 0xFF) {
            continue;
        }
        buf[i]++;
        break;
    }
}

static void subcmd_access_reg(int argc, char **argv) {
    if (argc != 1 && argc != 2) {
        PRINT("wrong arguments, check help for detail\n");
        return;
    }

    char *endptr;

    int reg = strtol(argv[0], &endptr, 0);
    if (*endptr != '\0') {
        PRINT("Error: Invalid register address '%s'\n", argv[1]);
        return;
    }

    if (argc == 1) {
        uint8_t value = nrf24_read_reg(g_cmd_nrf24, reg);
        PRINT("0x%02X: 0x%02X\n", reg, value);
    } else if (argc == 2) {
        int val = strtol(argv[1], &endptr, 0);
        if (*endptr != '\0') {
            PRINT("Error: Invalid value '%s'\n", argv[2]);
            return;
        }

        nrf24_write_reg(g_cmd_nrf24, reg, val);
    }
}

static void subcmd_access_cfg(int argc, char **argv) {
    nrf24_user_cfg_t cfg;
    nrf24_user_cfg_t cfg_backup;
    int changed = 0;

    // 读取当前配置
    if (nrf24_usercfg_read(g_cmd_nrf24, &cfg) != 0) {
        PRINT("Failed to read current config\n");
        return;
    }

    memcpy(&cfg_backup, &cfg, sizeof(cfg));

    if (argc == 0) {
        // 打印当前配置
        PRINT("Current Config:\n");
        PRINT("\tadr:     %d Mbps\n",
                   cfg.rf_adr == NRF24_ADR_1Mbps ? 1 : 2);
        PRINT("\tpower:   %d (3=MAX)\n", cfg.rf_power);
        PRINT("\tchannel: %d (2.%03dGHz)\n", cfg.rf_channel,
                   400 + cfg.rf_channel);
        if (nrf24_role_is_ptx(g_cmd_nrf24)) {

            PRINT("\ttxaddr:      ");
            print_array2(cfg.tx_addr, 5, "%02X", 1, 1, 1);
        }
        PRINT("\trxpipe0: %-3s ", cfg.rxpipes[0].enable ? "on" : "off");
        print_array2(cfg.rxpipes[0].addr, 5, "%02X", 1, 1, 1);

        if (nrf24_role_is_prx(g_cmd_nrf24)) {
            PRINT("\trxpipe1: %-3s ", cfg.rxpipes[1].enable ? "on" : "off");
            print_array2(cfg.rxpipes[1].addr, 5, "%02X", 1, 1, 1);

            PRINT("\trxpipe2: %-3s 0x--------%02X\n",
                       cfg.rxpipes[2].enable ? "on" : "off", cfg.rxpipes[2].addr_lsb);
            PRINT("\trxpipe3: %-3s 0x--------%02X\n",
                       cfg.rxpipes[3].enable ? "on" : "off", cfg.rxpipes[3].addr_lsb);
            PRINT("\trxpipe4: %-3s 0x--------%02X\n",
                       cfg.rxpipes[4].enable ? "on" : "off", cfg.rxpipes[4].addr_lsb);
            PRINT("\trxpipe5: %-3s 0x--------%02X\n",
                       cfg.rxpipes[5].enable ? "on" : "off", cfg.rxpipes[5].addr_lsb);
        }
        return;
    }

    // 处理参数
    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "adr") == 0 && i + 1 < argc) {
            int val = strtol(argv[++i], NULL, 0);
            if (val == 1) {
                cfg.rf_adr = NRF24_ADR_1Mbps;
            } else if (val == 2) {
                cfg.rf_adr = NRF24_ADR_2Mbps;
            } else {
                PRINT("Error: Invalid ADR value. Use 1 or 2.\n");
            }
        } else if (strcmp(argv[i], "power") == 0 && i + 1 < argc) {
            int val = strtol(argv[++i], NULL, 0);
            if (val >= 0 && val <= 3) {
                cfg.rf_power = val;
            } else {
                PRINT("Error: Power must be 0~3.\n");
            }
        } else if (strcmp(argv[i], "ch") == 0 && i + 1 < argc) {
            int val = strtol(argv[++i], NULL, 0);
            if (val >= 0 && val <= 125) {
                cfg.rf_channel = val;
            } else {
                PRINT("Error: Channel must be 0~125.\n");
            }
        } else if (strcmp(argv[i], "txaddr") == 0) {
            uint8_t addr[5] = {0};
            int j;
            for (j = 0; j < 5 && i + 1 + j < argc; j++) {
                int val = strtol(argv[i + 1 + j], NULL, 0);
                if (val < 0 || val > 255) {
                    PRINT("Error: Invalid byte value in TX address.\n");
                    return;
                }
                addr[j] = val;
            }
            if (j == 0) {
                PRINT("Error: Missing TX address bytes.\n");
                return;
            }
            // LSB first -> stored as addr[0] is LSB
            for (int k = 0; k < j; k++) {
                cfg.tx_addr[k] = addr[k];
            }
            i += j;
        }

        else if (strncmp(argv[i], "rp", 2) == 0 && strlen(argv[i]) == 3) {
            int pipe_num = argv[i][2] - '0';
            if (pipe_num < 0 || pipe_num > 5) {
                PRINT("Error: Pipe number must be 0~5.\n");
                return;
            }

            if (i + 1 >= argc) {
                PRINT("Error: Missing pipe action.\n");
                return;
            }

            char *action = argv[++i];

            if (strcmp(action, "on") == 0) {
                cfg.rxpipes[pipe_num].enable = 1;
            } else if (strcmp(action, "off") == 0) {
                cfg.rxpipes[pipe_num].enable = 0;
            } else if (strcmp(action, "addr") == 0 && i + 1 < argc) {
                if (pipe_num == 0 || pipe_num == 1) {
                    // rp0: 5 bytes
                    for (int j = 0; j < 5 && i + 1 + j < argc; j++) {
                        int val = strtol(argv[i + 1 + j], NULL, 0);
                        if (val < 0 || val > 255) {
                            PRINT("Error: RP0 address byte invalid.\n");
                            return;
                        }
                        cfg.rxpipes[pipe_num].addr[j] = val;
                    }
                    i += 5;

                } else {
                    // rp2~rp5: single byte
                    int val = strtol(argv[++i], NULL, 0);
                    if (val < 0 || val > 255) {
                        PRINT("Error: Address must be 0~255.\n");
                        return;
                    }
                    cfg.rxpipes[pipe_num].addr_lsb = val;
                }
            } else {
                PRINT(
                    "Error: Invalid pipe action. Use 'on', 'off' or 'addr'.\n");
                return;
            }
        } else {
            PRINT("Error: Unknown config item '%s'\n", argv[i]);
            return;
        }
    }

    if (memcmp(&cfg, &cfg_backup, sizeof(nrf24_user_cfg_t))) {
        if (nrf24_usercfg_write(g_cmd_nrf24, &cfg) != 0) {
            PRINT("Failed to write config\n");
        } else {
            // PRINT("Config updated.\n");
        }
    }
}

static void subcmd_report_status(int argc, char **argv) {
    int ret = 0;
    nrf24_regs_sta_t sta;
    *(uint8_t *)&sta.status = nrf24_read_reg(g_cmd_nrf24, NRF24_REG_STATUS);
    *(uint8_t *)&sta.fifo_status =
        nrf24_read_reg(g_cmd_nrf24, NRF24_REG_FIFO_STATUS);
    *(uint8_t *)&sta.observe_tx =
        nrf24_read_reg(g_cmd_nrf24, NRF24_REG_OBSERVE_TX);

    if (ret != 0) {
        PRINT("read register failed\r\n");
    }

    PRINT("%s\n", nrf24_role_is_ptx(g_cmd_nrf24) ? "PTX" : "PRX");
    PRINT("rx:\n");
    PRINT("    has new rx-data? : %s ",
               sta.status.rx_p_no != 0x07 ? "yes" : "no");
    if (sta.status.rx_dr) {
        PRINT("(pipe %d)\n", sta.status.rx_p_no);
    } else {
        PRINT("\n");
    }
    PRINT("    rxfifo sta: %s\n", sta.fifo_status.rx_full == 1 ? "full"
                                       : sta.fifo_status.rx_empty == 1
                                           ? "empty"
                                           : "have");

    PRINT("tx:\n");
    PRINT("    tx-data sent ok? : %s\n",
               sta.status.tx_ds == 1 ? "yes" : "no");
    PRINT("    txfifo sta: %s\n", sta.fifo_status.tx_full == 1 ? "full"
                                       : sta.fifo_status.tx_empty == 1
                                           ? "empty"
                                           : "have");
    if (sta.fifo_status.tx_reuse) {
        PRINT("    txfifo reused!");
    }

    if (nrf24_role_is_ptx(g_cmd_nrf24)) {
        PRINT("    max tx-retry? : %s\n",
                   sta.status.max_rt == 1 ? "yes" : "no");
        PRINT("    tx-retry counter : %d\n", sta.observe_tx.arc_cnt);
        PRINT("    tx-failed counter : %d\n", sta.observe_tx.plos_cnt);
    }
}

static void subcmd_power_up(int argc, char **argv) { nrf24_power_up(g_cmd_nrf24); }

static void subcmd_power_down(int argc, char **argv) {
    nrf24_power_down(g_cmd_nrf24);
}

static void subcmd_clear(int argc, char **argv) { nrf24_clear_all(g_cmd_nrf24); }

static void subcmd_radio_on(int argc, char **argv) { nrf24_radio_on(g_cmd_nrf24); }

static void subcmd_radio_off(int argc, char **argv) {
    nrf24_radio_off(g_cmd_nrf24);
}
static void subcmd_toggle_role(int argc, char **argv) {
    nrf24_role_switch(g_cmd_nrf24, nrf24_role_is_prx(g_cmd_nrf24) ? NRF24_ROLE_PTX
                                                            : NRF24_ROLE_PRX);
}

static void subcmd_read_rx_data(int argc, char **argv) {
    uint8_t buf[33];
    uint8_t len;
    uint8_t pipe;

    memset(buf, 0, 33);

    if (!nrf24_rxfifo_has_data(g_cmd_nrf24)) {
        PRINT("No data in RX FIFO\r\n");
        return;
    }

    nrf24_rxfifo_read(g_cmd_nrf24, buf, &len, &pipe);

    PRINT("(pipe%d) %d bytes: `%s`\n", pipe, len, (char *)buf);
    print_array(buf, len);
}

static void subcmd_set_tx_data(int argc, char **argv) {
    static uint32_t counter = 0;
    uint8_t buf[32];

    if (argc >= 1) {
        int rxpipe = strtol(argv[0], NULL, 10);
        if (!(rxpipe >= 0 && rxpipe < 6)) {
            PRINT("Invalid pipe number: %d\n", rxpipe);
            return;
        }

        nrf24_txfifo_set_prx_ackpipe(g_cmd_nrf24, rxpipe);
    }

    if (!nrf24_txfifo_has_space(g_cmd_nrf24)) {
        PRINT("No Free TX FIFO\n");
        return;
    }

    int len = sprintf((char *)buf, "This is %s %d",
                         nrf24_role_is_ptx(g_cmd_nrf24) ? "PTX" : "PRX", counter);

    nrf24_txfifo_write(g_cmd_nrf24, buf, len);

    counter++;
}

static void subcmd_set_tx_data_no_ack(int argc, char **argv) {
    static uint32_t counter = 0;
    uint8_t buf[32];

    if (nrf24_role_is_prx(g_cmd_nrf24)) {
        PRINT("For PTX only!\n");
        return;
    }

    if (!nrf24_txfifo_has_space(g_cmd_nrf24)) {
        PRINT("No Free TX FIFO\n");
        return;
    }

    int len = sprintf((char *)buf, "This is %s %d",
                         nrf24_role_is_ptx(g_cmd_nrf24) ? "PTX" : "PRX", counter);

    nrf24_txfifo_ptx_write_no_ack(g_cmd_nrf24, buf, len);

    counter++;
}

/**
 * @note Utilize the feature that there are three FIFOs
 * @note Will verify the received data
 */
static void subcmd_perf_test_halfduplex(int argc, char **argv) {
    const int HANDSHAKE_TIMEOUT = 10;
    const int PRX_TRANSFER_TIMEOUT = 1;
    int duration_s = PT_DEFAULT_DURATION;
    int payload_size = 32;

    if (argc >= 1) {
        duration_s = atoi(argv[0]);
    }
    if (argc >= 2) {
        payload_size = atoi(argv[1]);
        if (!((payload_size <= 32) && (payload_size >= 1))) {
            PRINT("invalid payload size\n");
            return;
        }
    }

    PRINT("Performance test (half-duplex) (%ds):\n", duration_s);
    PRINT("Role: %s\n", nrf24_role_is_ptx(g_cmd_nrf24) ? "PTX" : "PRX");
    PRINT("Payload size: %d bytes\n", payload_size);

    /***********/
    /* prepare */
    /***********/
    uint8_t txbuf[32];
    uint8_t rxbuf[32];
    uint8_t rxbuf_expect[32];

    memset(txbuf, 0x01, sizeof(txbuf));
    memset(rxbuf, 0x01, sizeof(rxbuf));
    memset(rxbuf_expect, 0x00, sizeof(rxbuf_expect));

    /* do reset */
    nrf24_radio_off(g_cmd_nrf24);
    TIME_WAIT_MS(10);
    nrf24_clear_all(g_cmd_nrf24);
    TIME_WAIT_MS(10);

    /* report rf info */
    {
        nrf24_user_cfg_t ucfg;
        nrf24_usercfg_read(g_cmd_nrf24, &ucfg);
        PRINT("RF: %dMHz %dMbps\n", 2400 + ucfg.rf_channel,
                   ucfg.rf_adr + 1);
    }

    /*************/
    /* handshake */
    /*************/
    uint32_t  start_ms = TIME_GET_MS();
    PRINT("Handshake...\n");
    // set tx data
    nrf24_txfifo_write(g_cmd_nrf24, txbuf, payload_size);
    TIME_WAIT_MS(10);
    nrf24_radio_on(g_cmd_nrf24);
    if (nrf24_role_is_prx(g_cmd_nrf24)) {
        // wait for rx done or timeout
        while (1) {
            TIME_WAIT_MS(1);
            int result =
                nrf24_status_routine(g_cmd_nrf24, nrf24_read_and_clear_status(g_cmd_nrf24));
            if (result & NRF24_STA_HAS_RXDATA) {
                PRINT("rx ok\n");
                break;
            }
            if (TIME_GET_MS() - start_ms >
                (HANDSHAKE_TIMEOUT * 1000)) {
                PRINT("timeout, abort\n");
                nrf24_clear_all(g_cmd_nrf24);
                return;
            }
        }
        // complete
        uint8_t rxlen = 0;
        nrf24_rxfifo_read(g_cmd_nrf24, rxbuf_expect, &rxlen, 0);
        nrf24_clear_all(g_cmd_nrf24);
        PRINT("handshake complete.\n");
    } else {
        // waitting for tx-rx done or timeout
        while (1) {
            TIME_WAIT_MS(50);
            if ((TIME_GET_MS() - start_ms) >
                (HANDSHAKE_TIMEOUT * 1000)) {
                PRINT("timeout, abort\n");
                return;
            }

            uint8_t sta = nrf24_read_status(g_cmd_nrf24);
            if (sta & REG_STATUS_BITMASK_RX_DR) {
                uint8_t rxlen = 0;
                nrf24_rxfifo_read(g_cmd_nrf24, rxbuf_expect, &rxlen, 0);
                if (memcmp(rxbuf_expect, txbuf, rxlen) != 0) {
                    PRINT("data mismatch, redo\n");
                    nrf24_clear_all(g_cmd_nrf24);
                    nrf24_txfifo_write(g_cmd_nrf24, txbuf, payload_size);
                    continue;
                }

                PRINT("tx-rx ok\n");
                break;
            }

            if (sta & REG_STATUS_BITMASK_TX_DS) {
                nrf24_txfifo_write(g_cmd_nrf24, txbuf, payload_size);
            }

            if (sta & REG_STATUS_BITMASK_MAX_RT) {
                PRINT("max retry reached, redo\n");
                TIME_WAIT_MS(500);
                nrf24_clear_all(g_cmd_nrf24);
                nrf24_txfifo_write(g_cmd_nrf24, txbuf, payload_size);
            }
        }

        nrf24_clear_all(g_cmd_nrf24);
        PRINT("handshake complete.\n");
        TIME_WAIT_MS(10); // give PRX more time to get ready
    }

    /************/
    /* transfer */
    /************/
    uint32_t loopcnt = 0;
    uint32_t txcnt = 0;
    uint32_t rxcnt = 0;
    uint32_t record_max_rt = 0;
    uint32_t record_sum_rt = 0;
    uint32_t record_sum_pl = 0;
    uint32_t txdscnt = 0;

    PRINT("IO...\n");
    uint32_t  transfer_begin_ms = TIME_GET_MS();

    txcnt++;
    increment_u8arr_content(txbuf, payload_size);
    nrf24_txfifo_write(g_cmd_nrf24, txbuf, payload_size);

    if (nrf24_role_is_prx(g_cmd_nrf24)) {
        uint32_t  tran_last_tick_ms = TIME_GET_MS();

        while (1) {
            /* timeout logic since last transaction */
            if (TIME_GET_MS() - tran_last_tick_ms >
                (PRX_TRANSFER_TIMEOUT * 1000)) {
                PRINT("time is up\n");
                nrf24_clear_all(g_cmd_nrf24);
                break;
            }

            int result =
                nrf24_status_routine(g_cmd_nrf24, nrf24_read_and_clear_status(g_cmd_nrf24));
            if (result == 0) {
                continue;
            }

            tran_last_tick_ms = TIME_GET_MS();

            if (result & NRF24_STA_HAS_RXDATA) {
                uint8_t rxlen = 0;
                nrf24_rxfifo_read(g_cmd_nrf24, rxbuf, &rxlen, 0);
                if (rxlen != payload_size) {
                    PRINT("fatal: unexpected rx-data length %d \n", rxlen);
                    return;
                }
                increment_u8arr_content(rxbuf_expect, payload_size);
                if (memcmp(rxbuf, rxbuf_expect, payload_size) != 0) {
                    PRINT("fatal: unexpected rx-data content \n");
                    print_array(rxbuf_expect, 32);
                    print_array(rxbuf, 32);
                    return;
                }
                if (nrf24_rxfifo_has_data(g_cmd_nrf24)) {
                    PRINT("warning: rx-fifo still has data\n");
                }
                rxcnt++;
            }
            if (result & NRF24_STA_TX_SENT) {
                txdscnt++;
            }

#ifdef PT_UTILIZE_ALL_FIFOS
            /* fill up the TX FIFO */
            while (nrf24_txfifo_has_space(g_cmd_nrf24)) {
                txcnt++;
                increment_u8arr_content(txbuf, payload_size);
                nrf24_txfifo_write(g_cmd_nrf24, txbuf, payload_size);
            }
#else
            if (nrf24_txfifo_is_empty(g_cmd_nrf24)) {
                txcnt++;
                increment_u8arr_content(txbuf, payload_size);
                nrf24_txfifo_write(g_cmd_nrf24, txbuf, payload_size);
            }
#endif

            loopcnt++;
        }

    } else {
        while (1) {

            if (TIME_GET_MS() - transfer_begin_ms >
                (duration_s * 1000)) {
                PRINT("time is up\n");

                int need_exit = 0;

                do {
                    int result = nrf24_status_routine(
                        g_cmd_nrf24, nrf24_read_and_clear_status(g_cmd_nrf24));
                    if (result == 0) {
                        if (need_exit) {
                            break;
                        }
                        continue;
                    }
                    if (result & NRF24_STA_TX_SENT) {
                        txdscnt++;
                    }
                    if (result & NRF24_STA_HAS_RXDATA) {
                        rxcnt++;
                        nrf24_rxfifo_flush(g_cmd_nrf24);
                    }
                    if (result == NRF24_STA_TX_FAIL) {
                        PRINT("the last txdata failed\n");
                        break;
                    }
                    if (need_exit) {
                        break;
                    }
                    if (!nrf24_txfifo_is_empty(g_cmd_nrf24)) {
                        continue;
                    }
                    need_exit = 1;
                    TIME_WAIT_MS(10);
                } while (1);
                PRINT("time is up\n");
                break;
            }

            int result =
                nrf24_status_routine(g_cmd_nrf24, nrf24_read_and_clear_status(g_cmd_nrf24));
            if (result == 0) {

                continue;
            }

            if (result == NRF24_STA_TX_FAIL) {
                record_sum_pl++;
                PRINT("TX FAIL happen, retry, (%d)\n", loopcnt);
                nrf24_clear_txfail_flag(g_cmd_nrf24);
            }

            if (result & NRF24_STA_HAS_RXDATA) {
                uint8_t rxlen = 0;
                nrf24_rxfifo_read(g_cmd_nrf24, rxbuf, &rxlen, 0);
                if (rxlen != payload_size) {
                    PRINT("fatal: unexpected rx-data length %d \n", rxlen);
                    return;
                }
                increment_u8arr_content(rxbuf_expect, payload_size);
                if (memcmp(rxbuf, rxbuf_expect, payload_size) != 0) {
                    PRINT("fatal: unexpected rx-data content \n");
                    print_array(rxbuf_expect, 32);
                    print_array(rxbuf, 32);
                    return;
                }
                if (nrf24_rxfifo_has_data(g_cmd_nrf24)) {
                    PRINT("warning: rx-fifo still has data\n");
                }
                rxcnt++;
            }

            if (result & NRF24_STA_TX_SENT) {
                txdscnt++;

                uint8_t obt = nrf24_read_reg(g_cmd_nrf24, NRF24_REG_OBSERVE_TX);
                int rt = obt & 0x0F;
                record_max_rt = MAX(record_max_rt, rt);
                record_sum_rt += rt;
            }

#ifdef PT_UTILIZE_ALL_FIFOS
            while (nrf24_txfifo_has_space(g_cmd_nrf24)) {
                txcnt++;
                increment_u8arr_content(txbuf, payload_size);
                nrf24_txfifo_write(g_cmd_nrf24, txbuf, payload_size);
            }
#else
            if (nrf24_txfifo_is_empty(g_cmd_nrf24)) {
                txcnt++;
                increment_u8arr_content(txbuf, payload_size);
                nrf24_txfifo_write(g_cmd_nrf24, txbuf, payload_size);
            }
#endif

            loopcnt++;
        }
    }
    uint32_t  transfer_end_ms = TIME_GET_MS();
    uint32_t  duration_ms = transfer_end_ms - transfer_begin_ms;
    if (nrf24_role_is_prx(g_cmd_nrf24)) {
        duration_ms -= PRX_TRANSFER_TIMEOUT * 1000;
    }
    PRINT("io complete.\n");

    /* */
    nrf24_clear_all(g_cmd_nrf24);

    /***********/
    /* summary */
    /***********/
    PRINT("Summary:\n");
    PRINT("\t%s\n", nrf24_role_is_prx(g_cmd_nrf24) ? "PRX" : "PTX");
    PRINT("\tduration: %d s\n", duration_ms / 1000);
    PRINT("\tpayload-size: %d bytes\n", payload_size);
    PRINT("\tloop: %d, txcnt:%d, txdscnt: %d, rxcnt:%d, \n", loopcnt,
               txcnt, txdscnt, rxcnt);
    PRINT("\ttx: %d bytes, %d bps\n", txcnt * payload_size,
               (txcnt * payload_size / (duration_ms / 1000)) * 8);
    PRINT("\trx: %d bytes, %d bps\n", rxcnt * payload_size,
               (rxcnt * payload_size / (duration_ms / 1000)) * 8);

    if (nrf24_role_is_ptx(g_cmd_nrf24)) {
        PRINT("\trt: %d, %d (max, sum)\n", record_max_rt, record_sum_rt);
        PRINT("\tpackets lost: %d\n", record_sum_pl);
    }
}

/**
 * @note Utilize the feature that there are three FIFOs
 * @note Will verify the received data
 */
static void subcmd_perf_test_simplex(int argc, char **argv) {
    const int HANDSHAKE_TIMEOUT = 10;
    const int PRX_TRANSFER_TIMEOUT = 1;
    int duration_s = PT_DEFAULT_DURATION;
    int payload_size = 32;

    if (argc >= 1) {
        duration_s = atoi(argv[0]);
    }
    if (argc >= 2) {
        payload_size = atoi(argv[1]);
        if (!((payload_size <= 32) && (payload_size >= 1))) {
            PRINT("invalid payload size\n");
            return;
        }
    }

    PRINT("Performance testing (simplex) (%ds):\n", duration_s);
    PRINT("Role: %s\n", nrf24_role_is_ptx(g_cmd_nrf24) ? "PTX" : "PRX");
    PRINT("Payload size: %d bytes\n", payload_size);

    /* prepare */
    uint8_t txbuf[32];
    uint8_t rxbuf[32];
    uint8_t rxbuf_expect[32];

    memset(txbuf, 0x01, sizeof(txbuf));
    memset(rxbuf, 0x01, sizeof(rxbuf));
    memset(rxbuf_expect, 0x00, sizeof(rxbuf_expect));

    // do reset
    nrf24_radio_off(g_cmd_nrf24);
    TIME_WAIT_MS(10);
    nrf24_clear_all(g_cmd_nrf24);
    TIME_WAIT_MS(10);

    // report rf info
    {
        nrf24_user_cfg_t ucfg;
        nrf24_usercfg_read(g_cmd_nrf24, &ucfg);
        PRINT("RF: %dMHz %dMbps\n", 2400 + ucfg.rf_channel,
                   ucfg.rf_adr + 1);
    }
    /*************/
    /* handshake */
    /*************/
    uint32_t  start_ms = TIME_GET_MS();
    PRINT("Handshake...\n");
    // set tx data
    nrf24_txfifo_write(g_cmd_nrf24, txbuf, payload_size);
    TIME_WAIT_MS(10);
    nrf24_radio_on(g_cmd_nrf24);
    if (nrf24_role_is_prx(g_cmd_nrf24)) {
        // wait for rx done or timeout
        while (1) {
            TIME_WAIT_MS(100);
            int result =
                nrf24_status_routine(g_cmd_nrf24, nrf24_read_and_clear_status(g_cmd_nrf24));
            if (result & NRF24_STA_HAS_RXDATA) {
                PRINT("tx done\n");
                break;
            }
            if ((TIME_GET_MS() - start_ms) >
                (HANDSHAKE_TIMEOUT * 1000)) {
                PRINT("timeout, abort\n");
                nrf24_clear_all(g_cmd_nrf24);
                return;
            }
        }
        // complete
        uint8_t rxlen = 0;
        nrf24_rxfifo_read(g_cmd_nrf24, rxbuf_expect, &rxlen, 0);
        nrf24_clear_all(g_cmd_nrf24);
        PRINT("handshake complete.\n");
    } else {
        // wait for tx-rx done or timeout
        while (1) {
            TIME_WAIT_MS(100);
            if ((TIME_GET_MS() - start_ms) >
                (HANDSHAKE_TIMEOUT * 1000)) {
                PRINT("timeout, abort\n");
                return;
            }

            uint8_t sta = nrf24_read_status(g_cmd_nrf24);
            if (sta & REG_STATUS_BITMASK_RX_DR) {
                uint8_t rxlen = 0;
                nrf24_rxfifo_read(g_cmd_nrf24, rxbuf_expect, &rxlen, 0);
                if (memcmp(rxbuf_expect, txbuf, rxlen) != 0) {
                    PRINT("data mismatch, redo\n");
                    nrf24_clear_all(g_cmd_nrf24);
                    nrf24_txfifo_write(g_cmd_nrf24, txbuf, payload_size);
                    continue;
                }

                PRINT("tx-rx done\n");
                break;
            }

            if (sta & REG_STATUS_BITMASK_TX_DS) {
                nrf24_txfifo_write(g_cmd_nrf24, txbuf, payload_size);
            }

            if (sta & REG_STATUS_BITMASK_MAX_RT) {
                PRINT("max retry reached, redo\n");
                TIME_WAIT_MS(500);
                nrf24_clear_all(g_cmd_nrf24);
                nrf24_txfifo_write(g_cmd_nrf24, txbuf, payload_size);
            }
        }

        // complete
        nrf24_clear_all(g_cmd_nrf24);
        PRINT("handshake complete.\n");
        TIME_WAIT_MS(10); // give PRX more time to get ready
    }

    /************/
    /* transfer */
    /************/
    uint32_t loopcnt = 0;
    uint32_t txcnt = 0;
    uint32_t rxcnt = 0;
    uint32_t record_max_rt = 0;
    uint32_t record_sum_rt = 0;
    uint32_t record_sum_pl = 0;
    uint32_t txdscnt = 0;

    PRINT("IO...\n");
    uint32_t  transfer_begin_ms = TIME_GET_MS();

    if (nrf24_role_is_prx(g_cmd_nrf24)) {
        uint32_t  tran_last_tick_ms = TIME_GET_MS();
        while (1) {
            /* timeout logic since last transaction */
            if (TIME_GET_MS() - tran_last_tick_ms >
                (PRX_TRANSFER_TIMEOUT * 1000)) {
                PRINT("time is up\n");
                nrf24_clear_all(g_cmd_nrf24);
                break;
            }

            int result =
                nrf24_status_routine(g_cmd_nrf24, nrf24_read_and_clear_status(g_cmd_nrf24));
            if (result == 0) {
                continue;
            }

            tran_last_tick_ms = TIME_GET_MS();

            if (result & NRF24_STA_HAS_RXDATA) {
                increment_u8arr_content(rxbuf_expect, payload_size);
                uint8_t rxlen = 0;
                nrf24_rxfifo_read(g_cmd_nrf24, rxbuf, &rxlen, 0);
                if (rxlen != payload_size) {
                    PRINT("fatal: unexpected rx-data length %d \n", rxlen);
                    return;
                }
                if (memcmp(rxbuf, rxbuf_expect, payload_size) != 0) {
                    PRINT("fatal: unexpected rx-data content \n");
                    print_array(rxbuf_expect, 32);
                    print_array(rxbuf, 32);
                    return;
                }
                if (nrf24_rxfifo_has_data(g_cmd_nrf24)) {
                    PRINT("warning: rx-fifo still has data\n");
                }
                rxcnt++;
            }

            loopcnt++;
        }

    } else {
        txcnt++;
        increment_u8arr_content(txbuf, payload_size);
        nrf24_txfifo_write(g_cmd_nrf24, txbuf, payload_size);
        while (1) {

            if (TIME_GET_MS() - transfer_begin_ms >
                (duration_s * 1000)) {
                int need_exit = 0;
                do {
                    int result = nrf24_status_routine(
                        g_cmd_nrf24, nrf24_read_and_clear_status(g_cmd_nrf24));
                    if (result == 0) {
                        if (need_exit) {
                            break;
                        }
                        continue;
                    }
                    if (result & NRF24_STA_TX_SENT) {
                        txdscnt++;
                    }
                    if (result & NRF24_STA_HAS_RXDATA) {
                        rxcnt++;
                        nrf24_rxfifo_flush(g_cmd_nrf24);
                    }
                    if (result == NRF24_STA_TX_FAIL) {
                        PRINT("the last txdata failed\n");
                        break;
                    }
                    if (need_exit) {
                        break;
                    }
                    if (!nrf24_txfifo_is_empty(g_cmd_nrf24)) {
                        continue;
                    }
                    need_exit = 1;
                    TIME_WAIT_MS(5);
                } while (1);
                PRINT("time is up\n");
                break;
            }

            int result =
                nrf24_status_routine(g_cmd_nrf24, nrf24_read_and_clear_status(g_cmd_nrf24));
            if (result == 0) {
                continue;
            }

            if (result == NRF24_STA_TX_FAIL) {
                record_sum_pl++;
                PRINT("Warn: TX FAIL happen, retry, (%d)\n", loopcnt);
                nrf24_clear_txfail_flag(g_cmd_nrf24);
            }

            if (result & NRF24_STA_HAS_RXDATA) {
                rxcnt++;
                PRINT("Warn: Received data.\n");
                nrf24_rxfifo_flush(g_cmd_nrf24);
            }

            if (result & NRF24_STA_TX_SENT) {
                txdscnt++;

                uint8_t obt = nrf24_read_reg(g_cmd_nrf24, NRF24_REG_OBSERVE_TX);
                int rt = obt & 0x0F;
                record_max_rt = MAX(record_max_rt, rt);
                record_sum_rt += rt;
            }

#ifdef PT_UTILIZE_ALL_FIFOS
            /* fill up the TX FIFO */
            while (nrf24_txfifo_has_space(g_cmd_nrf24)) {
                txcnt++;
                increment_u8arr_content(txbuf, payload_size);
                nrf24_txfifo_write(g_cmd_nrf24, txbuf, payload_size);
            }
#else
            if (nrf24_txfifo_is_empty(g_cmd_nrf24)) {
                txcnt++;
                increment_u8arr_content(txbuf, payload_size);
                nrf24_txfifo_write(g_cmd_nrf24, txbuf, payload_size);
            }
#endif

            loopcnt++;
        }
    }
    uint32_t  transfer_end_ms = TIME_GET_MS();
    uint32_t  duration_ms = transfer_end_ms - transfer_begin_ms;
    if (nrf24_role_is_prx(g_cmd_nrf24)) {
        duration_ms -= PRX_TRANSFER_TIMEOUT * 1000;
    }
    PRINT("io complete.\n");

    /* */
    nrf24_clear_all(g_cmd_nrf24);

    /***********/
    /* summary */
    /***********/
    PRINT("Summary:\n");
    PRINT("\t%s\n", nrf24_role_is_prx(g_cmd_nrf24) ? "PRX" : "PTX");
    PRINT("\tduration: %d s\n", duration_ms / 1000);
    PRINT("\tpayload-size: %d bytes\n", payload_size);
    PRINT("\tloop: %d, txcnt:%d, txdscnt: %d, rxcnt:%d, \n", loopcnt,
               txcnt, txdscnt, rxcnt);
    PRINT("\ttx: %d bytes, %d bps\n", txcnt * payload_size,
               (txcnt * payload_size / (duration_ms / 1000)) * 8);
    PRINT("\trx: %d bytes, %d bps\n", rxcnt * payload_size,
               (rxcnt * payload_size / (duration_ms / 1000)) * 8);

    if (nrf24_role_is_ptx(g_cmd_nrf24)) {
        PRINT("\trt: %d, %d (max, sum)\n", record_max_rt, record_sum_rt);
        PRINT("\tpackets lost: %d\n", record_sum_pl);
    }
}

static nrf24_subcmd_t g_subcmds[] = {
    {"help", subcmd_help, "Show command help", "Usage: help <cmd>\n"},
    {"reg", subcmd_access_reg, "Access register",
     "Usage: reg <reg> [val]\n"
     "    Example: `reg 0x0E 0x07` write register (REG[0x0E]=0x07)\n"
     "    Example: `reg 0x0E` read register (REG[0x0E])\n"},
    {"cfg", subcmd_access_cfg, "Access config",
     "Usage: cfg [<item> ...]\n"
     "Items:\n"
     "    adr <bps>: set air-data-rate, 1 or 2\n"
     "    ch <channel>: set rf-channel, 0-125\n"
     "    power <level>: set rf-power, 0-3, 3 is the highest level\n"
     "    txaddr <byte1> [byte2 ... byte5]: set tx address\n"
     "    rp<x> <action>: x is 0-5\n"
     "          on: enable rx pipe\n"
     "          off: disable rx pipe\n"
     "          addr: set rx pipe address (note: pipe2~5 are 1 byte width "
     "address\n"
     "Example: `cfg` show all configs\n"
     "Example: `cfg adr 1` set 1Mbps air-data-rate\n"
     "Example: `cfg txaddr 0xE7 0xC7` set txaddr as 0xXXXXXXC7E7\n"},
    {"sta", subcmd_report_status, "Report status", "Usage: sta\n"},
    {"clear", subcmd_clear, "Clear all (status and data)", "Usage: clear\n"},
    {"dotx", subcmd_set_tx_data, "Write tx data (auto changed content)",
     "Usage: dotx [rxpipe]\n"
     "Note: rxpipe is optional and has memory"},
    {"dotx-noack", subcmd_set_tx_data_no_ack, "Write tx data with no-ack (auto changed content) (ptx only)"},
    {"dorx", subcmd_read_rx_data, "Read rx data (try)", "Usage: dorx\n"},
    {"powerup", subcmd_power_up, "Power up device", "Usage: powerup\n"},
    {"powerdown", subcmd_power_down, "Power down device", "Usage: powerdown\n"},
    {"radioon", subcmd_radio_on, "Radio on", "Usage: radioon\n"},
    {"radiooff", subcmd_radio_off, "Radio off", "Usage: radiooff\n"},
    {"toggle-role", subcmd_toggle_role, "Toggle role", "Usage: toggle-role\n"},
    {"pt-s", subcmd_perf_test_simplex, "Do performance test (simplex)",
     "Usage: pt-s [duration_s] [payload_size](1-32)\n"},
    {"pt-hd", subcmd_perf_test_halfduplex, "Do performance test (half-duplex)",
     "Usage: pt-hd [duration_s] [payload_size](1-32)\n"},
    USER_SUBCMDS
    {NULL, NULL, NULL, NULL}};

nrf24_subcmd_t *nrf24_subcmds = &g_subcmds[0];

static void subcmd_help(int argc, char **argv) {
    if (argc != 1) {
        PRINT("Usage: <cmd> ...\n");
        for (int i = 0; g_subcmds[i].name; i++) {
            PRINT("             %s: %s\n", g_subcmds[i].name, g_subcmds[i].brief);
        }
        return;
    }

    for (int i = 0; g_subcmds[i].name; i++) {
        if (strcmp(argv[0], g_subcmds[i].name) == 0) {
            const char *help_str = g_subcmds[i].help;
            const char *line_start = help_str;
            const char *line_end;

            // 逐行打印
            while ((line_end = strchr(line_start, '\n')) != NULL) {
                // 计算当前行长度
                size_t line_len = line_end - line_start;

                char line_buf[128]; //
                size_t copy_len = (line_len < sizeof(line_buf) - 1)
                                      ? line_len
                                      : sizeof(line_buf) - 1;

                memcpy(line_buf, line_start, copy_len);
                line_buf[copy_len] = '\0';

                PRINT("%s\n", line_buf);

                line_start = line_end + 1; // 下一行开始
            }

            // 打印最后一行（如果非空）
            if (*line_start != '\0') {
                PRINT("%s\n", line_start);
            }

            return;
        }
    }

    PRINT("No such command: %s\n", argv[0]);
}

void nrf24_cmd_set_cmd_device(nrf24_t *nrf24)
{
    g_cmd_nrf24 = nrf24;
}

nrf24_t *nrf24_cmd_get_cmd_device(void)
{
    return g_cmd_nrf24;
}

void nrf24_cmd_entry(int argc, char **argv) {
    if (argc == 1) {
        PRINT("Usage: %s <subcmd> ...\n", argv[0]);
        for (int i = 0; g_subcmds[i].name; i++) {
            PRINT("             %s: %s\n", g_subcmds[i].name,
                       g_subcmds[i].brief);
        }
        return;
    }

    char *subcmd = argv[1];

    for (int i = 0; g_subcmds[i].name; i++) {
        if (strcmp(subcmd, g_subcmds[i].name) == 0) {
            // no name arg
            g_subcmds[i].func(argc - 2, argv + 2);
            return;
        }
    }
}
