#define DEMONAME listening

static void DEMONAME(void);

#include "nrf24_demo_main.c.inc"

static void DEMONAME(void)
{
    nrf24_user_cfg_t ucfg;
    nrf24_usercfg_init_default(&ucfg);
    ucfg.rxpipes[0].enable = 1; // enable pipe
    ucfg.rxpipes[0].enable_aa = 0; // disable auto ack
    nrf24_setup_full(&g_nrf24, NRF24_ROLE_PRX, &ucfg, nrf24_default_regval_list, nrf24_default_regval_list_num); 
    
    /* Start */
    uint8_t pipe;
    uint8_t len;
    uint8_t buf[33];
    while(1) {
        if (nrf24_rxfifo_has_data(&g_nrf24)) {
            nrf24_rxfifo_read(&g_nrf24, buf, &len, &pipe);
            buf[len] = '\0';
            rt_kprintf("Listening: data from pipe %d: %s\n", pipe, buf);
        }else {
            rt_thread_mdelay(5);
        }
    }
}
