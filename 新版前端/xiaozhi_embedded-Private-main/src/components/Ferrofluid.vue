// Ferrofluid —— 移植自 React Bits（reactbits.dev），将 React 组件改写为 Vue 2 组件
// 基于 ogl 的流体铁屑效果：若干轮廓线流体层 + 光标磁吸凸起，支持颜色按高度分布
<template>
  <div
    ref="container"
    class="ferrofluid-container"
    :class="className"
    :style="mixBlendMode ? { mixBlendMode } : undefined"
  ></div>
</template>

<script>
import { Renderer, Program, Mesh, Triangle } from 'ogl';

const MAX_COLORS = 8;

const hexToRGB = (hex) => {
  const c = String(hex || '').replace('#', '').padEnd(6, '0');
  const r = parseInt(c.slice(0, 2), 16) / 255;
  const g = parseInt(c.slice(2, 4), 16) / 255;
  const b = parseInt(c.slice(4, 6), 16) / 255;
  return [r, g, b];
};

const prepColors = (input) => {
  const base = (input && input.length ? input : ['#4F46E5', '#06B6D4', '#E0F2FE']).slice(0, MAX_COLORS);
  const count = base.length;
  const arr = [];
  for (let i = 0; i < MAX_COLORS; i++) arr.push(hexToRGB(base[Math.min(i, base.length - 1)]));
  const avg = [0, 0, 0];
  for (let i = 0; i < count; i++) {
    avg[0] += arr[i][0];
    avg[1] += arr[i][1];
    avg[2] += arr[i][2];
  }
  avg[0] /= count;
  avg[1] /= count;
  avg[2] /= count;
  return { arr, count, avg };
};

const flowVec = (d) => {
  switch (d) {
    case 'up':
      return [0, 1];
    case 'down':
      return [0, -1];
    case 'left':
      return [-1, 0];
    case 'right':
      return [1, 0];
    default:
      return [0, -1];
  }
};

const vertex = `
attribute vec2 position;
attribute vec2 uv;
varying vec2 vUv;
void main() {
  vUv = uv;
  gl_Position = vec4(position, 0.0, 1.0);
}
`;

const fragment = `
precision highp float;

uniform vec3  iResolution;
uniform vec2  iMouse;
uniform float iTime;

uniform vec3  uColor0;
uniform vec3  uColor1;
uniform vec3  uColor2;
uniform vec3  uColor3;
uniform vec3  uColor4;
uniform vec3  uColor5;
uniform vec3  uColor6;
uniform vec3  uColor7;
uniform int   uColorCount;

uniform vec3  uMouseColor;
uniform vec2  uFlow;
uniform float uSpeed;
uniform float uScale;
uniform float uTurbulence;
uniform float uFluidity;
uniform float uRimWidth;
uniform float uSharpness;
uniform float uShimmer;
uniform float uGlow;
uniform float uOpacity;
uniform float uMouseEnabled;
uniform float uMouseStrength;
uniform float uMouseRadius;

varying vec2 vUv;

#define PI 3.14159265

vec3 palette(float h) {
  int count = uColorCount;
  if (count < 1) count = 1;
  int idx = int(floor(clamp(h, 0.0, 0.999999) * float(count)));
  if (idx <= 0) return uColor0;
  if (idx == 1) return uColor1;
  if (idx == 2) return uColor2;
  if (idx == 3) return uColor3;
  if (idx == 4) return uColor4;
  if (idx == 5) return uColor5;
  if (idx == 6) return uColor6;
  return uColor7;
}

float hash(vec3 p3) {
  p3 = fract(p3 * 0.1031);
  p3 += dot(p3, p3.zyx + 33.33);
  return fract((p3.x + p3.y) * p3.z);
}

float smin(float a, float b, float k) {
  float r = exp2(-a / k) + exp2(-b / k);
  return -k * log2(r);
}

float sinlerp(float a, float b, float w) {
  return mix(a, b, (sin(w * PI - PI / 2.0) + 1.0) / 2.0);
}

float vn(vec2 p, float s, float seed) {
  vec2 cellp = floor(p / s);
  vec2 relp = mod(p, s);
  float g1 = hash(vec3(cellp, seed));
  float g2 = hash(vec3(cellp.x + 1.0, cellp.y, seed));
  float g3 = hash(vec3(cellp.x + 1.0, cellp.y + 1.0, seed));
  float g4 = hash(vec3(cellp.x, cellp.y + 1.0, seed));
  float bx = sinlerp(g1, g2, relp.x / s);
  float tx = sinlerp(g4, g3, relp.x / s);
  return sinlerp(bx, tx, relp.y / s);
}

float dbn(vec2 p, float s, float seed) {
  float o = s / 2.0;
  float n0 = vn(p, s, seed);
  float n1 = vn(p + vec2(o, o), s, seed + 0.1);
  float n2 = vn(p + vec2(-o, o), s, seed + 0.2);
  float n3 = vn(p + vec2(o, -o), s, seed + 0.3);
  float n4 = vn(p + vec2(-o, -o), s, seed + 0.4);
  return (2.0 * n0 + 1.5 * n1 + 1.25 * n2 + 1.125 * n3 + n4) / 7.0;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
  float ref = 700.0 / max(uScale, 0.05);
  vec2 p = fragCoord / iResolution.y * ref;

  float spd = 200.0 * uSpeed;
  float t = iTime;

  vec2 dir = uFlow;
  vec2 perp = vec2(-dir.y, dir.x);

  float distort1 = vn(p + perp * (t * spd), 60.0, 10.0) * 50.0 * uTurbulence;
  float distort2 = vn(p - perp * (t * spd), 120.0, 15.0) * 100.0 * uTurbulence;

  float peaks = dbn(p + distort1 + dir * (t * spd * 0.5), 40.0, 1.0);
  float peaks2 = dbn(p + distort2 - dir * (t * spd * 0.5), 40.0, 0.0);

  float mapeaks = smin(peaks, peaks2, max(uFluidity, 0.001));

  float mGlow = 0.0;
  if (uMouseEnabled > 0.5) {
    vec2 mp = iMouse / iResolution.y * ref;
    float md = length(p - mp) / ref;
    float rr = max(uMouseRadius, 0.02);
    mGlow = exp(-md * md / (rr * rr)) * uMouseStrength;
  }

  float band = (uRimWidth - abs((mapeaks - 0.4) * 2.0)) * 5.0;
  float ltn = clamp(band - vn(p + dir * (t * spd * 0.5), 60.0, 12.0) * uShimmer, 0.0, 1.0);
  ltn = pow(ltn, uSharpness) * uGlow;
  ltn *= clamp(1.0 - mGlow, 0.0, 1.0);

  float h = clamp(0.5 + (peaks - peaks2) * 0.8, 0.0, 1.0);
  vec3 col = palette(h);

  vec3 outc = col * ltn;
  float a = clamp(max(outc.r, max(outc.g, outc.b)), 0.0, 1.0);
  fragColor = vec4(outc, a * uOpacity);
}

void main() {
  vec4 color;
  mainImage(color, vUv * iResolution.xy);
  gl_FragColor = color;
}
`;

const NUM_PROPS = [
  'speed', 'scale', 'turbulence', 'fluidity', 'rimWidth', 'sharpness',
  'shimmer', 'glow', 'opacity', 'mouseStrength', 'mouseRadius'
];

const PROP_TO_UNIFORM = {
  speed: 'uSpeed',
  scale: 'uScale',
  turbulence: 'uTurbulence',
  fluidity: 'uFluidity',
  rimWidth: 'uRimWidth',
  sharpness: 'uSharpness',
  shimmer: 'uShimmer',
  glow: 'uGlow',
  opacity: 'uOpacity',
  mouseStrength: 'uMouseStrength',
  mouseRadius: 'uMouseRadius'
};

const numWatchers = {};
NUM_PROPS.forEach((p) => {
  numWatchers[p] = function (v) {
    if (this._program) this._program.uniforms[PROP_TO_UNIFORM[p]].value = v;
  };
});

export default {
  name: 'Ferrofluid',
  props: {
    className: { type: String, default: '' },
    dpr: { type: Number, default: null },
    paused: { type: Boolean, default: false },
    colors: { type: Array, default: () => ['#ffffff', '#ffffff', '#ffffff'] },
    speed: { type: Number, default: 0.5 },
    scale: { type: Number, default: 1.6 },
    turbulence: { type: Number, default: 1 },
    fluidity: { type: Number, default: 0.1 },
    rimWidth: { type: Number, default: 0.2 },
    sharpness: { type: Number, default: 2.5 },
    shimmer: { type: Number, default: 1.5 },
    glow: { type: Number, default: 2 },
    flowDirection: { type: String, default: 'down' },
    opacity: { type: Number, default: 1 },
    mouseInteraction: { type: Boolean, default: true },
    mouseStrength: { type: Number, default: 1 },
    mouseRadius: { type: Number, default: 0.35 },
    mouseDampening: { type: Number, default: 0.15 },
    mixBlendMode: { type: String, default: undefined }
  },
  data() {
    return {
      reducedMotion: false
    };
  },
  computed: {
    rendererDpr() {
      if (this.dpr) return this.dpr;
      return typeof window !== 'undefined' ? window.devicePixelRatio || 1 : 1;
    }
  },
  watch: {
    ...numWatchers,
    colors() {
      this.updateColors();
    },
    flowDirection(v) {
      if (this._program) this._program.uniforms.uFlow.value = flowVec(v);
    },
    mouseInteraction(v) {
      if (this._program) this._program.uniforms.uMouseEnabled.value = v ? 1 : 0;
    }
  },
  mounted() {
    this.reducedMotion =
      typeof window !== 'undefined' &&
      typeof window.matchMedia === 'function' &&
      window.matchMedia('(prefers-reduced-motion: reduce)').matches;

    const container = this.$refs.container;
    if (!container) return;

    const renderer = new Renderer({
      dpr: this.rendererDpr,
      alpha: true,
      antialias: true
    });
    this._renderer = renderer;
    const gl = renderer.gl;
    const canvas = gl.canvas;
    gl.clearColor(0, 0, 0, 0);
    canvas.style.width = '100%';
    canvas.style.height = '100%';
    canvas.style.display = 'block';
    canvas.style.touchAction = 'none';
    container.appendChild(canvas);

    const uniforms = {
      iResolution: { value: [gl.drawingBufferWidth, gl.drawingBufferHeight, 1] },
      iMouse: { value: [0, 0] },
      iTime: { value: 0 },
      uColor0: { value: [0, 0, 0] },
      uColor1: { value: [0, 0, 0] },
      uColor2: { value: [0, 0, 0] },
      uColor3: { value: [0, 0, 0] },
      uColor4: { value: [0, 0, 0] },
      uColor5: { value: [0, 0, 0] },
      uColor6: { value: [0, 0, 0] },
      uColor7: { value: [0, 0, 0] },
      uColorCount: { value: 1 },
      uMouseColor: { value: [0.5, 0.5, 0.5] },
      uFlow: { value: flowVec(this.flowDirection) },
      uSpeed: { value: this.speed },
      uScale: { value: this.scale },
      uTurbulence: { value: this.turbulence },
      uFluidity: { value: this.fluidity },
      uRimWidth: { value: this.rimWidth },
      uSharpness: { value: this.sharpness },
      uShimmer: { value: this.shimmer },
      uGlow: { value: this.glow },
      uOpacity: { value: this.opacity },
      uMouseEnabled: { value: this.mouseInteraction ? 1 : 0 },
      uMouseStrength: { value: this.mouseStrength },
      uMouseRadius: { value: this.mouseRadius }
    };

    const program = new Program(gl, { vertex, fragment, uniforms });
    this._program = program;
    const geometry = new Triangle(gl);
    this._geometry = geometry;
    const mesh = new Mesh(gl, { geometry, program });
    this._mesh = mesh;

    this.updateColors();

    this._mouseTarget = [0, 0];
    this._lastTime = 0;

    const resize = () => {
      const rect = container.getBoundingClientRect();
      if (rect.width <= 0 || rect.height <= 0) return;
      renderer.setSize(rect.width, rect.height);
      uniforms.iResolution.value = [gl.drawingBufferWidth, gl.drawingBufferHeight, 1];
    };
    this._resize = resize;
    resize();

    if (typeof ResizeObserver !== 'undefined') {
      const ro = new ResizeObserver(resize);
      ro.observe(container);
      this._ro = ro;
    } else {
      window.addEventListener('resize', resize);
      this._resizeListener = resize;
    }

    const onPointerMove = (e) => {
      const rect = canvas.getBoundingClientRect();
      const sc = renderer.dpr || 1;
      const x = (e.clientX - rect.left) * sc;
      const y = (rect.height - (e.clientY - rect.top)) * sc;
      this._mouseTarget = [x, y];
      if (this.mouseDampening <= 0) {
        uniforms.iMouse.value = [x, y];
      }
    };
    this._onPointerMove = onPointerMove;
    if (this.mouseInteraction) {
      canvas.addEventListener('pointermove', onPointerMove);
      canvas.addEventListener('pointerdown', onPointerMove);
    }

    const loop = (t) => {
      this._raf = requestAnimationFrame(loop);
      uniforms.iTime.value = t * 0.001;
      if (this.mouseDampening > 0) {
        if (!this._lastTime) this._lastTime = t;
        const dt = (t - this._lastTime) / 1000;
        this._lastTime = t;
        const tau = Math.max(1e-4, this.mouseDampening);
        let factor = 1 - Math.exp(-dt / tau);
        if (factor > 1) factor = 1;
        const target = this._mouseTarget;
        const cur = uniforms.iMouse.value;
        cur[0] += (target[0] - cur[0]) * factor;
        cur[1] += (target[1] - cur[1]) * factor;
      } else {
        this._lastTime = t;
      }
      // 后台标签页或暂停时不渲染，节省 GPU
      if (!this.paused && !document.hidden && this._program && this._mesh) {
        try {
          renderer.render({ scene: this._mesh });
        } catch (e) {
          console.error(e);
        }
      }
    };
    this._raf = requestAnimationFrame(loop);
  },
  beforeDestroy() {
    if (this._raf) cancelAnimationFrame(this._raf);
    const container = this.$refs.container;
    if (container && this._renderer) {
      const canvas = this._renderer.gl && this._renderer.gl.canvas;
      if (canvas && canvas.parentElement === container) {
        container.removeChild(canvas);
      }
    }
    if (this._ro) this._ro.disconnect();
    if (this._resizeListener) window.removeEventListener('resize', this._resizeListener);
    if (this._renderer && this._renderer.gl && this._renderer.gl.canvas) {
      const canvas = this._renderer.gl.canvas;
      canvas.removeEventListener('pointermove', this._onPointerMove);
      canvas.removeEventListener('pointerdown', this._onPointerMove);
    }
    const callIfFn = (obj, key) => {
      const fn = obj && obj[key];
      if (typeof fn === 'function') fn.call(obj);
    };
    callIfFn(this._program, 'remove');
    callIfFn(this._geometry, 'remove');
    callIfFn(this._mesh, 'remove');
    callIfFn(this._renderer, 'destroy');
    this._program = null;
    this._geometry = null;
    this._mesh = null;
    this._renderer = null;
  },
  methods: {
    updateColors() {
      const { arr, count, avg } = prepColors(this.colors);
      if (!this._program) return;
      const u = this._program.uniforms;
      for (let i = 0; i < MAX_COLORS; i++) {
        u['uColor' + i].value = arr[i];
      }
      u.uColorCount.value = count;
      u.uMouseColor.value = avg;
    }
  }
};
</script>

<style lang="scss" scoped>
.ferrofluid-container {
  position: relative;
  width: 100%;
  height: 100%;
  overflow: hidden;
}
</style>
