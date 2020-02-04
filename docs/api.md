# API 手册

## 重要结构体说明

***

```c
struct nrf24_callback
{
    void (*rx_ind)(nrf24_t nrf24, uint8_t *data, uint8_t len, int pipe);
    void (*tx_done)(nrf24_t nrf24, int pipe);
};
```

作用: 配置回调函数

详细:

- rx_ind: 接收回调函数，当 run 检测并接收到数据时会调用。参数 len 指示 data 的长度，pipe 参数指示从哪个通道接收到的数据（仅针对 ROLE_PRX 有意义）

- tx_done: 发送回调函数，当 run 检测到发送结果时调用。对于 ROLE_PTX, pipe 参数指示发送失败(NRF24_PIPE_NONE)或成功(非NRF24_PIPE_NONE)

***

```c
struct nrf24_cfg
{
    nrf24_role_et role;
    nrf24_power_et power;
    nrf24_crc_et crc;
    nrf24_adr_et adr;
    uint8_t channel :7;    //range: 0 ~ 127 (frequency:)

    int _irq_pin;
    
    uint8_t txaddr[5];

    struct {
        uint8_t bl_enabled;
        uint8_t addr[5];
    } rxpipe0;
    struct {
        uint8_t bl_enabled;
        uint8_t addr[5];
    } rxpipe1;

    struct {
        uint8_t bl_enabled;
        uint8_t addr;
    } rxpipe2;
    struct {
        uint8_t bl_enabled;
        uint8_t addr;
    } rxpipe3;
    struct {
        uint8_t bl_enabled;
        uint8_t addr;
    } rxpipe4;
    struct {
        uint8_t bl_enabled;
        uint8_t addr;
    } rxpipe5;

};
```

作用: 配置 nrf24l01

详细:

- role: 角色选择 (选项参见相应 enum)

- power: 功率 (选项参见相应 enum)

- crc：CRC (选项参见相应 enum)

- adr：空中速率 (选项参见相应 enum)

- channel: 频率 (范围：0～127)

- txaddr: 发送地址

- rxpipex: bl_enabled 视为布尔值，决定是否启用该 pipe。addr 为pipe接收地址

## 重要函数说明

***

```c
int nrf24_init(nrf24_t nrf24, char *spi_dev_name, int ce_pin, int irq_pin, const struct nrf24_callback *cb, const nrf24_cfg_t cfg);
```

功能：初始化 nrf24 对象

参数：

- spi_dev_name: spi 设备名
- ce_pin: nrf24l01 的 ce 引脚
- irq_pin: nrf24l01 的 中断 引脚。 若配置为 NRF24_PIN_NONE 表示不使用中断
- cb：回调函数
- cfg: 配置

***

```c
nrf24_t nrf24_create(char *spi_dev_name, int ce_pin, int irq_pin, const struct nrf24_callback *cb, const nrf24_cfg_t cfg);
```

功能：创建 nrf24 对象并初始化

参数：同上

***

```c
int nrf24_fill_default_config_on(nrf24_cfg_t cfg);
```

功能：将 cfg 设置为默认值（配置）

***

```c
int nrf24_default_init(nrf24_t nrf24, char *spi_dev_name, int ce_pin, int irq_pin, const struct nrf24_callback *cb, nrf24_role_et role);
```

功能：将配置尽量采取默认值，简化版初始化

参数：基本同上

***

```c
nrf24_t nrf24_default_create(char *spi_dev_name, int ce_pin, int irq_pin, const struct nrf24_callback *cb, nrf24_role_et role);
```

nrf24_default_init 的 create 版本

***

```c
int nrf24_send_data(nrf24_t nrf24, uint8_t *data, uint8_t len, uint8_t pipe);
```

功能：发送数据

参数：

- pipe 为 枚举类型 (NRF24_PIPE_X)

注意：该函数返回时并不意味着发送完成，发送的结果（失败或成功）是通过回调函数通知的。


***

```c
int nrf24_run(nrf24_t nrf24);
```

功能：运行 nrf24 对象

注意：

- 对于使用中断的 nrf24，run会阻塞。非中断则不会

```c
void nrf24_enter_power_up_mode(nrf24_t nrf24);
```

功能：进入上电模式，以便正常工作

***

```c
void nrf24_enter_power_down_mode(nrf24_t nrf24);
```

功能：进入掉电模式，进入最低功耗模式（无法通信）

_*更详细的信息，参见源码*_
