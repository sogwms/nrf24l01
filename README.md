# nRF24L01

## 1、介绍

这是一个 RT-Thread 的软件包，该软件包提供了 nRF24L01 模块的驱动。

nRF24L01 是由 NORDIC 生产的工作在 2.4GHz~2.5GHz 的 ISM 频段的单片无线收发器芯片。

> 更多关于 nRF24L01 的信息请参阅 [_features.md_](/docs/features.md) 或 _数据手册_

### 1.1 目录结构

| 名称 | 说明 |
| ---- | ---- |
| docs | 文档 |
| demo | 有关使用该驱动的样例代码 |
| src  | 源代码目录 |

### 1.2 许可证

nRF24L01 package 遵循 Apache license v2.0 许可，详见 `LICENSE` 文件。

### 1.3 依赖

- RT-Thread PIN 设备
- RT-Thread SPI 设备
- RT-Thread IPC (SEM)

## 2、获取软件包

使用 nRF24L01 package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages
    peripheral libraries and drivers --->
        [*] nRF24L01: Single-chip 2.4GHz wireless transceiver.
```

选中后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3、使用 nRF24L01

_**用户指南 --> [user_guide.md](./docs/user_guide.md)**_

_**具体用例, 参见 demo --> [demo.c](./demo/demo.c)**_

## 4、注意事项

无

## 5、联系方式

- 维护：sogwyms@gmail.com
- 主页：https://github.com/sogwms/nrf24l01
