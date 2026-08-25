import { initControlPage } from "./control.js";
import { initSerialMonitorPage } from "./serial-monitor.js";
import { initPermission } from "./permission.js";

function initTabs() {
  const tabs = document.querySelectorAll(".tab");
  const panels = document.querySelectorAll(".panel");
  tabs.forEach((tab) => {
    tab.addEventListener("click", () => {
      tabs.forEach((t) => t.classList.remove("active"));
      panels.forEach((p) => p.classList.remove("active"));
      tab.classList.add("active");
      document.getElementById(tab.dataset.tab).classList.add("active");
    });
  });
}

initTabs();
initPermission();
initControlPage();
initSerialMonitorPage();
