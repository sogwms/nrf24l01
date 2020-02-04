#include <nrf24l01.h>
#include <rtconfig.h>

#if (PKG_NRF24L01_DEMO_CE_PIN < 0)
#error Please specify a valid pin
#endif

#ifdef PKG_NRF24L01_DEMO_ROLE_PTX
    #define NRF24_DEMO_ROLE     ROLE_PTX
    #define NRF24_DEMO_SEND_INTERVAL        PKG_NRF24L01_DEMO_INTERVAL_SEND
#else
    #define NRF24_DEMO_ROLE     ROLE_PRX
#endif

#define NRF24_DEMO_SPI_DEV_NAME         PKG_NRF24L01_DEMO_SPI_DEV_NAME
#define NRF24_DEMO_CE_PIN               PKG_NRF24L01_DEMO_CE_PIN
#define NRF24_DEMO_IRQ_PIN              PKG_NRF24L01_DEMO_IRQ_PIN

const static char *ROLE_TABLE[] = {"PTX", "PRX"};

static void rx_ind(nrf24_t nrf24, uint8_t *data, uint8_t len, int pipe)
{
    /*! Don't need to care the pipe if the role is ROLE_PTX */
    rt_kprintf("(p%d): ", pipe);
    rt_kprintf((char *)data);
}

static void tx_done(nrf24_t nrf24, int pipe)
{
    static int cnt = 0;
    static char tbuf[32];

    cnt++;

    /*! Here just want to tell the user when the role is ROLE_PTX
    the pipe have no special meaning except indicating (send) FAILED or OK 
        However, it will matter when the role is ROLE_PRX*/
    if (nrf24->cfg.role == ROLE_PTX)
    {
        if (pipe == NRF24_PIPE_NONE)
            rt_kprintf("tx_done failed");
        else
            rt_kprintf("tx_done ok");
    }
    else
    {
        rt_kprintf("tx_done ok");
    }

    rt_kprintf(" (pipe%d)\n", pipe);
    
    rt_sprintf(tbuf, "My role is %s [%dth]\n", ROLE_TABLE[nrf24->cfg.role], cnt);
    nrf24_send_data(nrf24, (uint8_t *)tbuf, rt_strlen(tbuf), pipe);
#ifdef PKG_NRF24L01_DEMO_ROLE_PTX
    rt_thread_mdelay(NRF24_DEMO_SEND_INTERVAL);
#endif
}

const static struct nrf24_callback _cb = {
    .rx_ind = rx_ind,
    .tx_done = tx_done,
};

static void thread_entry(void *param)
{
    nrf24_t nrf24;

    rt_kprintf("[nrf24/demo] Version:%s\n", PKG_NRF24L01_VERSION);

    nrf24 = nrf24_default_create(NRF24_DEMO_SPI_DEV_NAME, NRF24_DEMO_CE_PIN, NRF24_DEMO_IRQ_PIN, &_cb, NRF24_DEMO_ROLE);

    if (nrf24 == RT_NULL)
    {
        rt_kprintf("\n[nrf24/demo] Failed to create nrf24. stop!\n");
        for(;;) rt_thread_mdelay(10000);
    }
    else
    {
        rt_kprintf("[nrf24/demo] running.");
    }

    nrf24_send_data(nrf24, "Hi\n", 3, NRF24_DEFAULT_PIPE);

    while (1)
    {
        nrf24_run(nrf24);

        if(!nrf24->flags.using_irq)
            rt_thread_mdelay(10);
    }
    
}

static int nrf24l01_sample_init(void)
{
    rt_thread_t thread;

    thread = rt_thread_create("nrfDemo", thread_entry, RT_NULL, 1024, RT_THREAD_PRIORITY_MAX/2, 20);
    rt_thread_startup(thread);

    return RT_EOK;
}

INIT_APP_EXPORT(nrf24l01_sample_init);
