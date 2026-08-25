# ============================================
# 本地源码启动 xiaozhi-server（调试模式）
# 依赖：venv（Python 3.11）+ micromamba 环境的 opus.dll
# 前置：Docker 里的智控台（web/db/redis）需在运行；
#        Docker 里的 xiaozhi-esp32-server 容器需已停止（避免端口冲突）
# ============================================

# 1. 把 opus/ffmpeg 动态库加入 PATH
$env:PATH = "D:\xiaozhi-esp32-server\mamba\envs\xiaozhi\Library\bin;$env:PATH"

# 2. 进入服务器源码目录
Set-Location "D:\vibe coding project\embedded\Tabletop conversational robot\new version 1.1\xiaozhi-esp32-server-main\main\xiaozhi-server"

# 3. 启动（Ctrl+C 停止）
& "D:\xiaozhi-esp32-server\venv\Scripts\python.exe" app.py
