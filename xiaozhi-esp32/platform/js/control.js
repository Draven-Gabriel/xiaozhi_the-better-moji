import { requestApproval, isReadonlyTool, isDangerTool } from "./permission.js";

export function initControlPage() {
  const $ = (id) => document.getElementById(id);
  const deviceIp = $("deviceIp");
  const devicePort = $("devicePort");
  const connectBtn = $("ctrlConnect");
  const disconnectBtn = $("ctrlDisconnect");
  const wsStatus = $("wsStatus");
  const refreshStatusBtn = $("refreshStatus");
  const statusCards = $("statusCards");
  const controlLog = $("controlLog");
  const toolList = $("toolList");
  const toolSearch = $("toolSearch");
  const hideServo = $("hideServo");
  const rawJson = $("rawJson");
  const rawSend = $("rawSend");

  const volumeSlider = $("volumeSlider");
  const volumeValue = $("volumeValue");
  const setVolumeBtn = $("setVolume");
  const brightnessSlider = $("brightnessSlider");
  const brightnessValue = $("brightnessValue");
  const setBrightnessBtn = $("setBrightness");
  const themeSelect = $("themeSelect");
  const setThemeBtn = $("setTheme");
  const previewUrl = $("previewUrl");
  const previewImageBtn = $("previewImage");
  const systemInfoBtn = $("systemInfoBtn");
  const rebootBtn = $("rebootBtn");

  const state = {
    ws: null,
    nextId: 1,
    pending: new Map(),
    tools: [],
  };

  const SERVO_TOOLS = new Set([
    "self.electron.hand_action",
    "self.electron.body_turn",
    "self.electron.head_move",
    "self.electron.servo_move",
    "self.electron.servo_sequences",
    "self.electron.set_trim",
    "self.electron.get_trims",
    "self.electron.home",
    "self.electron.stop",
  ]);

  function escapeHtml(text) {
    return String(text)
      .replaceAll("&", "&amp;")
      .replaceAll("<", "&lt;")
      .replaceAll(">", "&gt;")
      .replaceAll('"', "&quot;");
  }

  function formatValue(value) {
    if (value === undefined || value === null) return String(value);
    if (typeof value === "string") return value;
    return JSON.stringify(value, null, 2);
  }

  function log(text, data) {
    const line = document.createElement("div");
    let content = `[${new Date().toLocaleTimeString()}] ${text}`;
    if (data !== undefined) {
      content += ` ${JSON.stringify(data)}`;
    }
    line.textContent = content;
    controlLog.appendChild(line);
    controlLog.scrollTop = controlLog.scrollHeight;
  }

  function setWsStatus(text, cls) {
    wsStatus.textContent = text;
    wsStatus.className = `badge ${cls}`;
  }

  function enableControls(on) {
    connectBtn.disabled = on;
    disconnectBtn.disabled = !on;
    refreshStatusBtn.disabled = !on;
    setVolumeBtn.disabled = !on;
    setBrightnessBtn.disabled = !on;
    setThemeBtn.disabled = !on;
    previewImageBtn.disabled = !on;
    systemInfoBtn.disabled = !on;
    rebootBtn.disabled = !on;
    rawSend.disabled = !on;
  }

  function send(method, params = {}) {
    return new Promise((resolve, reject) => {
      if (!state.ws || state.ws.readyState !== WebSocket.OPEN) {
        reject(new Error("WebSocket 未连接"));
        return;
      }
      const id = state.nextId++;
      state.pending.set(id, { resolve, reject, method });
      state.ws.send(JSON.stringify({ jsonrpc: "2.0", method, params, id }));
      setTimeout(() => {
        if (state.pending.has(id)) {
          state.pending.delete(id);
          reject(new Error(`请求超时: ${method}`));
        }
      }, 10000);
    });
  }

  async function callTool(name, args = {}) {
    const ok = await requestApproval({
      kind: name,
      title: "设备操作确认",
      danger: isDangerTool(name),
      readonly: isReadonlyTool(name),
      detail: `工具: ${name}\n参数: ${JSON.stringify(args || {}, null, 2)}`,
      rememberKey: isReadonlyTool(name) ? null : name,
    });
    if (!ok) throw new Error("操作已被拒绝");
    return send("tools/call", { name, arguments: args });
  }

  function handleMessage(raw) {
    let msg;
    try {
      msg = JSON.parse(raw);
    } catch (e) {
      log("收到非 JSON 文本", raw);
      return;
    }

    if (msg.id !== undefined && state.pending.has(msg.id)) {
      const p = state.pending.get(msg.id);
      state.pending.delete(msg.id);
      if (msg.error) {
        log(`✗ ${p.method}: ${msg.error.message || JSON.stringify(msg.error)}`);
        p.reject(new Error(msg.error.message || "RPC 错误"));
        return;
      }
      if (p.method === "tools/list" && msg.result && Array.isArray(msg.result.tools)) {
        state.tools = msg.result.tools;
        renderTools();
      }
      log(`✓ ${p.method}`, msg.result);
      p.resolve(msg.result);
      return;
    }

    log("收到通知/未知消息", msg);
  }

  function connect() {
    const ip = deviceIp.value.trim();
    const port = devicePort.value.trim() || "8080";
    if (!ip) {
      log("请先填写设备 IP");
      return;
    }
    const url = `ws://${ip}:${port}/ws`;
    setWsStatus("连接中", "mid");
    log(`正在连接 ${url}`);

    const ws = new WebSocket(url);
    state.ws = ws;

    ws.onopen = async () => {
      setWsStatus("已连接", "ok");
      enableControls(true);
      log("WebSocket 已连接", url);
      try {
        await send("tools/list", {});
        await refreshStatus();
      } catch (e) {
        log("初始化失败: " + e.message);
      }
    };

    ws.onmessage = (ev) => handleMessage(ev.data);

    ws.onclose = () => {
      setWsStatus("已断开", "bad");
      enableControls(false);
      log("WebSocket 已关闭");
      for (const p of state.pending.values()) p.reject(new Error("连接已断开"));
      state.pending.clear();
      state.ws = null;
    };

    ws.onerror = () => log("WebSocket 发生错误");
  }

  function disconnect() {
    if (state.ws) {
      state.ws.close();
    }
  }

  async function refreshStatus() {
    statusCards.innerHTML = "";
    const available = new Set(state.tools.map((t) => t.name));
    const names = [
      "self.get_device_status",
      "self.electron.get_ip",
      "self.electron.get_status",
      "self.battery.get_level",
      "self.screen.get_info",
    ];
    for (const name of names) {
      if (!available.has(name)) continue;
      const card = document.createElement("div");
      card.className = "status-card";
      card.innerHTML = `<div class="title">${escapeHtml(name)}</div><pre>加载中...</pre>`;
      statusCards.appendChild(card);
      try {
        const result = await callTool(name, {});
        card.querySelector("pre").textContent = formatValue(result);
      } catch (e) {
        card.querySelector("pre").textContent = "错误: " + e.message;
      }
    }
  }

  function renderToolCard(tool) {
    const card = document.createElement("div");
    card.className = "tool-card";

    const schema = (tool.inputSchema && tool.inputSchema.properties) || {};
    const required = new Set((tool.inputSchema && tool.inputSchema.required) || []);
    const form = document.createElement("div");
    form.className = "tool-form";

    for (const [propName, propSchema] of Object.entries(schema)) {
      const wrap = document.createElement("div");
      wrap.className = "prop";
      const label = document.createElement("label");
      label.textContent = `${propName}${required.has(propName) ? " *" : ""} (${propSchema.type || "any"})`;
      wrap.appendChild(label);

      let input;
      if (propSchema.type === "integer" || propSchema.type === "number") {
        input = document.createElement("input");
        input.type = "number";
        if (propSchema.minimum !== undefined) input.min = propSchema.minimum;
        if (propSchema.maximum !== undefined) input.max = propSchema.maximum;
        if (propSchema.default !== undefined) input.value = propSchema.default;
        input.step = propSchema.type === "number" ? "any" : "1";
      } else if (propSchema.type === "boolean") {
        input = document.createElement("input");
        input.type = "checkbox";
        if (propSchema.default === true) input.checked = true;
      } else {
        input = document.createElement("input");
        input.type = "text";
        if (propSchema.default !== undefined) input.value = propSchema.default;
      }
      input.dataset.prop = propName;
      input.dataset.propType = propSchema.type || "string";
      wrap.appendChild(input);
      form.appendChild(wrap);
    }

    const details = document.createElement("details");
    const summary = document.createElement("summary");
    summary.textContent = tool.name;
    details.appendChild(summary);
    if (tool.description) {
      const desc = document.createElement("div");
      desc.className = "desc";
      desc.textContent = tool.description;
      details.appendChild(desc);
    }
    details.appendChild(form);

    const callBtn = document.createElement("button");
    callBtn.className = "primary";
    callBtn.textContent = "调用";

    callBtn.addEventListener("click", async () => {
      const args = {};
      let missing = false;
      for (const input of form.querySelectorAll("input[data-prop]")) {
        const propName = input.dataset.prop;
        const type = input.dataset.propType;
        if (type === "boolean") {
          args[propName] = input.checked;
          continue;
        }
        const raw = input.value.trim();
        if (required.has(propName) && raw === "") {
          missing = true;
          break;
        }
        if (raw === "") continue;
        if (type === "integer" || type === "number") {
          args[propName] = Number(raw);
        } else {
          args[propName] = raw;
        }
      }
      if (missing) {
        log(`缺少必填参数: ${tool.name}`);
        return;
      }
      callBtn.disabled = true;
      try {
        const result = await callTool(tool.name, args);
        log(`工具返回: ${tool.name}`, result);
      } catch (e) {
        log(`工具调用失败: ${tool.name}: ${e.message}`);
      } finally {
        callBtn.disabled = false;
      }
    });

    card.appendChild(details);
    card.appendChild(callBtn);
    return card;
  }

  function renderTools() {
    toolList.innerHTML = "";
    const search = toolSearch.value.trim().toLowerCase();
    const hide = hideServo.checked;
    const filtered = state.tools.filter((tool) => {
      if (hide && SERVO_TOOLS.has(tool.name)) return false;
      if (search) {
        const haystack = `${tool.name} ${tool.description || ""}`.toLowerCase();
        if (!haystack.includes(search)) return false;
      }
      return true;
    });

    if (!filtered.length) {
      toolList.innerHTML = '<p class="hint">没有匹配的工具。</p>';
      return;
    }

    filtered.sort((a, b) => a.name.localeCompare(b.name));
    for (const tool of filtered) {
      toolList.appendChild(renderToolCard(tool));
    }
  }

  connectBtn.addEventListener("click", connect);
  disconnectBtn.addEventListener("click", disconnect);
  refreshStatusBtn.addEventListener("click", refreshStatus);
  hideServo.addEventListener("change", renderTools);
  toolSearch.addEventListener("input", renderTools);

  volumeSlider.addEventListener("input", () => {
    volumeValue.textContent = volumeSlider.value;
  });
  setVolumeBtn.addEventListener("click", async () => {
    try {
      await callTool("self.audio_speaker.set_volume", { volume: parseInt(volumeSlider.value, 10) });
    } catch (e) {
      log("设置音量失败: " + e.message);
    }
  });

  brightnessSlider.addEventListener("input", () => {
    brightnessValue.textContent = brightnessSlider.value;
  });
  setBrightnessBtn.addEventListener("click", async () => {
    try {
      await callTool("self.screen.set_brightness", {
        brightness: parseInt(brightnessSlider.value, 10),
      });
    } catch (e) {
      log("设置亮度失败: " + e.message);
    }
  });

  setThemeBtn.addEventListener("click", async () => {
    try {
      await callTool("self.screen.set_theme", { theme: themeSelect.value });
    } catch (e) {
      log("设置主题失败: " + e.message);
    }
  });

  previewImageBtn.addEventListener("click", async () => {
    const url = previewUrl.value.trim();
    if (!url) {
      log("请先填写图片 URL");
      return;
    }
    try {
      await callTool("self.screen.preview_image", { url });
    } catch (e) {
      log("预览图片失败: " + e.message);
    }
  });

  systemInfoBtn.addEventListener("click", async () => {
    try {
      await callTool("self.get_system_info", {});
    } catch (e) {
      log("获取系统信息失败: " + e.message);
    }
  });

  rebootBtn.addEventListener("click", async () => {
    if (!window.confirm("确定要重启设备吗？")) return;
    try {
      await callTool("self.reboot", {});
    } catch (e) {
      log("重启失败: " + e.message);
    }
  });

  rawSend.addEventListener("click", async () => {
    try {
      const msg = JSON.parse(rawJson.value);
      if (!state.ws || state.ws.readyState !== WebSocket.OPEN) {
        log("未连接，无法发送");
        return;
      }
      const ok = await requestApproval({
        kind: "raw",
        title: "发送原始 JSON-RPC 消息",
        danger: true,
        detail: rawJson.value,
      });
      if (!ok) {
        log("已拒绝发送");
        return;
      }
      state.ws.send(JSON.stringify(msg));
      log("已发送原始消息", msg);
    } catch (e) {
      log("JSON 解析失败: " + e.message);
    }
  });

  enableControls(false);
}
