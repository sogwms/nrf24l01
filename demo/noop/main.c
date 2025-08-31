#define DEMONAME noop

static void DEMONAME(void);

#include "nrf24_demo_main.c.inc"

static void DEMONAME(void)
{
    nrf24_setup(&g_nrf24, NRF24_ROLE_PRX); 

    while (1) {
        rt_thread_mdelay(10000);
    }
}
