let modeEl = null;
let modalEl = null;
let titleEl = null;
let detailEl = null;
let rememberCheck = null;
let allowBtn = null;
let denyBtn = null;

const remembered = new Set();
let pending = null;

function hideModal() {
  if (modalEl) modalEl.style.display = "none";
}

export function initPermission() {
  modeEl = document.getElementById("permissionMode");
  modalEl = document.getElementById("approveModal");
  titleEl = document.getElementById("approveTitle");
  detailEl = document.getElementById("approveDetail");
  rememberCheck = document.getElementById("approveRemember");
  allowBtn = document.getElementById("approveAllow");
  denyBtn = document.getElementById("approveDeny");

  allowBtn.addEventListener("click", () => {
    const p = pending;
    if (p && rememberCheck.checked) remembered.add(p.key);
    pending = null;
    hideModal();
    if (p) p.resolve(true);
  });

  denyBtn.addEventListener("click", () => {
    const p = pending;
    pending = null;
    hideModal();
    if (p) p.resolve(false);
  });
}

function currentMode() {
  return modeEl ? modeEl.value : "ask";
}

export function isReadonlyTool(name) {
  if (!name) return true;
  return /^(self\.(get_|electron\.get_|battery\.|screen\.get_info)|tools\/list)/.test(name);
}

export function isDangerTool(name) {
  return name === "self.reboot" || name === "self.upgrade_firmware";
}

export function requestApproval(options = {}) {
  const {
    kind = "operation",
    title = "操作确认",
    detail = "",
    danger = false,
    readonly = false,
    rememberKey = null,
  } = options;

  return new Promise((resolve) => {
    if (readonly || currentMode() === "off") {
      resolve(true);
      return;
    }
    if (currentMode() === "danger" && !danger) {
      resolve(true);
      return;
    }
    if (rememberKey && remembered.has(rememberKey)) {
      resolve(true);
      return;
    }
    if (!modalEl) {
      resolve(true);
      return;
    }

    if (pending) {
      const old = pending;
      pending = null;
      old.resolve(false);
    }
    titleEl.textContent = title || "操作确认";
    detailEl.textContent = detail || kind;
    rememberCheck.checked = false;
    pending = { key: rememberKey || kind, resolve };
    modalEl.style.display = "flex";
  });
}
