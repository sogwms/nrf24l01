# nRF24L01

## 1、介绍

这是一个 RT-Thread 的软件包，该软件包提供了 nRF24L01 模块的驱动。

nRF24L01 是由 NORDIC 生产的工作在 2.4GHz~2.5GHz 的 ISM 频段的单片无线收发器芯片。

> 更多关于 nRF24L01 的信息请参阅 [_features.md_](/docs/features.md) 或 _数据手册_

### 1.1 目录结构

| 名称 | 说明 |
| ---- | ---- |
| docs  | 文档 |
| examples | 有关如何使用该驱动的样例代码 |
| src  | 源代码目录 |

### 1.2 许可证

nRF24L01 package 遵循 Apache license v2.0 许可，详见 `LICENSE` 文件。

### 1.3 依赖

- RT-Thread PIN 设备
- RT-Thread SPI 设备

## 2、如何打开 nRF24L01

> 说明：描述该 package 位于 menuconfig 的位置，并对与其相关的配置进行介绍

使用 hello package 需要在 RT-Thread 的包管理器中选择它，具体路径如下：

```
RT-Thread online packages
    miscellaneous packages --->
        [*] A hello package
```

然后让 RT-Thread 的包管理器自动更新，或者使用 `pkgs --update` 命令更新包到 BSP 中。

## 3、使用 hello

> 说明：在这里介绍 package 的移植步骤、使用方法、初始化流程、准备工作、API 等等，如果移植或 API 文档内容较多，可以将其独立至 `docs` 目录下。

在打开 hello package 后，当进行 bsp 编译时，它会被加入到 bsp 工程中进行编译。

- 完整的 API 手册可以访问这个[链接](docs/api.md)
- 更多文档位于 [`/docs`](/docs) 下，使用前 **务必查看**

## 4、注意事项

> 说明：列出在使用这个 package 过程中需要注意的事项；列出常见的问题，以及解决办法。

## 5、联系方式

- 维护：sogwms
- 主页：https://github.com/sogwms/nrf24l01
