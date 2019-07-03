#include <rtthread.h>
#include "nrf24l01.h"

#include "sample.h"

static rt_sem_t nrfirq_sem;

static void _irq_init(void);
static void _waitirq(void);

void sample_nrf24l01_task(void *param)
{
    struct hal_nrf24l01_port_cfg halcfg;
    nrf24_cfg_t cfg;
    int rlen;
    uint8_t rbuf[32 + 1];
    uint8_t tbuf[32] = "first\r\n";
    uint32_t cnt = 0;

    _irq_init();

    nrf24_default_param(&cfg);
    halcfg.ce_pin = NRF24L01_CE_PIN;
    halcfg.spi_device_name = NRF24L01_SPI_DEVICE;    
    cfg.role = ROLE_PTX; /* PTX */
    cfg.ud = &halcfg;
    cfg.use_irq = 1;     /* True */    
    nrf24_init(&cfg);

    while (1) {
        rlen = nrf24_irq_ptx_run(rbuf, tbuf, rt_strlen((char *)tbuf), _waitirq);
        /* sent successfully and received data */
        if (rlen > 0)
        {
            rbuf[rlen] = '\0';
            rt_kputs((char *)rbuf);
            
            rt_sprintf((char *)tbuf, "i-am-PTX:%dth\r\n", cnt);
            cnt++;
        }
        else if (rlen == 0)
        /* sent successfully but no data received */
        {
            rt_sprintf((char *)tbuf, "i-am-PTX:%dth\r\n", cnt);
            cnt++;
        }
        /* sent failed */
        else
        {
            rt_kputs("send failed\r\n");            
        }
    }
}

static void _irq_cb(void *param)
{
    rt_sem_release(nrfirq_sem);
}

static void _irq_init(void)
{
    nrfirq_sem = rt_sem_create("nrfIRQ", 0, RT_IPC_FLAG_FIFO);

    rt_pin_attach_irq(NRF24_IRQ_PIN, PIN_IRQ_MODE_FALLING, _irq_cb, 0);
    rt_pin_irq_enable(NRF24_IRQ_PIN, PIN_IRQ_ENABLE);
}

static void _waitirq(void)
{
    rt_sem_take(nrfirq_sem, RT_WAITING_FOREVER);
}

static int nrf24l01_sample_init(void)
{
    rt_thread_t thread;

    thread = rt_thread_create("samNrfPTX", sample_nrf24l01_task, RT_NULL, 512, RT_THREAD_PRIORITY_MAX/2, 20);
    if (thread != RT_NULL)
    {
        rt_thread_startup(thread);
    }

    return RT_EOK;
}
INIT_APP_EXPORT(nrf24l01_sample_init);
