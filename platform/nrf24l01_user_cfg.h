#include <rtthread.h>

/* set log sink */
#define NRF24L01_LOG_SINK(level, fmt, ...) rt_kprintf("[nrf24l01][%c]: " fmt "\r\n", level, ##__VA_ARGS__)

/* config log level */
#define NRF24L01_LOG_MIN_OUTPUT_LEVEL 'I'

/* declare using Si24R1 */
// #define NRF24L01_Si24R1_DEVICE