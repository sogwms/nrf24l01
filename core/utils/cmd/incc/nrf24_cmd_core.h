/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Copyright sogwms
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2025-07-26     sogwms       first version       
 */

#ifndef NRF24_CMD_H
#define NRF24_CMD_H

#include "nrf24l01.h"

typedef struct {
    const char *name;
    void (*func)(int argc, char **argv);
    const char *brief;
    const char *help;
}nrf24_subcmd_t;

extern nrf24_subcmd_t *nrf24_subcmds;

void nrf24_cmd_set_cmd_device(nrf24_t *nrf24);
nrf24_t *nrf24_cmd_get_cmd_device(void);
void nrf24_cmd_entry(int argc, char **argv);

#endif
