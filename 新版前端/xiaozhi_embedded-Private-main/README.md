# xiaozhi Web 控制台前端

小智（xiaozhi）服务端的 PC Web 管理控制台（8001 端口），基于 **Vue 2 + Element UI + Vue CLI 5** 构建。

本仓库从 [xinnan-tech/xiaozhi-esp32-server](https://github.com/xinnan-tech/xiaozhi-esp32-server)（MIT）的 `main/manager-web` 抽取而来，包含登录/注册页面的全新定制 UI（GSAP/OGL 动效）、Tailwind 及自定义组件库等二次开发。

## 整体架构

前端控制台（本仓库）依赖 `manager-api`（Java，8002 端口）提供后端接口，接口访问由开发服务器代理：

```
manager-web (本仓库, 8001 端口, Vue)  ── /xiaozhi 代理 ──►  manager-api (8002 端口, Java)
```

## 技术栈

- Vue 2.6.14 / vue-router / vuex / vue-i18n（zh、en、de、pt-BR、vi、zh-TW）
- Element UI 2.15
- Vue CLI 5（webpack、workbox PWA、gzip 压缩、可选 CDN 构建）
- GSAP、OGL（登录页动效）、TailwindCSS（自定义组件样式）
- flyio（HTTP 请求）、opus-decoder/recorder（音频处理）、sm-crypto（加密）

## 快速开始

```bash
# 安装依赖
npm install

# 本地开发（端口 8001，/xiaozhi 代理到本机 8002 的 manager-api）
npm run serve

# 生产构建（输出到 dist/）
npm run build

# 单元测试
npm run test:unit

# i18n 缺失校验
npm run check:i18n
```

## 环境变量

| 文件 | 变量 | 说明 |
| --- | --- | --- |
| `.env` | `VUE_APP_TITLE` / `VUE_APP_DESCRIPTION` / `VUE_APP_KEYWORDS` | 页面标题与 SEO 信息 |
| `.env.production` | `VUE_APP_API_BASE_URL=/xiaozhi` | 生产环境 API 前缀 |
| `.env.production` | `VUE_APP_USE_CDN` | 是否开启 CDN 构建（`true` 时通过 unpkg 加载 Vue/Element 等，需修改 `vue.config.js` 中的 `cdnResources`） |
| `.env.development` | `VUE_APP_API_BASE_URL=/xiaozhi` | 开发环境 API 前缀 |

> `.env.development` 会被复制到本机使用，但已被 `.gitignore` 排除，不会提交。

## 目录结构

```
├─ public/          # 静态资源（含网页版唤醒词/字库生成器所需模型与字体，约 80MB）
├─ scripts/         # npm 脚本（check-i18n 等）
├─ src/
│  ├─ apis/         # 后端接口封装（对接 manager-api）
│  ├─ components/   # 组件（对话框、表格、动效组件等）
│  ├─ i18n/         # 多语言文案
│  ├─ router/       # 路由与登录守卫
│  ├─ store/        # vuex 状态
│  ├─ styles/       # 全局样式、Tailwind、设计变量
│  ├─ utils/        # 工具（gsap 注册等）
│  └─ views/        # 页面（登录/注册/设备/模型/知识库/音色/声纹/OTA 等）
└─ tests/           # node:test 编写的契约测试
```

## 许可证

MIT，上游保留 `xinnan-tech` 版权声明，详见 [LICENSE](LICENSE)。
