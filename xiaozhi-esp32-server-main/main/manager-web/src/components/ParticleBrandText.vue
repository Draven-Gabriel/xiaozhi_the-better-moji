<template>
  <canvas ref="canvas" class="pbt-canvas" aria-hidden="true"></canvas>
</template>

<script>
const FONT_STACK =
  '-apple-system, BlinkMacSystemFont, "SF Pro Display", "SF Pro Text", "Helvetica Neue", "PingFang SC", "Microsoft YaHei", "Segoe UI", sans-serif';
const FONT_WEIGHT = 600;
const MAX_PARTICLES = 280;
const HIGHLIGHT_RATIO = 0.15;

export default {
  name: 'ParticleBrandText',
  props: {
    text: { type: String, default: 'Console' },
    fontSize: { type: Number, default: 30 },
    density: { type: Number, default: 4 },
    particleSize: { type: Number, default: 1.8 },
    scatter: { type: Number, default: 70 },
    gatherDuration: { type: Number, default: 1700 },
    stagger: { type: Number, default: 360 },
    repelRadius: { type: Number, default: 80 },
    repelStrength: { type: Number, default: 0.28 },
    drift: { type: Number, default: 0.7 },
    primaryColor: { type: String, default: '#1d1d1f' },
    highlightColor: { type: String, default: 'rgba(0,113,227,0.9)' },
    glow: { type: Boolean, default: true },
    padding: { type: Number, default: 10 },
  },
  data() {
    return {
      reducedMotion: false,
    };
  },
  mounted() {
    this._s = {
      ctx: null,
      particles: null,
      rx: null,
      ry: null,
      width: 0,
      height: 0,
      cssWidth: 0,
      cssHeight: 0,
      dpr: 1,
      startTime: 0,
      rafId: 0,
      running: false,
      staticDrawn: false,
      mouse: { x: -1e4, y: -1e4, active: false, lastT: 0 },
      docVisible: true,
    };

    this.dpr = Math.min(window.devicePixelRatio || 1, 2);

    const mq = window.matchMedia('(prefers-reduced-motion: reduce)');
    this._mq = mq;
    this.reducedMotion = mq.matches;
    this._mqListener = (ev) => {
      this.reducedMotion = ev.matches;
      if (this.reducedMotion) {
        this.stopLoop();
        this.drawStatic();
      }
    };
    if (mq.addEventListener) mq.addEventListener('change', this._mqListener);
    else mq.addListener(this._mqListener);

    this._mouseListener = this.onMouseMove.bind(this);
    window.addEventListener('mousemove', this._mouseListener, { passive: true });

    this._visibilityListener = () => {
      this._s.docVisible = !document.hidden;
      if (this._s.docVisible && !this.reducedMotion) this.startLoop();
    };
    document.addEventListener('visibilitychange', this._visibilityListener);

    this.bootstrap();
  },
  beforeDestroy() {
    this.stopLoop();
    if (this._mouseListener) window.removeEventListener('mousemove', this._mouseListener);
    if (this._visibilityListener) document.removeEventListener('visibilitychange', this._visibilityListener);
    if (this._mq && this._mqListener) {
      if (this._mq.removeEventListener) this._mq.removeEventListener('change', this._mqListener);
      else this._mq.removeListener(this._mqListener);
    }
  },
  methods: {
    async bootstrap() {
      const c = this.$refs.canvas;
      if (!c) return;
      this._s.ctx = c.getContext('2d');
      try {
        if (document.fonts && document.fonts.ready) {
          await document.fonts.ready;
        }
      } catch (e) {
      }

      let density = this.density;
      let measured;
      for (let i = 0; i < 8; i++) {
        measured = this.measureText(density);
        if (measured.points.length > 0 && measured.points.length <= MAX_PARTICLES) break;
        density += 1;
      }
      this._densityUsed = density;
      this._s.width = measured.width;
      this._s.height = measured.height;
      this._s.cssWidth = measured.width + this.padding * 2;
      this._s.cssHeight = measured.height + this.padding * 2;
      c.width = Math.round(this._s.cssWidth * this.dpr);
      c.height = Math.round(this._s.cssHeight * this.dpr);
      c.style.width = this._s.cssWidth + 'px';
      c.style.height = this._s.cssHeight + 'px';
      this._s.ctx.setTransform(this.dpr, 0, 0, this.dpr, 0, 0);

      this.buildParticles(measured.points);

      if (this.reducedMotion) {
        this.drawStatic();
        return;
      }
      this._s.startTime = performance.now();
      this.startLoop();
    },

    measureText(density) {
      const px = document.createElement('canvas').getContext('2d');
      px.font = `${FONT_WEIGHT} ${this.fontSize}px ${FONT_STACK}`;
      px.textBaseline = 'middle';
      const m = px.measureText(this.text);
      const textW = Math.max(1, Math.ceil(m.width));
      const textH = Math.max(1, Math.ceil(this.fontSize * 1.25));
      const off = document.createElement('canvas');
      off.width = textW;
      off.height = textH;
      const ctx = off.getContext('2d');
      ctx.fillStyle = '#000';
      ctx.font = `${FONT_WEIGHT} ${this.fontSize}px ${FONT_STACK}`;
      ctx.textBaseline = 'middle';
      ctx.fillText(this.text, 0, textH / 2);
      const data = ctx.getImageData(0, 0, textW, textH).data;
      const points = [];
      for (let y = 0; y < textH; y += density) {
        for (let x = 0; x < textW; x += density) {
          const idx = (y * textW + x) * 4;
          if (data[idx + 3] > 128) {
            const jitterX = (Math.random() - 0.5) * density * 0.5;
            const jitterY = (Math.random() - 0.5) * density * 0.5;
            points.push({ x: x + jitterX, y: y + jitterY });
          }
        }
      }
      return { width: textW, height: textH, points };
    },

    buildParticles(points) {
      const n = points.length;
      const arr = new Array(n);
      const staggerMax = this.stagger;
      const scatter = this.scatter;
      const pad = this.padding;
      for (let i = 0; i < n; i++) {
        const p = points[i];
        const angle = Math.random() * Math.PI * 2;
        const radius = scatter * (0.6 + Math.random() * 0.7);
        arr[i] = {
          tx: p.x + pad,
          ty: p.y + pad,
          x: p.x + pad + Math.cos(angle) * radius,
          y: p.y + pad + Math.sin(angle) * radius,
          vx: (Math.random() - 0.5) * 1.4,
          vy: (Math.random() - 0.5) * 1.4,
          phase: Math.random() * Math.PI * 2,
          phaseY: Math.random() * Math.PI * 2,
          stagger: Math.random() * staggerMax,
          highlight: Math.random() < HIGHLIGHT_RATIO,
        };
      }
      this._s.particles = arr;
      this._s.rx = new Float32Array(n);
      this._s.ry = new Float32Array(n);
    },

    startLoop() {
      if (this._s.running) return;
      if (this.reducedMotion) { this.drawStatic(); return; }
      this._s.running = true;
      this._s.rafId = requestAnimationFrame(this.tick);
    },

    stopLoop() {
      this._s.running = false;
      if (this._s.rafId) {
        cancelAnimationFrame(this._s.rafId);
        this._s.rafId = 0;
      }
    },

    tick(now) {
      if (!this._s.running) return;
      if (!this._s.docVisible) {
        this.stopLoop();
        return;
      }
      this.draw(now);
      this._s.rafId = requestAnimationFrame(this.tick);
    },

    draw(now) {
      const s = this._s;
      const ctx = s.ctx;
      const cssW = s.cssWidth;
      const cssH = s.cssHeight;
      ctx.clearRect(0, 0, cssW, cssH);

      const elapsed = now - s.startTime;
      const t = elapsed / 1000;
      const mouseAge = performance.now() - s.mouse.lastT;
      const mouseActive = s.mouse.active && mouseAge < 700;

      const px = s.particles;
      const rxArr = s.rx;
      const ryArr = s.ry;
      const n = px.length;
      const ease = 0.085;
      const damp = 0.86;
      const repR = this.repelRadius;
      const repR2 = repR * repR;
      const repS = this.repelStrength * 28;
      const mx = s.mouse.x;
      const my = s.mouse.y;
      const drift = this.drift;
      const gather = this.gatherDuration;

      for (let i = 0; i < n; i++) {
        const p = px[i];
        if (elapsed >= p.stagger) {
          let vx = p.vx + (p.tx - p.x) * ease;
          let vy = p.vy + (p.ty - p.y) * ease;
          if (mouseActive) {
            const dx = p.x - mx;
            const dy = p.y - my;
            const d2 = dx * dx + dy * dy;
            if (d2 < repR2) {
              const d = Math.sqrt(d2) || 1;
              const fall = 1 - d / repR;
              const f = fall * fall;
              vx += (dx / d) * f * repS;
              vy += (dy / d) * f * repS;
            }
          }
          vx *= damp;
          vy *= damp;
          p.vx = vx;
          p.vy = vy;
          p.x += vx;
          p.y += vy;
        }
        const local = (elapsed - p.stagger) / gather;
        const settle = local < 0 ? 0 : local > 1 ? 1 : local;
        const amp = drift * settle;
        rxArr[i] = p.x + Math.sin(t * 0.6 + p.phase) * amp;
        ryArr[i] = p.y + Math.cos(t * 0.55 + p.phaseY) * amp * 0.6;
      }

      const sz = this.particleSize;
      const half = sz * 0.5;

      ctx.fillStyle = this.primaryColor;
      if (this.glow) {
        ctx.shadowColor = 'rgba(29,29,31,0.28)';
        ctx.shadowBlur = 3;
      }
      for (let i = 0; i < n; i++) {
        if (px[i].highlight) continue;
        ctx.fillRect(rxArr[i] - half, ryArr[i] - half, sz, sz);
      }

      if (this.glow) {
        ctx.shadowColor = this.highlightColor;
        ctx.shadowBlur = 7;
      }
      ctx.fillStyle = this.highlightColor;
      for (let i = 0; i < n; i++) {
        if (!px[i].highlight) continue;
        ctx.fillRect(rxArr[i] - half, ryArr[i] - half, sz, sz);
      }

      ctx.shadowBlur = 0;
      ctx.shadowColor = 'transparent';
    },

    drawStatic() {
      const s = this._s;
      if (!s.ctx || s.staticDrawn) return;
      const ctx = s.ctx;
      ctx.clearRect(0, 0, s.cssWidth, s.cssHeight);
      const px = s.particles;
      const n = px.length;
      const sz = this.particleSize;
      const half = sz * 0.5;

      ctx.fillStyle = this.primaryColor;
      if (this.glow) {
        ctx.shadowColor = 'rgba(29,29,31,0.28)';
        ctx.shadowBlur = 3;
      }
      for (let i = 0; i < n; i++) {
        if (px[i].highlight) continue;
        ctx.fillRect(px[i].tx - half, px[i].ty - half, sz, sz);
      }

      if (this.glow) {
        ctx.shadowColor = this.highlightColor;
        ctx.shadowBlur = 7;
      }
      ctx.fillStyle = this.highlightColor;
      for (let i = 0; i < n; i++) {
        if (!px[i].highlight) continue;
        ctx.fillRect(px[i].tx - half, px[i].ty - half, sz, sz);
      }

      ctx.shadowBlur = 0;
      ctx.shadowColor = 'transparent';
      s.staticDrawn = true;
    },

    onMouseMove(e) {
      const c = this.$refs.canvas;
      if (!c || !this._s) return;
      const r = c.getBoundingClientRect();
      this._s.mouse.x = e.clientX - r.left;
      this._s.mouse.y = e.clientY - r.top;
      this._s.mouse.active = true;
      this._s.mouse.lastT = performance.now();
    },
  },
};
</script>

<style lang="scss" scoped>
.pbt-canvas {
  display: block;
  pointer-events: none;
  vertical-align: middle;
}
</style>
