#include <rtthread.h>
#include <rtdevice.h>
#include "nrf24l01_dep_impl.h"

#ifdef NRF24L01_SUPPORT_AUTO_SPIDEV
#define DEMIMPL_SUPPORT_GEN_SPIDEV
#endif

static int ops_init(void *ctx) {
    struct nrf24_depimpl_ctx *p = (struct nrf24_depimpl_ctx *)ctx;

    rt_pin_mode(p->ce_pin, PIN_MODE_OUTPUT);
    rt_pin_write(p->ce_pin, 0);

#ifdef DEMIMPL_SUPPORT_GEN_SPIDEV

    if (p->need_gen_spidev)
    {
        rt_pin_mode(p->cs_pin, PIN_MODE_OUTPUT);
        rt_pin_write(p->cs_pin, 1);
        if (p->spi_bus != RT_NULL)
        {
            struct rt_spi_device *spi_device = RT_NULL;

            spi_device = (struct rt_spi_device *)rt_malloc(sizeof(struct rt_spi_device));
            if (spi_device == RT_NULL)
            {
                rt_kprintf("Failed to malloc the spi device.");
                return -RT_ENOMEM;
            }

/* Compatible for spi api variance betweent differrent kernel versions */
#if defined(RTTHREAD_VERSION) && (RTTHREAD_VERSION >= RT_VERSION_CHECK(5, 0, 0))
            if (rt_spi_bus_attach_device_cspin(spi_device, p->spi_dev_name, p->spi_bus, p->cs_pin, RT_NULL) != RT_EOK)
#else
            if (rt_spi_bus_attach_device(spi_device, p->spi_dev_name, p->spi_bus, &p->cs_pin)) 
#endif
            {
                rt_kprintf("Failed to attach the spi device.");
                return -RT_ERROR;
            }
        }
    }

#endif

    p->spi_dev_handle = (struct rt_spi_device *)rt_device_find(p->spi_dev_name);
    if (p->spi_dev_handle == RT_NULL) {
        rt_kprintf("Failed to find the spi device.");
        return -RT_ERROR;
    }

    struct rt_spi_configuration cfg;
    cfg.data_width = 8;
    cfg.max_hz = NRF24L01_SPI_MAX_SPEED_HZ;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MSB | RT_SPI_MODE_0;
    if (rt_spi_configure(p->spi_dev_handle, &cfg) != RT_EOK) {
        rt_kprintf("Failed to configure the spi device.");
        return -RT_ERROR;
    }

    rt_thread_mdelay(10);

    return 0;
}

static void ops_deinit(void *ctx) {
    struct nrf24_depimpl_ctx *p = (struct nrf24_depimpl_ctx *)ctx;

}

static void ops_set_ce_high(void *ctx) {
    struct nrf24_depimpl_ctx *p = (struct nrf24_depimpl_ctx *)ctx;
    rt_pin_write(p->ce_pin, 1);
}

static void ops_set_ce_low(void *ctx) {
    struct nrf24_depimpl_ctx *p = (struct nrf24_depimpl_ctx *)ctx;
    rt_pin_write(p->ce_pin, 0);
}

static int ops_spi_send(void *ctx, const uint8_t *buf, uint8_t len) {
    int ret = 0;
    struct nrf24_depimpl_ctx *p = (struct nrf24_depimpl_ctx *)ctx;

    rt_spi_send(p->spi_dev_handle, buf, len);

    return ret;
}

static int ops_spi_send_then_send(void *ctx, const uint8_t *buf1, uint8_t len1,
                                  const uint8_t *buf2, uint8_t len2) {
    int ret = 0;
    struct nrf24_depimpl_ctx *p = (struct nrf24_depimpl_ctx *)ctx;

    ret = rt_spi_send_then_send(p->spi_dev_handle, buf1, len1, buf2, len2);

    return ret;
}

static int ops_spi_send_then_recv(void *ctx, const uint8_t *wbuf, uint8_t wlen,
                                  uint8_t *rbuf, uint8_t rlen) {
    int ret = 0;
    struct nrf24_depimpl_ctx *p = (struct nrf24_depimpl_ctx *)ctx;

    ret = rt_spi_send_then_recv(p->spi_dev_handle, wbuf, wlen, rbuf, rlen);

    return ret;
}

static nrf24_dep_ops_t g_nrf24_depimpl_ops = {
    .init = ops_init,
    .deinit = ops_deinit,
    .spi_send = ops_spi_send,
    .spi_send_then_send = ops_spi_send_then_send,
    .spi_send_then_recv = ops_spi_send_then_recv,
    .set_ce_high = ops_set_ce_high,
    .set_ce_low = ops_set_ce_low,
};

nrf24_dep_ops_t *nrf24_depimpl_get_ops(void)  
{
    return &g_nrf24_depimpl_ops;
}

#ifdef DEMIMPL_SUPPORT_GEN_SPIDEV

void nrf24_depimpl_init_ctx(struct nrf24_depimpl_ctx *ctx, char *spi_dev_name, int ce_pin, char *spi_bus, int cs_pin)
{
    ctx->need_gen_spidev = 1;
    ctx->spi_bus = spi_bus;
    ctx->spi_dev_name = spi_dev_name;
    ctx->cs_pin = cs_pin;
    ctx->ce_pin = ce_pin;
    ctx->spi_dev_handle = RT_NULL;
}

#endif

void nrf24_depimpl_init_ctx_with_spidev(struct nrf24_depimpl_ctx *ctx, char *spi_dev_name,int ce_pin)
{
    ctx->need_gen_spidev = 0;
    ctx->spi_bus = 0;
    ctx->cs_pin = 0;
    ctx->spi_dev_name = spi_dev_name;
    ctx->ce_pin = ce_pin;
    ctx->spi_dev_handle = RT_NULL;
}
