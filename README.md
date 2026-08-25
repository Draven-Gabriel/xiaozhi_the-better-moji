# Moji 桌面 AI 机器人（xiaozhi-moji）

基于 [xiaozhi-esp32](https://github.com/78/xiaozhi-esp32) 与 [xiaozhi-esp32-server](https://github.com/xinnan-tech/xiaozhi-esp32-server) 的定制项目。

- **硬件**：MoveCall Moji（ESP32-S3，240×240 圆屏，支持对话打断/AEC）
- **固件**：`xiaozhi-esp32/`（已精简，仅保留 moji-esp32s3 板型，版本 99.0.0）
- **服务端**：`xiaozhi-esp32-server-main/`（Python 引擎 + 智控台，含定制：抢话过滤、Open-Meteo 免费天气、音乐匹配修复、记忆优化）
- **前端**：智控台为修改版 UI（极光背景/渐变组件/自定义设计系统）

---

## 快速部署（Docker，3 步）

### 第 1 步：准备目录与模型

```bash
# 创建部署数据目录
mkdir -p server-data/data server-data/models/SenseVoiceSmall server-data/uploadfile

# 下载语音识别模型（约 900MB，二选一）
# 魔搭：https://modelscope.cn/models/iic/SenseVoiceSmall/resolve/master/model.pt
# 百度网盘：https://pan.baidu.com/share/init?surl=QlgM58FHhYv1tFnUT_A8Sg 提取码 qvna
# 下载后放到：server-data/models/SenseVoiceSmall/model.pt
```

### 第 2 步：配置

```bash
# 复制配置模板
cp xiaozhi-esp32-server-main/main/xiaozhi-server/config_from_api.yaml server-data/data/.config.yaml
```

编辑 `server-data/data/.config.yaml`，`manager-api.url` 保持默认（容器内网地址即可）：

```yaml
manager-api:
  url: http://xiaozhi-esp32-server-web:8002/xiaozhi
  secret: 这里先留空，启动后从智控台获取
```

### 第 3 步：启动

```bash
docker compose up -d --build
```

> 首次构建：server 镜像约 5 分钟；web 镜像（前端 npm + 后端 Maven）约 10-30 分钟。构建完成后：

1. 打开智控台：`http://服务器IP:8002`，**注册第一个账号**（自动成为超级管理员）
2. 登录后进入 **参数管理**，找到 `server.secret`，把参数值复制到 `server-data/data/.config.yaml` 的 `manager-api.secret`
3. 重启 server 容器：`docker restart xiaozhi-esp32-server`
4. 浏览器访问 `http://服务器IP:8002/xiaozhi/ota/`，显示"OTA接口运行正常"即完成

---

## 固件编译与烧录

> 💡 不想编译？直接用预编译固件：`releases/movecall-moji-esp32s3_99.0.0.bin`（含全部定制，0x0 直接烧录，见 `releases/README.md`）

```bash
cd xiaozhi-esp32
# 需要 ESP-IDF v6.0.2 环境
python scripts/build.py movecall/moji-esp32s3 --name movecall-moji-esp32s3
# 产物：build/merged-binary.bin
```

烧录方式：
- 乐鑫 ESP Launchpad（浏览器在线烧录）：https://espressif.github.io/esp-launchpad/
- 或 esptool：`esptool.py write_flash 0x0 build/merged-binary.bin`

## 设备连接服务器

1. 设备进配网模式（长按 BOOT 开机）
2. 手机连设备热点，浏览器打开 `192.168.4.1`
3. 填写 WiFi，**高级选项 → OTA 地址**填：`http://服务器IP:8002/xiaozhi/ota/`
4. 保存重启，设备屏幕显示 6 位激活码
5. 智控台 → 设备管理 → 输入激活码绑定（建议绑定到默认智能体）

---

## 目录结构

```
├── docker-compose.yml          # Docker 一键部署入口
├── xiaozhi-esp32/              # 固件（仅 moji-esp32s3 板型）
├── xiaozhi-esp32-server-main/  # 服务端
│   ├── main/xiaozhi-server/    #   Python 核心引擎（含定制改动）
│   ├── main/manager-web/       #   智控台前端（修改版 UI）
│   ├── main/manager-api/       #   智控台后端（Java）
│   └── Dockerfile-server/web   #   构建文件
├── server-data/                # 运行数据（部署时创建，不入库）
└── platform/                   # 设备操控网页（可选）
```

## 常见问题

| 问题 | 解决 |
|---|---|
| 天气播报失败 | 默认已用 Open-Meteo（免费无需 key）；如需城市默认值改智控台参数 |
| 设备连不上 | 确认 8000/8002 端口开放、设备与服务器同网段 |
| 对话无声音 | 检查智控台模型配置：TTS 选 EdgeTTS（免费）、ASR 选 FunASR（本地） |
| 记忆不生效 | 智控台模型配置 → 记忆选 mem_local_short（用 DeepSeek 总结） |

## 许可

固件/服务端沿用上游 MIT 协议。
