/*
 * Copyright (c) 2019, sogwyms@gmail.com
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2019-05-23     sogwms       the first version
 * 2020-02-02     sogwms       refactor to object-oriented and add irq support
 */

#include "drv_gpio.h"
#include "drv_spi.h"
#include "nrf24l01_port.h"

#define DBG_SECTION_NAME  "nrf24l01_port"
#define DBG_LEVEL         DBG_LOG
#include <rtdbg.h>

static int send_then_recv(hal_nrf24_port_t halport, const uint8_t *tbuf, uint8_t tlen, uint8_t *rbuf, uint8_t rlen)
{
    return rt_spi_send_then_recv(halport->dev, tbuf, tlen, rbuf, rlen);
}

static int send_then_send(hal_nrf24_port_t halport, const uint8_t *buf1, uint8_t len1, const uint8_t *buf2, uint8_t len2)
{
    return rt_spi_send_then_send(halport->dev, buf1, len1, buf2, len2);
}

static int write(hal_nrf24_port_t halport, const uint8_t *buf, uint8_t len)
{
    return rt_spi_send(halport->dev, buf, len);
}

static void set_ce(hal_nrf24_port_t halport)
{
    rt_pin_write(halport->ce_pin, PIN_HIGH);
}

static void reset_ce(hal_nrf24_port_t halport)
{
    rt_pin_write(halport->ce_pin, PIN_LOW);
}

static int read_irq_pin(hal_nrf24_port_t halport)
{
    return rt_pin_read(halport->irq_pin);
}

static void hdr(void *args)
{
    hal_nrf24_port_t halport = (hal_nrf24_port_t)args;
    halport->_irq_cb(halport);
}

const static struct hal_nrf24_port_ops g_hal_port_ops = {
    .send_then_recv = send_then_recv,
    .send_then_send = send_then_send,
    .write = write,
    .set_ce = set_ce,
    .reset_ce = reset_ce,
    .read_irq_pin = read_irq_pin,
};

int hal_nrf24_port_init(hal_nrf24_port_t halport, char *spi_dev_name, int ce_pin, int irq_pin, void(*irq_callback)(hal_nrf24_port_t halport))
{
    struct rt_spi_device *dev;
    struct rt_spi_configuration cfg;

    RT_ASSERT(halport != RT_NULL);

    dev = (struct rt_spi_device *)rt_device_find(spi_dev_name);
    if (!dev) {
        LOG_E("Can't find device on %s\n", spi_dev_name);
        return RT_ERROR;
    }

    halport->dev = dev;
    halport->ce_pin = ce_pin;
    halport->irq_pin = irq_pin;
    halport->_irq_cb = irq_callback;
    halport->ops = &g_hal_port_ops;

    cfg.data_width = 8;
    cfg.max_hz = 8 * 1000 * 1000;
    cfg.mode = RT_SPI_MASTER | RT_SPI_MSB | RT_SPI_MODE_0;
    rt_spi_configure(dev, &cfg);

    rt_pin_mode(ce_pin, PIN_MODE_OUTPUT);
    reset_ce(halport);

    if(irq_pin != NRF24_PIN_NONE && irq_callback != RT_NULL)
    {
        rt_pin_mode(irq_pin, PIN_MODE_INPUT_PULLUP);
        rt_pin_attach_irq(irq_pin, PIN_IRQ_MODE_FALLING, hdr, halport);
        rt_pin_irq_enable(irq_pin, PIN_IRQ_ENABLE);
    }

    return RT_EOK;
}

hal_nrf24_port_t hal_nrf24_port_create(char *spi_dev_name, int ce_pin, int irq_pin, void(*irq_callback)(hal_nrf24_port_t halport))
{
    struct hal_nrf24_port *halport;

    halport = (struct hal_nrf24_port *)rt_malloc(sizeof(struct hal_nrf24_port));
    if (halport == RT_NULL)
    {
        LOG_E("Failed to allocate memory!");
    }
    else
    {
        if (hal_nrf24_port_init(halport, spi_dev_name, ce_pin, irq_pin, irq_callback) != RT_EOK)
        {
            rt_free(halport);
            halport = RT_NULL;
        }
    }

    return halport;
}
