# 单模块部署固件OTA自动升级配置指南

本教程将指导你如何在**单模块部署**场景下配置固件OTA自动升级功能，实现设备固件的自动更新。

如果你已经使用**全模块部署**，请忽略本教程。

## 功能介绍

在单模块部署中，xiaozhi-server内置了OTA固件管理功能，可以自动检测设备版本并下发升级固件。系统会根据设备型号和当前版本，自动匹配并推送最新的固件版本。

## 前提条件

- 你已经成功进行**单模块部署**并运行xiaozhi-server
- 设备能够正常连接到服务器

## 第一步 准备固件文件

### 1. 创建固件存放目录

固件文件需要放在`data/bin/`目录下。如果该目录不存在，请手动创建：

```bash
mkdir -p data/bin
```

### 2. 固件文件命名规则

固件文件必须遵循以下命名格式：

```
{设备型号}_{版本号}.bin
```

**命名规则说明：**
- `设备型号`：设备的型号名称，例如 `lichuang-dev`、`bread-compact-wifi` 等
- `版本号`：固件版本号，必须以数字开头，支持数字、字母、点号、下划线和短横线，例如 `1.6.6`、`2.0.0` 等
- 文件扩展名必须是 `.bin`

**命名示例：**
```
bread-compact-wifi_1.6.6.bin
lichuang-dev_2.0.0.bin
```

### 3. 放置固件文件

将准备好的固件文件（.bin文件）复制到`data/bin/`目录下：

重要的事情说三遍：升级的bin文件是`xiaozhi.bin`，不是全量固件文件`merged-binary.bin`!

重要的事情说三遍：升级的bin文件是`xiaozhi.bin`，不是全量固件文件`merged-binary.bin`!

重要的事情说三遍：升级的bin文件是`xiaozhi.bin`，不是全量固件文件`merged-binary.bin`!

```bash
cp xiaozhi.bin data/bin/设备型号_版本号.bin
```

例如：
```bash
cp xiaozhi.bin data/bin/bread-compact-wifi_1.6.6.bin
```

## 第二步 配置公网访问地址（仅公网部署需要）

**注意：此步骤仅适用于单模块公网部署的场景。**

如果你的xiaozhi-server是公网部署（使用公网IP或域名），**必须**配置`server.vision_explain`参数，因为OTA固件下载地址会使用该配置的域名和端口。

如果你是局域网部署，可以跳过此步骤。

### 为什么要配置这个参数？

在单模块部署中，系统生成固件下载地址时，会使用`vision_explain`配置的域名和端口作为基础地址。如果不配置或配置错误，设备将无法访问固件下载地址。

### 配置方法

打开`data/.config.yaml`文件，找到`server`配置段，设置`vision_explain`参数：

```yaml
server:
  vision_explain: http://你的域名或IP:端口号/mcp/vision/explain
```

**配置示例：**

局域网部署（默认）：
```yaml
server:
  vision_explain: http://192.168.1.100:8003/mcp/vision/explain
```

公网域名部署：
```yaml
server:
  vision_explain: http://yourdomain.com:8003/mcp/vision/explain
```

### 注意事项

- 域名或IP必须是设备能够访问的地址
- 如果使用Docker部署，不能使用Docker内部地址（如127.0.0.1或localhost）
- 如果你使用了nginx反向代理，请填写对外的地址和端口号，不是本项目运行的端口号


## 常见问题

### 1. 设备收不到固件更新

**可能原因和解决方法：**

- 检查固件文件命名是否符合规则：`{型号}_{版本号}.bin`
- 检查固件文件是否正确放置在`data/bin/`目录
- 检查设备型号是否与固件文件名中的型号匹配
- 检查固件版本号是否高于设备当前版本
- 查看服务器日志，确认OTA请求是否正常处理

### 2. 设备报告下载地址无法访问

**可能原因和解决方法：**

- 检查`server.vision_explain`配置的域名或IP是否正确
- 确认端口号配置正确（默认8003）
- 如果是公网部署，确保设备能够访问该公网地址
- 如果是Docker部署，确保不是使用了内部地址（127.0.0.1）
- 检查防火墙是否开放了对应端口
- 如果你使用了nginx反向代理，请填写对外的地址和端口号，不是本项目运行的端口号

### 3. 如何确认设备当前版本

查看OTA请求日志，日志中会显示设备上报的版本号：

```
[ota_handler] - 设备 AA:BB:CC:DD:EE:FF 固件已是最新: 1.6.6
```

### 4. 固件文件放置后没有生效

系统有30秒的缓存时间（默认），可以：
- 等待30秒后再让设备发起OTA请求
- 重启xiaozhi-server服务
- 调整`firmware_cache_ttl`配置为更短的时间
