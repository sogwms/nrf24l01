# 使用前必读

## I. 必要了解（关于 nRF24L01）

### 1. nRF24L01 特点

- nRF24L01 是一个无线通信模块，其通信方式属于**半双工**

- nRF24L01 通信以包为单位，一个包最多携带 32 字节的数据

- nRF24L01 中有增强性突发模式，该模式可实现自动应答、重传等机制而无需 MCU 的干预. 在该模式下有两个角色：Primary Transmitter（PTX）和 Primary Receiver（PRX）. _（本驱动向上层提供的API即基于此模式）_

- 要使 nRF24L01 模块间通信必须匹配关键设置：速率、地址、频率(频道)、CRC 等

### 2. PTX、PRX 通信过程**简要说明**

1. PTX 方发送数据
2. PRX 方接收到数据
3. PRX 方回应一个 ACK，该 ACK 可以携带数据（发送给 PTX 方的）
4. PTX 方接收到 ACK (可能包含 PRX 发送来的数据)

从上述通信过程可以看出：_PTX 属于主动方，PRX 属于被动方。如果 PTX 方不发送数据则也无法接收数据，同时 PRX 方即不能发送也无法接收到数据。_

### 3. 当前驱动特点

- 默认项（不可配置）
    - 5-byte 地址宽度

- 支持多实例
- 支持中断方式
- 支持一对多通信

## II. 使用流程

用户首先进行配置和初始化，以获得一个完整的 nrf24 对象， 然后在线程中使用函数 "int nrf24_run(nrf24_t nrf24)" 运行该对象即可。对象会在运行期间调用用户的回调函数

### 1 配置和初始化

为了便利使用，提供了两类初始化， 一类是默认的和非默认的，另一类是动态的和静态的。 默认和非默认的区别在于用户需要进行的配置工作的多少。 动态和静态是指 nrf24 对象的创建方式。

此处以最简便的方式(动态，默认)为例：

```c

static void rx_ind(nrf24_t nrf24, uint8_t *data, uint8_t len, int pipe)
{
    // TODO
}

static void tx_done(nrf24_t nrf24, int pipe)
{
    // TODO
}

const static struct nrf24_callback _cb = {
    .rx_ind = rx_ind,
    .tx_done = tx_done,
};

/* 不使用 IRQ */
nrf24_t nrf24 = nrf24_default_create("spi10", 36, NRF24_PIN_NONE, _cb, ROLE_PTX);

/* 使用 IRQ */
nrf24_t nrf24 = nrf24_default_create("spi10", 36, 37, _cb, ROLE_PTX);

```

- 配置项的详细说明及更灵活的配置和初始化参见 [api.md](./api.md)

显然最简方式关于 nrf24 射频相关的仅需配置一项 ROLE 即可。同样是否使用中断模式配置也很简单仅需配置中断引脚为有效或无效(NRF24_PIN_NONE)即可

ps：对于两端的通信，使用以上方式，仅两端配置成不同角色（一端 ROLE_PTX, 一端 ROLE_PRX）即可

### 2 运行对象

直接调用即可，需要注意的是若配置为中断模式（即中断引脚有效）。run 会阻塞线程。 轮询时(无效中断引脚)则不会。

```c
...
nrf24_run(nrf24);
...
```

### 3 数据发送和接收

不论是哪种 role 发送数据都是调用函数 'nrf24_send_data'，但请一定要认识到 role 的特性和所带来的影响（简单来说 ROLE_PTX 是主动方，ROLE_PRX 是被动的）。xx_run 在检测到在发送完成后（注意：失败也算发送完成）会调用回调函数 tx_done 进行通知。同样 xx_run 在检测并接收到数据后也会调用 rx_ind 进行通知。

## III. 其它

### 1. 驱动提供有 demo 可用，默认关闭，可在通过 `menuconfig` 中打开

```shell
    [*] nRF24L01: Single-chip 2.4GHz wireless transceiver. --->
    [*]     Ese demo
              Role (PTX)   --->
    (spi10)   The spi device name for nrf24l01
    (-1)      The ce pin of nrf24l01
    (-1)      The irq pin of nrf24l01
    (100)     The interval of sending data (for PTX_ROLE)
```

必须指定 spi device name 和 ce pin 为有效值

- -1 为无效 pin。
- irq 为无效 pin 表示不使用中断

### 2. 驱动提供了辅助调试的功能，默认关闭，可在 `menuconfig` 中打开

```shell
    [*] nRF24L01: Single-chip 2.4GHz wireless transceiver. --->
    [*] Enable debug
    [*]   Use info report
    [*]   Use shell cmd
```

- Use info report

选中该选项会启用 void nrf24_report(void); 该函数的功能是读取 nRF24L01 几乎所有的寄存器，然后进行解读并输出解读信息

- Use shell cmd

选中该选项会添加一 msh 命令 nrf24，使用该命令可以对 nRF24L01 进行初始化、读写寄存器等
