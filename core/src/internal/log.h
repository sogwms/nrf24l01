/* Define default log sink (in case of no log sink defined) */
#ifndef NRF24L01_LOG_SINK
#define NRF24L01_LOG_SINK(level, fmt, ...)
#endif

/* Define default log level */
#ifndef NRF24L01_LOG_MIN_OUTPUT_LEVEL
#define NRF24L01_LOG_MIN_OUTPUT_LEVEL 'D'
#endif

/* Caller-Facing Logging API encapsulation */
#define LOG_V(fmt, ...) NRF24L01_LOG_SINK('V', fmt, ##__VA_ARGS__)
#define LOG_D(fmt, ...) NRF24L01_LOG_SINK('D', fmt, ##__VA_ARGS__)
#define LOG_I(fmt, ...) NRF24L01_LOG_SINK('I', fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...) NRF24L01_LOG_SINK('W', fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) NRF24L01_LOG_SINK('E', fmt, ##__VA_ARGS__)

/**/
#define NRF24_CHECK(cond) do { if (!(cond)) {LOG_E("Check '%s' failed!", #cond); return -128;}} while (0)

/* Log min-output-level implementaion */
#ifdef NRF24L01_LOG_MIN_OUTPUT_LEVEL
    #if (NRF24L01_LOG_MIN_OUTPUT_LEVEL == 'V')

    #elif (NRF24L01_LOG_MIN_OUTPUT_LEVEL == 'D') 
    #undef LOG_V
    #define LOG_V(fmt, ...)

    #elif (NRF24L01_LOG_MIN_OUTPUT_LEVEL == 'I') 
    #undef LOG_V
    #undef LOG_D
    #define LOG_V(fmt, ...)
    #define LOG_D(fmt, ...)

    #elif (NRF24L01_LOG_MIN_OUTPUT_LEVEL == 'W') 
    #undef LOG_V
    #undef LOG_D
    #undef LOG_I
    #define LOG_V(fmt, ...)
    #define LOG_D(fmt, ...)
    #define LOG_I(fmt, ...)

    #elif (NRF24L01_LOG_MIN_OUTPUT_LEVEL == 'E') 
    #undef LOG_V
    #undef LOG_D
    #undef LOG_I
    #undef LOG_W
    #define LOG_V(fmt, ...)
    #define LOG_D(fmt, ...)
    #define LOG_I(fmt, ...)
    #define LOG_W(fmt, ...)

    #else
    #error "Invalid NRF24L01_LOG_MIN_OUTPUT_LEVEL (range: 'V', 'D', 'I', 'W', 'E'), please check"

    #endif
#endif // NRF24L01_LOG_MIN_OUTPUT_LEVEL