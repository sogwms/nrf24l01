# nRF24L01

> Powered by idriver

## 1、介绍

这是一个 RT-Thread 的软件包，该软件包提供了 nRF24L01 模块的驱动。

nRF24L01 是由 NORDIC 生产的工作在 2.4GHz~2.5GHz 的 ISM 频段的单片无线收发器芯片。

> 更多关于 nRF24L01 的信息请参阅 [_features.md_](/docs/features.md) 或 _数据手册_


### 1.1 目录结构

| 名称 | 说明 |
| ---- | ---- |
| backup | 一些相关资料的备份 |
| doc | 文档 |
| demo | 示例 |
| core  | 驱动核心 |
| platform  | 平台相关的对接部分 |

### 1.2 许可证

nRF24L01 package 遵循 Apache license v2.0 许可，详见 `LICENSE` 文件。

### 1.3 依赖

- RT-Thread PIN 设备
- RT-Thread SPI 设备

## 2、获取软件包

使用 nRF24L01 package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages
    peripheral libraries and drivers --->
        [*] nRF24L01: Single-chip 2.4GHz wireless transceiver.
```

选中后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到工程中。

## 3、使用 nRF24L01

_**[用户指南](./doc/user_guide.md)**_

_**具体用例：参见 demo 目录**_

_**[开发者/维护者指南](./doc/dev_guide.md)**_

## 4、注意事项

- 因 Si24R1 相当于对 nRF24L01 的复刻，此也适用于 Si24R1 芯片。但要注意两者还是略有不同, 如发射功率等有差异（寄存器和具体功率）; SPI 频率上限不同;

- 因 RT-Thread 的 SPI 的设备挂载等相关的 API 在不同版本或 BSP 中可能声明或行为存在差异，驱动对此仅简单应对，当出现相关的问题时，建议使用者关闭/不要使用本驱动的自动挂载设备的功能（即不要打开 `NRF24L01_SUPPORT_AUTO_SPIDEV`, 由用户负责 SPI 设备挂载（即使用`nrf24_depimpl_init_ctx_with_spidev` 而不是 `nrf24_depimpl_init_ctx`））（总结：为了兼容性/稳定性，建议向驱动提供已挂载好的SPI设备，而非由驱动进行自动挂载）

## 5、其他说明

- 如果有需要移植该驱动到其它平台（如裸机，Linux等），请查看 `https://github.com/IDriver/nrf24l01`

## 6、联系方式

- 维护：sogwyms@gmail.com
- 主页：https://github.com/sogwms/nrf24l01
