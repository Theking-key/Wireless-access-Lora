# Wireless Access

<p align="left">
  <img src="https://img.shields.io/badge/MCU-STM32F103C8-blue" alt="MCU">
  <img src="https://img.shields.io/badge/Radio-SX1276%2FSX1278-green" alt="Radio">
  <img src="https://img.shields.io/badge/Sensor-DHT11-orange" alt="Sensor">
  <img src="https://img.shields.io/badge/IDE-Keil%20MDK--ARM-red" alt="IDE">
  <img src="https://img.shields.io/badge/License-MIT-yellow" alt="License">
</p>

基于 **STM32F103C8** 的 LoRa 无线温湿度采集与显示系统。终端节点通过 DHT11 采集环境温湿度，经 SX1276/SX1278 LoRa 射频芯片发送至网关；网关接收后在串口屏上同步显示远端数据，并可回传自身温湿度。适用于无线传感器网络、环境监测和 LoRa 通信实验等场景。

---

## 📋 功能特性

- 🌡️ **DHT11 温湿度采集** — 默认 1 秒周期读取
- 📡 **LoRa 无线收发** — 基于 SX1276/SX1278，支持点对点通信
- 🔄 **双角色切换** — 支持 `device`（终端）和 `gateway`（网关）两种模式
- 🖥️ **串口屏交互** — 大彩串口屏实时显示本/远端温湿度及设备角色
- ⚙️ **参数在线配置** — 通过串口屏配置页调整频率、功率、扩频因子、带宽、网络 ID 等 LoRa 参数
- 💡 **LED 状态指示** — 湿度越高，闪烁越快，直观反映环境变化
- 📝 **日志调试** — 集成 EasyLogger，支持分级日志输出

## 🛠️ 硬件平台

| 模块 | 配置 |
|------|------|
| **MCU** | STM32F103C8, Cortex-M3 @ 72 MHz (HSE + PLL) |
| **LoRa 芯片** | SX1276 / SX1278 兼容驱动 |
| **温湿度传感器** | DHT11 (单总线) |
| **显示屏** | 大彩串口屏 (USART2, 9600 bps, DMA + 空闲中断) |
| **调试串口** | USART1, 115200 bps |
| **微秒定时器** | TIM4 @ 1 MHz |
| **LoRa SPI** | SPI1, 主机模式 |
| **开发环境** | Keil MDK-ARM (工程: `MDK-ARM/eventdriver.uvprojx`) |

### 引脚分配

| 功能 | 引脚 | 说明 |
|------|------|------|
| LED 指示 | PC13 | 低电平点亮 |
| DHT11 数据 | PC15 | 单总线 |
| 按键 1 | PC14 | 下降沿触发，上拉 |
| 按键 2 | PB13 | 下降沿触发，上拉 |
| 按键 3 | PB14 | 上拉输入 |
| LoRa NSS | PA4 | SPI 片选 |
| LoRa RESET | PB10 | 复位 |
| LoRa DIO0 | PB11 | 外部中断 |
| LoRa DIO1 | PB12 | 外部中断 |
| LoRa DIO3 | PB4 | 通用 IO |

## 📦 项目结构

```
.
├── Core/                       # STM32CubeMX 生成代码
│   ├── Inc/                    #   main.h, stm32f1xx_hal_conf.h, stm32f1xx_it.h
│   └── Src/                    #   main.c, stm32f1xx_hal_msp.c, stm32f1xx_it.c
├── Drivers/                    # STM32 HAL + CMSIS 官方库
│   ├── CMSIS/
│   └── STM32F1xx_HAL_Driver/
├── MDK-ARM/                    # Keil 工程文件与构建输出
├── user/                       # 应用层代码
│   ├── app.c / app.h           #   主业务逻辑（LoRa 收发、传感器、串口屏事件）
│   ├── bsp.c / bsp.h           #   板级支持包（外设句柄导出）
│   ├── dht11.c / dht11.h       #   DHT11 驱动
│   ├── key.c / key.h           #   按键扫描与消抖
│   ├── uart_interface.c/h      #   USART2 DMA 接收 + 空闲中断分帧
│   ├── vMsgExec.c/h            #   基于 CMSIS-RTOS 消息队列的执行器
│   ├── radio/                  #   SX1276/SX1278 LoRa 驱动
│   │   ├── sx1276-board.c/h    #     板级 SPI/中断适配
│   │   ├── radio.h             #     射频抽象接口
│   │   ├── delay.c/h           #     微秒级延时
│   │   ├── timer.c/h           #     硬件定时器封装
│   │   └── utilities.c/h       #     工具函数
│   └── mem/                    #   TLSF 动态内存管理器
│       └── tlsf.c/h
├── lcd/                        # 串口屏驱动与协议
│   ├── hmi_driver.c/h          #   大彩串口屏命令封装
│   ├── cmd_process.h           #   命令处理器
│   ├── cmd_queue.c/h           #   命令队列
│   └── hmi_user_uart.c/h       #   用户串口接口
├── easylogger/                 # EasyLogger 日志库
│   ├── inc/                    #   头文件
│   ├── src/                    #   核心代码
│   ├── port/                   #   平台移植接口
│   └── plugins/                #   插件
├── bin/                        # 预编译固件
│   ├── eventdriver-device.hex  #   终端节点
│   └── eventdriver-gateway.hex #   网关节点
└── eventdriver.ioc             # STM32CubeMX 配置
```

## 🧠 软件架构

项目采用 **消息驱动的协作式调度** 模型，核心组件为 `vMsgExec`（消息执行器）：

```
┌─────────────────────────────────────────────────────────┐
│                     主循环 Main Loop                      │
│              MsgExec_Exec() 消息分发调度                     │
└─────────────────────────────────────────────────────────┘
         │
         ├── 定时器消息 (Timer) ──── 500ms LED / 10ms 按键 / 1000ms 传感器
         ├── 串口屏消息 (UART) ──── 控件事件 → app_event_handler()
         └── LoRa 消息 (DIO IRQ) ── 无线数据接收 → 数据解析与显示更新
```

### 启动流程

1. **`main.c`** — HAL 初始化、时钟配置、GPIO/DMA/USART/TIM4/SPI 外设初始化
2. **初始化组件** — EasyLogger、TLSF 动态内存、vMsgExec、虚拟定时器
3. **`app_init()`** — 创建主应用模块，初始化按键、串口屏接收、LoRa 驱动和默认显示
4. **主循环** — 持续调用 `MsgExec_Exec()`，无阻塞调度消息

### LoRa 数据帧格式

应用层自定义帧结构（`user/app.c`）：

```c
typedef struct {
    uint8_t network_id;    // 网络 ID
    uint8_t source_addr;   // 源地址
    uint8_t dest_addr;     // 目标地址
    uint8_t temp;          // 温度
    uint8_t humi;          // 湿度
} lora_msg_frame_t;
```

- 终端节点每采集 2 次向网关 (地址 `0x00`) 发送一次温湿度
- 网关收到网络 ID 匹配的数据后，更新远端温湿度并回传自身数据

## ⚡ 默认 LoRa 参数

| 参数 | 默认值 | 可配置 |
|------|--------|:------:|
| 工作模式 | LoRa | ❌ |
| 频率 | 435.000 MHz | ✅ |
| 发射功率 | 18 dBm | ✅ |
| 带宽 | 125 kHz | ✅ |
| 扩频因子 | SF12 | ✅ |
| 编码率 | 4/5 | ❌ |
| 前导码长度 | 10 | ✅ |
| CRC | 开启 | ❌ |
| 跳频 | 关闭 | ❌ |
| 网络 ID | 0x10 | ✅ |
| 终端源地址 | 0x01 | ✅ |
| 网关地址 | 0x00 | ❌ |

> 可通过串口屏配置页修改带 ✅ 标记的参数，保存后调用 `app_update_channel()` 生效。

## 🚀 编译与烧录

### 使用 Keil MDK

1. 打开 `MDK-ARM/eventdriver.uvprojx`
2. 确认目标芯片为 **STM32F103C8**
3. 编译工程，已开启 HEX 文件生成
4. 使用 ST-Link / J-Link / 串口下载工具将 hex 烧录至目标板

### 使用预编译固件

`bin/` 目录下已包含两个预编译 hex：

| 文件 | 角色 |
|------|------|
| `bin/eventdriver-device.hex` | 终端节点 (device) |
| `bin/eventdriver-gateway.hex` | 网关节点 (gateway) |

直接烧录即可联调，无需编译环境。

## 🔧 设备角色配置

源码中通过 `user/app.c` 的 `app_init()` 设置角色：

```c
g_app.device_role = 0;   // 0 = 终端 (device), 1 = 网关 (gateway)
```

也可以通过串口屏配置页动态切换，保存后自动更新无线参数。

## 📁 关键源码文件说明

| 文件 | 职责 |
|------|------|
| `Core/Src/main.c` | 系统初始化、外设初始化、主循环 |
| `Core/Src/stm32f1xx_it.c` | DMA、USART、EXTI 中断服务 |
| `user/app.c` | 核心业务逻辑：LoRa 收发、串口屏事件、定时采集调度 |
| `user/vMsgExec.c` | 消息执行器，基于 CMSIS-RTOS mail queue + 虚拟定时器 |
| `user/uart_interface.c` | USART2 DMA 接收 + 空闲中断帧同步 |
| `user/dht11.c` | DHT11 单总线时序驱动 |
| `user/key.c` | 10ms 周期按键扫描与消抖 |
| `user/radio/sx1276-board.c` | SX1276 板级 SPI/RST/DIO 适配层 |
| `lcd/hmi_driver.c` | 大彩串口屏协议封装 |
| `easylogger/` | 嵌入式日志系统 |

## 💾 资源占用

| 类型 | 大小 |
|------|------|
| Code (RO) | 37.37 KB |
| Data (RW + ZI) | 16.32 KB |
| ROM (Code + RO data) | 37.54 KB |

内存映射：IROM `0x08000000–0x0800FFFF` (64 KB) / IRAM `0x20000000–0x20004FFF` (20 KB)

## ⚠️ 注意事项

1. **编码问题** — 部分中文注释存在编码显示异常（不影响编译），建议后续统一为 UTF-8 或 GBK
2. **未初始化变量** — `DHT11_Read_Bit()` 中 `retry` 未显式初始化，建议修正为 `uint8_t retry = 0;`
3. **射频开关** — `SX1276SetAntSw()` / `SX1276SetAntSwLowPower()` 当前为空实现，如需射频开关控制请按实际电路补充
4. **HAL Tick 重写** — `HAL_GetTick()` 被重写为返回 `os_time`，需确保 CMSIS-RTOS tick 正常维护
5. **角色管理** — 默认角色为 `device`，如需分别维护网关/终端固件，建议使用编译宏区分

## 📜 许可证

本项目仅用于学习和实验目的。
