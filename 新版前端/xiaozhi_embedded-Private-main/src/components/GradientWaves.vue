// GradientWaves —— 移植自 React Bits（reactbits.dev）"Gradient Waves"（JavaScript + CSS 变体）
// ogl(WebGL2) raymarch 海面：波浪向海平线滚动、雾化天际、浪尖高光、鼠标视差
<template>
  <div ref="container" class="gradient-waves-container" :class="className"></div>
</template>

<script>
import { Renderer, Program, Mesh, Triangle } from 'ogl';

const hexToRgb = (hex) => {
  const result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(String(hex));
  if (!result) return [1, 1, 1];
  return [parseInt(result[1], 16) / 255, parseInt(result[2], 16) / 255, parseInt(result[3], 16) / 255];
};

const detailToSteps = (detail) => {
  if (detail === 'low') return 40.0;
  if (detail === 'high') return 110.0;
  return 70.0;
};

const vertex = `#version 300 es
in vec2 position;
void main() {
  gl_Position = vec4(position, 0.0, 1.0);
}
`;

const fragment = `#version 300 es
precision highp float;
uniform vec2 iResolution;
uniform float iTime;
uniform float uSpeed;
uniform float uAmplitude;
uniform float uWaveScale;
uniform float uWaveRatio;
uniform float uSwell;
uniform float uTurbulence;
uniform float uTilt;
uniform float uZoom;
uniform float uHeight;
uniform float uFogDepth;
uniform float uSteps;
uniform float uBrightness;
uniform float uOpacity;
uniform float uGrain;
uniform float uGrainIntensity;
uniform vec2 uMouse;
uniform float uParallax;
uniform bool uEnableMouse;
uniform vec3 uHorizonColor;
uniform vec3 uWaveColor;
uniform vec3 uCrestColor;
out vec4 fragColor;

const float MAX_DIST = 20000.0;

float hash21(vec2 p) {
  vec3 p3 = fract(vec3(p.xyx) * 0.1031);
  p3 += dot(p3, p3.yzx + 33.33);
  return fract((p3.x + p3.y) * p3.z);
}

float plasma(vec3 r, vec2 freq, vec4 tc) {
  float mx = r.x + tc.x;
  mx += uSwell * sin((r.y + mx) / 20.0 + tc.y);
  float my = r.y - tc.z;
  my += uTurbulence * cos(r.x / 23.0 + tc.w);
  return r.z - (sin(mx * freq.x) * uAmplitude + sin(my * freq.y) * uAmplitude + uHeight);
}

float raymarch(vec3 pos, vec3 dir, vec2 freq, vec4 tc) {
  float dist = 0.0;
  for (int i = 0; i < 128; i++) {
    if (float(i) >= uSteps) break;
    float dscene = plasma(pos + dist * dir, freq, tc);
    if (abs(dscene) < 0.1) break;
    dist += 0.9 * dscene;
    if (!(abs(dist) < MAX_DIST)) return MAX_DIST;
  }
  return dist;
}

void main() {
  float T = iTime * uSpeed;
  vec2 freq = vec2(uWaveScale / 7.0, (uWaveScale * uWaveRatio) / 3.0);
  vec4 tc = vec4(T / 0.130, T / 0.810, T / 0.200, T / 0.710);
  float c, s;
  float vfov = (3.14159 / 2.3) / max(uZoom, 0.05);
  vec3 cam = vec3(0.0, 0.0, 30.0);
  vec2 uv = (gl_FragCoord.xy / iResolution.xy) - 0.5;
  uv.x *= iResolution.x / iResolution.y;
  uv.y *= -1.0;

  vec3 dir = vec3(0.0, 0.0, -1.0);
  float ulen = length(uv);
  float xrot = vfov * ulen;
  c = cos(xrot); s = sin(xrot);
  dir = mat3(1.0, 0.0, 0.0, 0.0, c, -s, 0.0, s, c) * dir;
  vec2 nuv = ulen > 1e-5 ? uv / ulen : vec2(1.0, 0.0);
  c = nuv.x; s = nuv.y;
  dir = mat3(c, -s, 0.0, s, c, 0.0, 0.0, 0.0, 1.0) * dir;
  c = cos(uTilt); s = sin(uTilt);
  dir = mat3(c, 0.0, s, 0.0, 1.0, 0.0, -s, 0.0, c) * dir;

  if (uEnableMouse) {
    float yaw = (uMouse.x - 0.5) * uParallax * 0.4;
    float pitch = (uMouse.y - 0.5) * uParallax * 0.4;
    c = cos(yaw); s = sin(yaw);
    dir = mat3(c, 0.0, s, 0.0, 1.0, 0.0, -s, 0.0, c) * dir;
    c = cos(pitch); s = sin(pitch);
    dir = mat3(1.0, 0.0, 0.0, 0.0, c, -s, 0.0, s, c) * dir;
  }

  float dist = raymarch(cam, dir, freq, tc);
  vec3 pos = cam + dist * dir;

  float t = clamp(uFogDepth / max(dist, 0.001), 0.0, 1.0);
  vec3 body = mix(uWaveColor, uCrestColor, clamp(pos.z * 0.08 + 0.5, 0.0, 1.0));
  vec3 col = mix(uHorizonColor, body, t);
  col *= uBrightness;
  col = clamp(col, 0.0, 1.0);

  float alpha = clamp(t, 0.0, 1.0) * uOpacity;
  if (uGrain > 0.5) {
    float g = hash21(gl_FragCoord.xy + mod(iTime, 64.0) * 11.0);
    alpha += (g - 0.5) * uGrainIntensity;
  }
  alpha = clamp(alpha, 0.0, 1.0);
  fragColor = vec4(col * alpha, alpha);
}
`;

const COLOR_FIELDS = [
  ['uHorizonColor', 'horizonColor'],
  ['uWaveColor', 'waveColor'],
  ['uCrestColor', 'crestColor']
];

export default {
  name: 'GradientWaves',
  props: {
    horizonColor: { type: String, default: '#5227FF' },
    waveColor: { type: String, default: '#FF9FFC' },
    crestColor: { type: String, default: '#FFFFFF' },
    speed: { type: Number, default: 0.4 },
    amplitude: { type: Number, default: 2.5 },
    waveScale: { type: Number, default: 0.6 },
    waveRatio: { type: Number, default: 0.9 },
    swell: { type: Number, default: 35 },
    turbulence: { type: Number, default: 20 },
    tilt: { type: Number, default: 1.11 },
    zoom: { type: Number, default: 1.0 },
    height: { type: Number, default: 5.5 },
    fogDepth: { type: Number, default: 15 },
    detail: { type: String, default: 'medium' },
    brightness: { type: Number, default: 1.0 },
    opacity: { type: Number, default: 1.0 },
    mouseInteraction: { type: Boolean, default: true },
    parallaxStrength: { type: Number, default: 0.5 },
    grain: { type: Boolean, default: true },
    grainIntensity: { type: Number, default: 0.05 },
    className: { type: String, default: '' }
  },
  watch: {
    mouseInteraction(v) {
      this._enableMouse = v;
      if (this._program) {
        this._program.uniforms.uEnableMouse.value = v;
      }
    },
    horizonColor() { this.applyColors(); },
    waveColor() { this.applyColors(); },
    crestColor() { this.applyColors(); },
    speed(v) { this.setUniform('uSpeed', v); },
    amplitude(v) { this.setUniform('uAmplitude', v); },
    waveScale(v) { this.setUniform('uWaveScale', v); },
    waveRatio(v) { this.setUniform('uWaveRatio', v); },
    swell(v) { this.setUniform('uSwell', v); },
    turbulence(v) { this.setUniform('uTurbulence', v); },
    tilt(v) { this.setUniform('uTilt', v); },
    zoom(v) { this.setUniform('uZoom', v); },
    height(v) { this.setUniform('uHeight', v); },
    fogDepth(v) { this.setUniform('uFogDepth', v); },
    detail(v) { this.setUniform('uSteps', detailToSteps(v)); },
    brightness(v) { this.setUniform('uBrightness', v); },
    opacity(v) { this.setUniform('uOpacity', v); },
    grain(v) { this.setUniform('uGrain', v ? 1.0 : 0.0); },
    grainIntensity(v) { this.setUniform('uGrainIntensity', v); },
    parallaxStrength(v) { this.setUniform('uParallax', v); }
  },
  mounted() {
    const container = this.$refs.container;
    if (!container) return;

    const renderer = new Renderer({
      webgl: 2,
      alpha: true,
      premultipliedAlpha: true,
      antialias: false,
      dpr: Math.min(window.devicePixelRatio || 1, 2)
    });

    const gl = renderer.gl;
    gl.clearColor(0, 0, 0, 0);
    const canvas = gl.canvas;
    canvas.style.width = '100%';
    canvas.style.height = '100%';
    canvas.style.display = 'block';
    canvas.style.touchAction = 'none';
    container.appendChild(canvas);

    const geometry = new Triangle(gl);
    const program = new Program(gl, {
      vertex,
      fragment,
      uniforms: {
        iTime: { value: 0 },
        iResolution: { value: new Float32Array([1, 1]) },
        uSpeed: { value: this.speed },
        uAmplitude: { value: this.amplitude },
        uWaveScale: { value: this.waveScale },
        uWaveRatio: { value: this.waveRatio },
        uSwell: { value: this.swell },
        uTurbulence: { value: this.turbulence },
        uTilt: { value: this.tilt },
        uZoom: { value: this.zoom },
        uHeight: { value: this.height },
        uFogDepth: { value: this.fogDepth },
        uSteps: { value: detailToSteps(this.detail) },
        uBrightness: { value: this.brightness },
        uOpacity: { value: this.opacity },
        uGrain: { value: this.grain ? 1.0 : 0.0 },
        uGrainIntensity: { value: this.grainIntensity },
        uMouse: { value: new Float32Array([0.5, 0.5]) },
        uParallax: { value: this.parallaxStrength },
        uEnableMouse: { value: this.mouseInteraction },
        uHorizonColor: { value: new Float32Array([1, 1, 1]) },
        uWaveColor: { value: new Float32Array([1, 1, 1]) },
        uCrestColor: { value: new Float32Array([1, 1, 1]) }
      }
    });
    const mesh = new Mesh(gl, { geometry, program });

    this._renderer = renderer;
    this._program = program;
    this._mesh = mesh;
    this._canvas = canvas;
    this._enableMouse = this.mouseInteraction;

    this.applyColors();

    const setSize = () => {
      const rect = container.getBoundingClientRect();
      const w = Math.max(1, Math.floor(rect.width));
      const h = Math.max(1, Math.floor(rect.height));
      renderer.setSize(w, h);
      const res = program.uniforms.iResolution.value;
      res[0] = gl.drawingBufferWidth;
      res[1] = gl.drawingBufferHeight;
      renderer.render({ scene: mesh });
    };
    this._setSize = setSize;

    let ro = null;
    if (typeof ResizeObserver !== 'undefined') {
      ro = new ResizeObserver(setSize);
      ro.observe(container);
      this._ro = ro;
    } else {
      window.addEventListener('resize', setSize);
      this._resizeListener = setSize;
    }
    setSize();

    this._currentMouse = [0.5, 0.5];
    this._targetMouse = [0.5, 0.5];

    this._onPointerMove = (e) => {
      const rect = canvas.getBoundingClientRect();
      this._targetMouse[0] = (e.clientX - rect.left) / rect.width;
      this._targetMouse[1] = 1.0 - (e.clientY - rect.top) / rect.height;
    };
    this._onPointerLeave = () => {
      this._targetMouse[0] = 0.5;
      this._targetMouse[1] = 0.5;
    };
    canvas.addEventListener('pointermove', this._onPointerMove);
    canvas.addEventListener('pointerleave', this._onPointerLeave);

    this._raf = 0;
    this._isVisible = true;
    this._isPageVisible = !document.hidden;
    const t0 = performance.now();

    this._loop = (t) => {
      this._raf = requestAnimationFrame(this._loop);
      program.uniforms.iTime.value = (t - t0) * 0.001;
      const tx = this._enableMouse ? this._targetMouse[0] : 0.5;
      const ty = this._enableMouse ? this._targetMouse[1] : 0.5;
      this._currentMouse[0] += 0.05 * (tx - this._currentMouse[0]);
      this._currentMouse[1] += 0.05 * (ty - this._currentMouse[1]);
      program.uniforms.uMouse.value[0] = this._currentMouse[0];
      program.uniforms.uMouse.value[1] = this._currentMouse[1];
      renderer.render({ scene: mesh });
    };

    this._tryStart = () => {
      if (this._isVisible && this._isPageVisible && this._raf === 0) {
        this._raf = requestAnimationFrame(this._loop);
      }
    };
    this._tryStop = () => {
      if (this._raf !== 0) {
        cancelAnimationFrame(this._raf);
        this._raf = 0;
      }
    };

    if (typeof IntersectionObserver !== 'undefined') {
      this._io = new IntersectionObserver(
        ([entry]) => {
          this._isVisible = entry.isIntersecting;
          this._isVisible ? this._tryStart() : this._tryStop();
        },
        { threshold: 0 }
      );
      this._io.observe(container);
    }

    this._onVisibility = () => {
      this._isPageVisible = !document.hidden;
      this._isPageVisible ? this._tryStart() : this._tryStop();
    };
    document.addEventListener('visibilitychange', this._onVisibility);

    this._tryStart();
  },
  beforeDestroy() {
    if (this._raf) {
      cancelAnimationFrame(this._raf);
      this._raf = 0;
    }
    if (this._ro) this._ro.disconnect();
    if (this._io) this._io.disconnect();
    document.removeEventListener('visibilitychange', this._onVisibility);
    if (this._canvas) {
      this._canvas.removeEventListener('pointermove', this._onPointerMove);
      this._canvas.removeEventListener('pointerleave', this._onPointerLeave);
      const container = this.$refs.container;
      try {
        if (container && this._canvas.parentElement === container) {
          container.removeChild(this._canvas);
        }
      } catch (e) {}
      const lose = this._renderer && this._renderer.gl && this._renderer.gl.getExtension('WEBGL_lose_context');
      if (lose) lose.loseContext();
    }
    if (this._resizeListener) window.removeEventListener('resize', this._resizeListener);
    this._renderer = null;
    this._program = null;
    this._mesh = null;
    this._canvas = null;
  },
  methods: {
    setUniform(name, value) {
      if (this._program) this._program.uniforms[name].value = value;
    },
    applyColors() {
      if (!this._program) return;
      const u = this._program.uniforms;
      COLOR_FIELDS.forEach(([uniformName, propName]) => {
        const rgb = hexToRgb(this[propName]);
        const v = u[uniformName].value;
        v[0] = rgb[0];
        v[1] = rgb[1];
        v[2] = rgb[2];
      });
    }
  }
};
</script>

<style lang="scss" scoped>
.gradient-waves-container {
  position: relative;
  width: 100%;
  height: 100%;
  overflow: hidden;
}
</style>
