#define DEMONAME prx

static void DEMONAME(void);

#include "nrf24_demo_main.c.inc"

#define UTILIZE_ALL_FIFOS

static void DEMONAME(void)
{
    nrf24_setup(&g_nrf24, NRF24_ROLE_PRX); 
    
    uint8_t pipe;
    uint8_t rxlen;
    uint8_t rxbuf[33];
    uint32_t txcnt = 0;
    uint8_t txbuf[32] = {0};
    while (1)
    {
#ifdef UTILIZE_ALL_FIFOS
        /* fill up the TX FIFO */
        while (nrf24_txfifo_has_space(&g_nrf24))
        {
            txcnt++;
            char actual_size = rt_snprintf(txbuf, sizeof(txbuf), "This is %s (%d)\n", DEMONAME_STR, txcnt);
            nrf24_txfifo_write(&g_nrf24, txbuf, actual_size);
        }
#else
        if (nrf24_txfifo_is_empty(&g_nrf24))
        {
            txcnt++;
            char actual_size = rt_snprintf(txbuf, sizeof(txbuf), "This is %s (%d)\n", DEMONAME_STR, txcnt);
            nrf24_txfifo_write(&g_nrf24, txbuf, actual_size);
        }
#endif

        int result =
            nrf24_status_routine(&g_nrf24, nrf24_read_and_clear_status(&g_nrf24));
        if (result == 0)
        {
            rt_thread_mdelay(5);
            continue;
        }

        if (result & NRF24_STA_HAS_RXDATA)
        {
            uint8_t rxlen = 0;
            nrf24_rxfifo_read(&g_nrf24, rxbuf, &rxlen, 0);
            rxbuf[rxlen] = '\0';
            rt_kprintf("rxdata (pipe %d) (%d bytes) : %s\n", pipe, rxlen, rxbuf);
        }
    }
}
