#include "nrf24l01_user_cfg.h"

// /* Set log sink */
// #define NRF24L01_LOG_SINK(level, fmt, ...)

// /* Config log level */
// #define NRF24L01_LOG_MIN_OUTPUT_LEVEL 'I'

/* Environment (dbg | dev | prod) implementation */
#define NRF24L01_ENV_DEBUG 0
#define NRF24L01_ENV_DEV 1
#define NRF24L01_ENV_PROD 2
#ifdef NRF24L01_ENV
#if NRF24L01_ENV == NRF24L01_ENV_DEBUG
    #undef NRF24L01_LOG_MIN_OUTPUT_LEVEL
    #define NRF24L01_LOG_MIN_OUTPUT_LEVEL 'V'
#elif NRF24L01_ENV == NRF24L01_ENV_DEV
    #undef NRF24L01_LOG_MIN_OUTPUT_LEVEL
    #define NRF24L01_LOG_MIN_OUTPUT_LEVEL 'D'
#elif NRF24L01_ENV == NRF24L01_ENV_PROD
    #undef NRF24L01_LOG_MIN_OUTPUT_LEVEL
    #define NRF24L01_LOG_MIN_OUTPUT_LEVEL 'W'
#else
#error "Invalid NRF24L01_ENV"
#endif
#endif // NRF24L01_ENV


/* Check mandatory macros definitions */
