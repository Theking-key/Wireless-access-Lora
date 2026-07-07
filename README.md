# Wireless Access

Wireless Access 是一个基于 STM32F103C8 的 LoRa 无线温湿度采集与显示工程。项目使用 SX1276/SX1278 LoRa 射频芯片进行节点间通信，使用 DHT11 采集温湿度，通过串口屏显示本地和远端数据，并支持在屏幕配置页修改部分 LoRa 参数和设备角色。

## 项目功能

- DHT11 温湿度采集，默认每 1 秒读取一次。
- LoRa 无线收发，默认工作在 LoRa 模式。
- 支持两种设备角色：
  - `device`：终端节点，采集本地温湿度并周期性发送给网关。
  - `gateway`：网关节点，接收终端数据，并可回传自身温湿度。
- 串口屏交互，显示本地温湿度、远端温湿度和当前设备角色。
- 配置页可调整频率、发射功率、前导码长度、扩频因子、带宽、网络 ID、源地址和设备角色。
- LED 状态指示，湿度越高闪烁越快。
- 使用 EasyLogger 输出调试日志。

## 硬件平台

| 模块 | 配置 |
| --- | --- |
| MCU | STM32F103C8，Cortex-M3 |
| 主频 | 外部 HSE，经 PLL 倍频到 72 MHz |
| LoRa 芯片 | SX1276/SX1278 兼容驱动 |
| 传感器 | DHT11 |
| 显示 | 大彩串口屏协议 |
| 开发环境 | Keil MDK-ARM，工程文件位于 `MDK-ARM/eventdriver.uvprojx` |

## 引脚说明

| 功能 | 引脚 |
| --- | --- |
| LED | PC13，低电平点亮 |
| DHT11 数据线 | PC15 |
| 按键 1 | PC14，下降沿/上拉 |
| 按键 2 | PB13，下降沿/上拉 |
| 按键 3 | PB14，上拉输入 |
| LoRa NSS | PA4 |
| LoRa RESET | PB10 |
| LoRa DIO0 | PB11，外部中断 |
| LoRa DIO1 | PB12，外部中断 |
| LoRa DIO3 | PB4 |
| 调试串口 | USART1，115200 bps |
| 串口屏 | USART2，9600 bps，DMA 接收 + 空闲中断 |
| LoRa SPI | SPI1，主机模式 |
| 微秒延时定时器 | TIM4，1 MHz 计数基准 |

## 默认 LoRa 参数

| 参数 | 默认值 |
| --- | --- |
| 工作模式 | LoRa |
| 频率 | 435000000 Hz |
| 发射功率 | 18 dBm |
| 带宽 | 125 kHz |
| 扩频因子 | SF12 |
| 编码率 | 4/5 |
| 前导码长度 | 10 |
| CRC | 开启 |
| 跳频 | 关闭 |
| 网络 ID | 0x10 |
| 终端源地址 | 0x01 |
| 网关地址 | 0x00 |

LoRa 应用层数据帧定义在 `user/app.c` 中，字段为：

```c
typedef struct lora_msg_frame_t
{
    uint8_t network_id;
    uint8_t source_addr;
    uint8_t dest_addr;
    uint8_t temp;
    uint8_t humi;
} lora_msg_frame_t;
```

## 目录结构

```text
.
├── Core/                  STM32CubeMX 生成的启动入口、中断和 HAL MSP 文件
├── Drivers/               STM32 HAL、CMSIS、DSP/NN 等官方库文件
├── MDK-ARM/               Keil 工程、启动文件和构建输出
├── bin/                   已生成的网关/终端 hex 固件
├── easylogger/            EasyLogger 日志库
├── lcd/                   串口屏驱动、命令队列和控件协议
├── user/                  应用层、BSP、DHT11、按键、串口接口、消息执行器
│   ├── mem/               TLSF 动态内存管理
│   └── radio/             SX1276/SX1278 LoRa 驱动及板级适配
└── eventdriver.ioc        STM32CubeMX 配置文件
```

## 软件流程

1. `Core/Src/main.c` 完成 HAL、时钟、GPIO、DMA、USART、TIM4 和 SPI1 初始化。
2. 初始化 EasyLogger、动态内存、消息执行器和虚拟定时器。
3. `app_init()` 创建主应用模块，初始化按键、串口屏接收、LoRa 驱动和默认显示内容。
4. 主循环持续调用 `MsgExec_Exec()`，集中处理定时器、串口屏、LoRa 接收等消息。
5. 应用定时器包含：
   - 500 ms LED 闪烁定时器，实际间隔会随湿度变化。
   - 10 ms 按键扫描定时器。
   - 1000 ms 传感器采集定时器。
6. 终端节点每采集 2 次向地址 `0x00` 的网关发送一次温湿度数据。
7. 网关收到目标地址匹配且网络 ID 正确的数据后刷新远端温湿度显示，并回传自身温湿度。

## 编译与烧录

### 使用 Keil MDK

1. 打开 `MDK-ARM/eventdriver.uvprojx`。
2. 确认目标芯片为 `STM32F103C8`。
3. 编译工程，输出文件名为 `eventdriver`，工程已开启 hex 生成。
4. 使用 ST-Link、J-Link 或串口下载工具将 hex 烧录到目标板。

### 使用已有固件

`bin/` 目录下已经包含两个预编译固件：

- `bin/eventdriver-device.hex`：终端节点固件。
- `bin/eventdriver-gateway.hex`：网关节点固件。

如果只做硬件联调，可直接烧录对应 hex 文件。

## 修改设备角色

源码默认角色在 `user/app.c` 的 `app_init()` 中设置：

```c
g_app.device_role = 0;
```

含义如下：

- `0`：终端节点 `device`
- `1`：网关节点 `gateway`

也可以通过串口屏配置页选择角色，点击保存后会调用 `app_update_channel()` 更新无线参数。

## 关键源码文件

| 文件 | 作用 |
| --- | --- |
| `Core/Src/main.c` | 系统初始化、主循环、外设初始化 |
| `Core/Src/stm32f1xx_it.c` | DMA、USART、EXTI 中断处理 |
| `user/app.c` | 主业务逻辑、LoRa 收发、串口屏事件、传感器定时采集 |
| `user/vMsgExec.c` | 基于 CMSIS-RTOS mail queue/timer 的消息执行器 |
| `user/uart_interface.c` | USART2 DMA 接收、空闲中断分帧、串口发送 |
| `user/dht11.c` | DHT11 单总线时序读取 |
| `user/key.c` | 10 ms 周期按键扫描和消抖 |
| `user/radio/sx1276-board.c` | SX1276 板级 SPI/中断适配 |
| `lcd/hmi_driver.c` | 大彩串口屏命令封装 |
| `easylogger/` | 日志输出组件 |

## 当前资源占用

根据已有 Keil map 文件，当前构建大致资源占用如下：

| 类型 | 大小 |
| --- | --- |
| RO Size | 37.37 KB |
| RW + ZI Size | 16.32 KB |
| ROM Size | 37.54 KB |

目标芯片工程配置为 IROM `0x08000000-0x0800FFFF`，IRAM `0x20000000-0x20004FFF`。

## 注意事项

- 代码中部分中文注释存在编码显示异常，不影响编译，但建议后续统一保存为 UTF-8 或 GBK。
- `DHT11_Read_Bit()` 中局部变量 `retry` 未显式初始化，建议后续修正为 `uint8_t retry = 0;`。
- `SX1276SetAntSw()` 和 `SX1276SetAntSwLowPower()` 当前为空实现，如硬件板需要射频开关控制，应按实际电路补充。
- `HAL_GetTick()` 被重写为返回 `os_time`，需要确保 CMSIS-RTOS tick 正常维护该变量。
- 当前源码默认 `device` 角色，如需要分别维护网关/终端版本，建议使用编译宏区分角色配置。

## 简单介绍

这是一个 STM32F103C8 上的 LoRa 无线温湿度采集系统。终端节点通过 DHT11 采集温湿度并用 SX1276/SX1278 发送给网关，网关接收后在串口屏上显示远端数据，同时可以回传自身数据。项目包含完整的 Keil 工程、HAL 驱动、LoRa 驱动、串口屏协议、日志系统和已编译好的网关/终端 hex 固件，适合用于无线传感器节点、环境监测和 LoRa 通信实验。
