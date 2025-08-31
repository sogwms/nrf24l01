#define DEMONAME prx_irq

static void DEMONAME(void);

#include "nrf24_demo_main.c.inc"

#define UTILIZE_ALL_FIFOS

#define NRF24_IRQ_PIN PKG_NRF24L01_DEMO_HAL_IRQ_PIN
static rt_sem_t g_nrf24_irq_sem;

static void nrf24_irq_handler(void *arg) { rt_sem_release(g_nrf24_irq_sem); }

static void DEMONAME(void)
{
    int irqpin = rt_pin_get(NRF24_IRQ_PIN);
    rt_pin_mode(irqpin, PIN_MODE_INPUT_PULLUP);
    rt_pin_attach_irq(irqpin, PIN_IRQ_MODE_FALLING, nrf24_irq_handler, &g_nrf24);
    rt_pin_irq_enable(irqpin, PIN_IRQ_ENABLE);
    g_nrf24_irq_sem = rt_sem_create("nrf24irq", 0, RT_IPC_FLAG_FIFO);

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


        /* Note: you can put `nrf24_read_and_clear_status(&g_nrf24)` in irq handler for faster response */
        int result =
            nrf24_status_routine(&g_nrf24, nrf24_read_and_clear_status(&g_nrf24));
        if (result == 0)
        {
            rt_sem_take(g_nrf24_irq_sem, RT_WAITING_FOREVER);
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
