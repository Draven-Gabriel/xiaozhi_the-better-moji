# 固件发布

## 预编译固件（直接烧录）

| 文件 | 版本 | 说明 |
|---|---|---|
| `movecall-moji-esp32s3_99.0.0.bin` | 99.0.0 | Moji 全功能版（barge-in 打断 + AEC + 像素表情 UI），合并固件，起始地址 0x0 直接烧录 |

## 烧录方法

### 方法一：ESP Launchpad（浏览器在线烧录，推荐）

1. 打开 https://espressif.github.io/esp-launchpad/
2. USB 连接 Moji（数据线），选 COM 口
3. 选择 `movecall-moji-esp32s3_99.0.0.bin`，起始地址 0x0，烧录

### 方法二：esptool

```bash
esptool.py --port COM3 write_flash 0x0 movecall-moji-esp32s3_99.0.0.bin
```

## 从源码编译（最新代码）

```bash
cd xiaozhi-esp32
# 需要 ESP-IDF v6.0.2
python scripts/build.py movecall/moji-esp32s3 --name movecall-moji-esp32s3
# 产物：build/merged-binary.bin（全量烧录）和 build/xiaozhi.bin（OTA 用）
```

## 注意事项

- 固件版本 99.0.0 高于官方所有版本，**不会被官方 OTA 覆盖**
- 首次使用需配网：长按 BOOT 进入配网模式，OTA 地址填你的服务器
- 自定义 OTA 推送：把 `build/xiaozhi.bin` 命名为 `movecall-moji-esp32s3_99.0.1.bin` 放到服务器 `server-data/data/bin/` 下（版本必须递增）
