#include <rtdevice.h>
#include <rtthread.h>
#include <string.h>
#include <nrf24l01.h>
#include <nrf24l01_dep_impl.h>

#define PRINT rt_kprintf
#define TIME_GET_MS rt_tick_get_millisecond
#define TIME_WAIT_MS rt_thread_mdelay

#include "nrf24_cmd_core.inc.c"

#ifndef NRF24L01_COMMAND_MANUAL_INIT

extern nrf24_t g_nrf24;

static int init_nrf24_cmd(void)
{
    nrf24_cmd_set_cmd_device(&g_nrf24);
    return 0;
}

INIT_APP_EXPORT(init_nrf24_cmd);

#endif

MSH_CMD_EXPORT_ALIAS(nrf24_cmd_entry, nrf24, nrf24 utility);
