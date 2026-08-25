# XiaoZhi 烧录与操控平台

一个纯静态网页平台，配套本仓库的 `electron-bot` 板型（ESP32-S3 桌面对话机器人，无头无手脚版本），提供：

- **烧录**：跳转乐鑫官方 ESP Launchpad，不在平台里重复做烧录器
- **操控**：通过设备自带 WebSocket 服务（`ws://<设备IP>:8080/ws`）调用固件 MCP 工具
- **串口监视**：直接读取设备串口日志、发送调试命令
- **使用说明**：编译、配网、切服务器的操作步骤

## 目录

```text
platform/
├── index.html              # 单页入口
├── css/style.css           # 样式
├── js/
│   ├── app.js              # 页面切换与模块入口
│   ├── control.js          # WebSocket 操控逻辑（JSON-RPC 2.0）
│   └── serial-monitor.js   # 串口监视逻辑
├── start.ps1 / start.bat   # 一键启动本地服务器
└── README.md
```

## 板型说明（重要）

按你提供的引脚反查，仓库里匹配的板型是 `electron-bot`：

| 外设 | 引脚 |
| --- | --- |
| 麦克风 I2S（WS/SCK/DIN） | GPIO 40 / 42 / 41 |
| 喇叭 I2S（DOUT/BCLK/LRCK） | GPIO 17 / 18 / 8 |
| 屏幕 SPI（SCLK/MOSI/CS/DC/RST） | GPIO 11 / 10 / 12 / 13 / 9 |
| 屏幕背光 / BOOT / 充电检测 | GPIO 46 / 0 / 14 |

`electron-bot` 的 `config.h` 还定义了 6 个舵机引脚（右臂 pitch/roll = 5/4、左臂 pitch/roll = 7/15、身体 = 6、头部 = 16）。你的设备没有头和手脚，这些引脚空置即可，不需要改代码。

硬件层（屏幕、麦克风、喇叭、电池、背光、按键）的 C/C++ 代码已经全部实现，**不需要再写引脚操控部分**。伙伴做的 WiFi 连接后的 agent 通过 MCP 工具调用这些能力。

## 启动平台

方法一（推荐）：

```powershell
cd platform
.\start.ps1
```

方法二：

```powershell
cd platform
python -m http.server 8011
```

如果 `python` 命令不在 PATH 里，可以用：

```powershell
py -3 -m http.server 8011
```

或直接使用完整路径：

```powershell
C:\Users\10984\AppData\Local\Programs\Python\Python313\python.exe -m http.server 8011
```

然后用 Chrome 或 Edge 打开：

```text
http://localhost:8011
```

> Web Serial 只在 Chrome/Edge（89+）可用，且页面必须是 HTTPS 或 localhost。
>
> `start.ps1` 会自动找空闲端口（默认从 8011 开始）。如果 8000 被其他程序占用，不要再手动用 8000，直接运行 `start.ps1` 即可。

## 烧录固件

烧录直接使用乐鑫官方在线工具 **ESP Launchpad**：

```text
https://espressif.github.io/esp-launchpad/
```

1. 让伙伴编译（或自己编译）`electron-bot`：

```bash
python scripts/build.py electron-bot --name electron-bot
```

产物：`build/merged-binary.bin`

2. 设备用支持数据传输的 USB 线连接电脑，在设备管理器里确认出现新的 COM 口（蓝牙串口不算）。
3. 用 Chrome/Edge 打开 ESP Launchpad，选择对应的 COM 口。
4. 选择 `merged-binary.bin`，起始地址固定 `0x0`，点击烧录。
5. 烧录完成后设备自动复位，进入配网/启动流程。

## 操控设备

设备连上 WiFi 后，固件会在 **8080 端口** 启动 WebSocket 控制服务：

```text
ws://<设备IP>:8080/ws
```

平台“操控”页的操作：

1. 填入设备 IP，端口默认 `8080`，点击“连接”。
2. 页面会自动调用 `tools/list`，把固件上报的全部 MCP 工具渲染成表单。
3. 音量、亮度、主题、状态查询已做成快捷按钮；其他工具按表单填写后调用。
4. “隐藏舵机/机器人动作工具”默认开启，适合当前无头无手脚设备；如果以后接上舵机，取消勾选即可看到全部动作工具。
5. 底部保留原始 JSON-RPC 输入框，方便调试。

## 操作授权

平台默认启用“每次操作都确认”：

- 状态查询（状态、电量、IP、系统信息等）自动放行，不影响日常查看；
- 重启、音量/亮度、屏幕预览以及所有控制类 MCP 工具，执行前都会弹出确认框；
- 只有点击“允许本次操作”才会真正发送；
- 可以在确认框勾选“本会话内不再询问同类操作”，刷新页面后恢复默认。

也可以在上方“操作授权”卡片切换为“仅高危操作确认”或“不确认（不推荐）”。

> 注意：这个确认机制保护的是“平台使用者”自己。如果 AI agent（伙伴的 Python 服务）通过服务器直接调用设备 MCP 工具，属于另一条链路，需要在服务器端做权限控制或接入审批，而不是靠这个网页。

常用工具：

| 工具 | 作用 |
| --- | --- |
| `self.get_device_status` | 设备整体状态 |
| `self.electron.get_ip` | WiFi IP 与连接状态 |
| `self.electron.get_status` | 动作状态（`idle`/`moving`） |
| `self.battery.get_level` | 电量与充电状态 |
| `self.audio_speaker.set_volume` | 设置音量（0-100） |
| `self.screen.set_brightness` | 设置亮度（0-100） |
| `self.screen.set_theme` | 设置主题（light/dark） |
| `self.screen.preview_image` | 显示图片到屏幕 |
| `self.get_system_info` | 系统信息 |
| `self.reboot` | 重启设备 |

## 串口监视

如果 ESP Launchpad 或“串口监视”页已经连接了串口，需要先断开，再到“串口监视”页重新连接同一个 COM 口。默认波特率 115200，可切换 460800/921600。

## 切换到你自己的服务器

固件激活时，服务器会下发 `mqtt` 或 `websocket` 配置；两者都没有时兜底 MQTT。默认 OTA 地址在 `main/Kconfig.projbuild`：

```text
https://api.tenclass.net/xiaozhi/ota/
```

换成自己的服务器（当前为全模块部署）：

1. 部署 `xiaozhi-esp32-server`（全模块），确认：
   - OTA 接口：`http://主机IP:8002/xiaozhi/ota/`（智控台提供；单模块部署时是 8003）
   - WebSocket：`ws://主机IP:8000/xiaozhi/v1/`
2. 设备激活后，登录智控台把设备绑定到智能体（设备管理 → 输入屏幕上的 6 位激活码）
3. 设备配网页的“高级选项”中填你的 OTA 地址（`http://主机IP:8002/xiaozhi/ota/`），重启设备完成激活。

## 常见问题

**Q：浏览器提示 Web Serial 不可用？**

必须使用 Chrome/Edge，并且页面通过 `http://localhost:8011` 或 HTTPS 打开。

**Q：看不到 ESP32 的 COM 口？**

确认 USB 线支持数据传输（不是纯充电线），并检查设备管理器；之前看到的 COM6/COM7 是蓝牙串口，不是 ESP32。

**Q：烧录失败，提示无法同步？**

先确认芯片是否进入下载模式，尝试在 ESP Launchpad 里换波特率 115200，或先擦除 Flash 再重新烧录。ElectronBot 的 BOOT 键（GPIO0）也可用于进入下载模式。

**Q：设备没有舵机，烧录 electron-bot 固件会不会出错？**

不会。舵机引脚空置时，固件的舵机初始化只是输出 PWM，不影响屏幕、麦克风、喇叭和电池；操控页默认也隐藏了舵机工具。
