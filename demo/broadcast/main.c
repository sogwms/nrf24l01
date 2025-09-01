#define DEMONAME broadcast

static void DEMONAME(void);

#include "nrf24_demo_main.inc.c"

static void DEMONAME(void)
{
    uint8_t txbuf[32];
    uint32_t txcnt = 0;

    nrf24_setup(&g_nrf24, NRF24_ROLE_PTX); 

    /* Start broadcasting */
    while (1) {
        /* ensure having free txfifo */
        if(nrf24_txfifo_has_space(&g_nrf24)) {
            /* write data to txfifo with no-ack set */
            txcnt++;
            nrf24_txfifo_ptx_write_no_ack(&g_nrf24, txbuf, rt_snprintf(txbuf, sizeof(txbuf), "This is a broadcast (%d)\n", txcnt));
        }

        rt_thread_mdelay(5);
    }
}
