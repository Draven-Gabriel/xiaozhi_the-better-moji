export function initSerialMonitorPage() {
  const $ = (id) => document.getElementById(id);
  const baudSelect = $("monBaud");
  const connectBtn = $("monConnect");
  const disconnectBtn = $("monDisconnect");
  const startBtn = $("monStart");
  const stopBtn = $("monStop");
  const clearBtn = $("monClear");
  const sendInput = $("monSendInput");
  const sendBtn = $("monSend");
  const output = $("monOutput");

  const decoder = new TextDecoder();
  const encoder = new TextEncoder();
  let port = null;
  let reader = null;
  let running = false;

  function append(text) {
    const el = document.createElement("div");
    el.textContent = text;
    output.appendChild(el);
    output.scrollTop = output.scrollHeight;
  }

  function setConnected(on) {
    connectBtn.disabled = on;
    disconnectBtn.disabled = !on;
    startBtn.disabled = !on || running;
    stopBtn.disabled = !on || !running;
    sendBtn.disabled = !on;
  }

  async function connect() {
    try {
      if (!navigator.serial) {
        append("当前浏览器不支持 Web Serial，请使用 Chrome/Edge。");
        return;
      }
      port = await navigator.serial.requestPort();
      await port.open({ baudRate: parseInt(baudSelect.value, 10) });
      append(`串口已连接，波特率 ${baudSelect.value}`);
      setConnected(true);
    } catch (e) {
      append(`连接失败: ${e.message || e}`);
    }
  }

  async function readLoop() {
    while (running && port && port.readable) {
      try {
        reader = port.readable.getReader();
        while (running) {
          const { value, done } = await reader.read();
          if (done) break;
          append(decoder.decode(value, { stream: true }));
        }
        try {
          reader.releaseLock();
        } catch (e) {
          // 锁可能已经被取消释放
        }
      } catch (e) {
        if (running) append(`读取中断: ${e.message || e}`);
        break;
      }
    }
  }

  async function start() {
    if (!port || running) return;
    running = true;
    setConnected(true);
    append("开始读取串口数据...");
    await readLoop();
    if (running) {
      running = false;
      setConnected(true);
    }
  }

  async function stop() {
    running = false;
    if (reader) {
      try {
        await reader.cancel();
      } catch (e) {
        // 忽略取消错误
      }
    }
    setConnected(true);
    append("已停止读取。");
  }

  async function sendLine() {
    if (!port || !port.writable) return;
    const text = sendInput.value;
    if (!text) return;
    try {
      const writer = port.writable.getWriter();
      await writer.write(encoder.encode(text + "\r\n"));
      writer.releaseLock();
      append(`→ ${text}`);
      sendInput.value = "";
    } catch (e) {
      append(`发送失败: ${e.message || e}`);
    }
  }

  async function disconnect() {
    await stop();
    if (port) {
      try {
        await port.close();
        append("串口已断开。");
      } catch (e) {
        append(`断开失败: ${e.message || e}`);
      }
    }
    port = null;
    setConnected(false);
  }

  connectBtn.addEventListener("click", connect);
  disconnectBtn.addEventListener("click", disconnect);
  startBtn.addEventListener("click", start);
  stopBtn.addEventListener("click", stop);
  clearBtn.addEventListener("click", () => {
    output.replaceChildren();
  });
  sendBtn.addEventListener("click", sendLine);
  sendInput.addEventListener("keydown", (e) => {
    if (e.key === "Enter") sendLine();
  });

  setConnected(false);
}
