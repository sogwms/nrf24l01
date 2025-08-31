#include <rtthread.h>
#include <rtdevice.h>
#include "nrf24l01.h"
#include "nrf24l01_dep_impl.h"

#ifdef NRF24L01_DEMO_HAL_AUTO_SPI_DEV
#define VAR_CS_PIN NRF24L01_DEMO_HAL_CS_PIN
#define VAR_SPI_BUS NRF24L01_DEMO_HAL_SPI_BUS_NAME
#define NEED_AUTO_GEN_SPIDEV
#endif

#define VAR_CE_PIN  NRF24L01_DEMO_HAL_CE_PIN
#define VAR_SPI_DEV  NRF24L01_DEMO_HAL_SPI_DEV_NAME

#define _STRINGIFY(x) #x
#define _TOSTRING(x) _STRINGIFY(x) 
#define INIT_FN_NAME nrf24l01_demo_##DEMONAME
#define THREAD_NAME "nrf24" _TOSTRING(DEMONAME) "demo"

#define DEMONAME_STR _TOSTRING(DEMONAME)

nrf24_t g_nrf24;
struct nrf24_depimpl_ctx g_depimpl_ctx;

static void thread_entry(void *param)
{
    rt_kprintf("nrf24l01 demo %s\n", DEMONAME_STR);

    if (rt_pin_get(VAR_CE_PIN) < 0) {
        rt_kprintf("invalid ce pin\n");
    }
    
    /* Init lower*/
#ifdef NEED_AUTO_GEN_SPIDEV
    if (rt_pin_get(VAR_CS_PIN) < 0) {
        rt_kprintf("invalid cs pin\n");
    }
    nrf24_depimpl_init_ctx(&g_depimpl_ctx, VAR_SPI_DEV, rt_pin_get(VAR_CE_PIN), VAR_SPI_BUS, rt_pin_get(VAR_CS_PIN));
#else
    nrf24_depimpl_init_ctx_with_spidev(&g_depimpl_ctx, VAR_SPI_DEV, rt_pin_get(VAR_CE_PIN));
#endif

    /* Do initialization */
    nrf24_global_init();

    nrf24_init(&g_nrf24, nrf24_depimpl_get_ops(), &g_depimpl_ctx);

    /* Routine */
    DEMONAME();
    
    /* Exit */
    nrf24_deinit(&g_nrf24);
}

static int INIT_FN_NAME(void)
{
    rt_thread_t thread;

    thread = rt_thread_create(THREAD_NAME, thread_entry, RT_NULL, 1024, RT_THREAD_PRIORITY_MAX/2, 20);
    rt_thread_startup(thread);

    return RT_EOK;
}

INIT_APP_EXPORT(INIT_FN_NAME);

