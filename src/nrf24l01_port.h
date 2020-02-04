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
#ifndef __NRF24L01_PORT_H__
#define __NRF24L01_PORT_H__

#include <stdint.h>
#include <rtdevice.h>

#define NRF24_PIN_NONE          -1

#define SUB_HALPORT_WIRTE(_nrf24, buf, len)                             _nrf24->halport.ops->write(&_nrf24->halport, buf, len)
#define SUB_HALPORT_SEND_THEN_RECV(_nrf24, tbuf, tlen, rbuf, rlen)      _nrf24->halport.ops->send_then_recv(&_nrf24->halport, tbuf, tlen, rbuf, rlen)
#define SUB_HALPORT_SEND_THEN_SEND(_nrf24, buf1, len1, buf2, len2)      _nrf24->halport.ops->send_then_send(&_nrf24->halport, buf1, len1, buf2, len2)
#define SUB_HALPORT_RESET_CE(_nrf24)                                    _nrf24->halport.ops->reset_ce(&_nrf24->halport)
#define SUB_HALPORT_SET_CE(_nrf24)                                      _nrf24->halport.ops->set_ce(&_nrf24->halport)

#define NRF24_HALPORT_WRITE(buf, len)                                   SUB_HALPORT_WIRTE(nrf24, buf, len)
#define NRF24_HALPORT_SEND_THEN_RECV(tbuf, tlen, rbuf, rlen)            SUB_HALPORT_SEND_THEN_RECV(nrf24, tbuf, tlen, rbuf, rlen)
#define NRF24_HALPORT_SEND_THEN_SEND(buf1, len1, buf2, len2)            SUB_HALPORT_SEND_THEN_SEND(nrf24, buf1, len1, buf2, len2)
#define NRF24_HALPORT_RESET_CE()                                        SUB_HALPORT_RESET_CE(nrf24)
#define NRF24_HALPORT_SET_CE()                                          SUB_HALPORT_SET_CE(nrf24)

typedef struct hal_nrf24_port *hal_nrf24_port_t;

struct hal_nrf24_port
{
    const struct hal_nrf24_port_ops *ops;

    int ce_pin;
    int irq_pin;
    void(*_irq_cb)(struct hal_nrf24_port *halport);
    struct rt_spi_device *dev;
};

struct hal_nrf24_port_ops
{
    int (*send_then_recv)(struct hal_nrf24_port *halport, const uint8_t *tbuf, uint8_t tlen, uint8_t *rbuf, uint8_t rlen);
    int (*send_then_send)(struct hal_nrf24_port *halport, const uint8_t *buf1, uint8_t len1, const uint8_t *buf2, uint8_t len2);
    int (*write)(struct hal_nrf24_port *halport, const uint8_t *buf, uint8_t len);
    void (*set_ce)(struct hal_nrf24_port *halport);
    void (*reset_ce)(struct hal_nrf24_port *halport);

    int (*read_irq_pin)(struct hal_nrf24_port *halport);
};

int hal_nrf24_port_init(hal_nrf24_port_t halport, char *spi_dev_name, int ce_pin, int irq_pin, void(*irq_callback)(hal_nrf24_port_t halport));
hal_nrf24_port_t hal_nrf24_port_create(char *spi_dev_name, int ce_pin, int irq_pin, void(*irq_callback)(hal_nrf24_port_t halport));

#endif // __NRF24L01_PORT_H__
