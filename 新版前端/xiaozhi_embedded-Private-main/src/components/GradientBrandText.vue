// 品牌文字 Logo：多层景深 + 鼠标视差 + 双语定时自动切换（console / 控制台）
// 点击不拦截事件，冒泡至父级（header-left）保持"点击 Logo 回主页"的原有行为
<template>
  <span
    ref="root"
    class="brand-text"
    :class="{ 'is-hover': hovering }"
    :style="wrapperStyle"
    :title="title"
    @mouseenter="onEnter"
    @mousemove="onMove"
    @mouseleave="onLeave"
  >
    <span
      v-for="(layer, i) in layers"
      :key="'layer-' + i"
      ref="layers"
      class="brand-text-layer"
      :class="{ 'brand-text-layer--main': layer.isMain }"
      :style="layerStyle(layer)"
    ></span>
  </span>
</template>

<script>
import { gsap } from '@/utils/gsap';

const STORAGE_KEY = 'xiaozhi-brand-language';
// 透明留白：容纳视差位移（±9px）与模糊光晕（约4px），防止雾层压到左右相邻元素
const PAD_X = 14;
const PAD_Y = 10;
const FONT_STACK =
  '-apple-system, BlinkMacSystemFont, "SF Pro Display", "SF Pro Text", "Helvetica Neue", "PingFang SC", "Microsoft YaHei", "Segoe UI", sans-serif';

export default {
  name: 'GradientBrandText',
  props: {
    text: { type: String, default: 'Console' },
    secondaryText: { type: String, default: '控制台' },
    size: { type: [String, Number], default: 18 },
    weight: { type: [String, Number], default: 600 },
    parallax: { type: Number, default: 10 },
    interval: { type: Number, default: 5000 }
  },
  data() {
    return {
      ready: false,
      activeIndex: 0,
      hovering: false
    };
  },
  computed: {
    layers() {
      return [
        { isMain: false, mult: -1.8, blur: 2.2 },
        { isMain: false, mult: 1.0, blur: 1.1 },
        { isMain: true, mult: 0.35, blur: 0 }
      ];
    },
    currentText() {
      return this.activeIndex === 0 ? this.text : this.secondaryText;
    },
    title() {
      const other = this.activeIndex === 0 ? this.secondaryText : this.text;
      return this.currentText + ' · ' + other + ' (click)';
    },
    fontPx() {
      return typeof this.size === 'number' ? this.size + 'px' : this.size;
    },
    wrapperStyle() {
      return {
        fontSize: this.fontPx,
        fontWeight: this.weight
      };
    }
  },
  mounted() {
    this.loadPersisted();
    this.fixRootSize();
    this.build();
    this.bindParallax();
    if (this.prefersReducedMotion()) {
      this.ready = true;
    } else {
      this.$nextTick(() => {
        this.animateEntrance();
        this.ready = true;
      });
    }
    this.startAutoSwitch();
  },
  beforeDestroy() {
    this.stopAutoSwitch();
    if (this.layerEls) gsap.killTweensOf(this.layerEls);
    this._triggers = null;
  },
  methods: {
    prefersReducedMotion() {
      return (
        typeof window !== 'undefined' &&
        window.matchMedia &&
        window.matchMedia('(prefers-reduced-motion: reduce)').matches
      );
    },
    loadPersisted() {
      try {
        const saved = localStorage.getItem(STORAGE_KEY);
        this.activeIndex = Number(saved) === 1 ? 1 : 0;
      } catch (e) {
        this.activeIndex = 0;
      }
    },
    persist() {
      try {
        localStorage.setItem(STORAGE_KEY, String(this.activeIndex));
      } catch (e) {}
    },
    // 同步测量两种文字宽度，将根容器固定为较宽者，避免切换时宽幅变化推移旁边导航
    measureTextWidth(text) {
      if (typeof document === 'undefined') return 0;
      const ctx = document.createElement('canvas').getContext('2d');
      ctx.font = `${this.weight} ${this.fontPx} ${FONT_STACK}`;
      const m = ctx.measureText(text);
      return Math.ceil(m.width + (m.actualBoundingBoxLeft || 0)) + 2;
    },
    fixRootSize() {
      const root = this.$el;
      if (!root) return;
      const w = Math.max(this.measureTextWidth(this.text), this.measureTextWidth(this.secondaryText));
      const px = parseInt(this.fontPx, 10) || 18;
      root.style.width = w + PAD_X * 2 + 'px';
      root.style.height = Math.round(px * 1.2) + PAD_Y * 2 + 'px';
      this._fixedWidth = w;
    },
    build() {
      const root = this.$el;
      if (!root) return;
      this.layerEls = this.$refs.layers || [];
      this.layerEls.forEach((layerEl) => {
        layerEl.textContent = '';
        const content = document.createElement('span');
        content.className = 'brand-text-content';
        content.textContent = this.currentText;
        layerEl.appendChild(content);
      });
      if (!this._fixedWidth && this.layerEls.length) {
        const main = this.layerEls[this.layerEls.length - 1];
        if (main) {
          root.style.width = main.offsetWidth + 'px';
          root.style.height = main.offsetHeight + 'px';
        }
      }
    },
    layerStyle(layer) {
      return layer.isMain ? {} : { filter: `blur(${layer.blur}px)` };
    },
    bindParallax() {
      if (this.prefersReducedMotion() || !this.layerEls || !this.layerEls.length) return;
      this._triggers = this.layerEls.map((el) => ({
        x: gsap.quickTo(el, 'x', { duration: 0.7, ease: 'power2.out' }),
        y: gsap.quickTo(el, 'y', { duration: 0.7, ease: 'power2.out' })
      }));
    },
    onEnter() {
      this._hovering = true;
      this.hovering = true;
    },
    onMove(e) {
      if (!this._triggers || !this._hovering) return;
      const r = this.$el.getBoundingClientRect();
      const dx = (e.clientX - r.left) / r.width - 0.5;
      const dy = (e.clientY - r.top) / r.height - 0.5;
      this.layers.forEach((layer, i) => {
        const t = this._triggers[i];
        t.x(dx * layer.mult * this.parallax);
        t.y(dy * layer.mult * this.parallax * 0.45);
      });
    },
    onLeave() {
      if (!this._triggers) return;
      this._triggers.forEach((t) => {
        t.x(0);
        t.y(0);
      });
      this._hovering = false;
      this.hovering = false;
    },
    animateEntrance() {
      const root = this.$el;
      if (!root) return;
      gsap.fromTo(
        root,
        { autoAlpha: 0, y: 6 },
        {
          autoAlpha: 1,
          y: 0,
          duration: 0.6,
          ease: 'power2.out',
          onComplete: () => gsap.set(root, { clearProps: 'transform' })
        }
      );
    },
    startAutoSwitch() {
      this.stopAutoSwitch();
      if (this.prefersReducedMotion()) {
        // 尊重系统减弱动态偏好：不做定时自动切换
        this.reducedMotionPreference = true;
        return;
      }
      this.reducedMotionPreference = false;
      this._timer = setInterval(() => {
        // 后台标签页不切换，避免无意义的动画开销
        if (document.hidden || !this.ready) return;
        this.swapText();
      }, this.interval);
    },
    stopAutoSwitch() {
      if (this._timer) {
        clearInterval(this._timer);
        this._timer = null;
      }
    },
    swapText() {
      if (this._switching) return;
      this._switching = true;
      const layers = this.layerEls;
      const finishSwap = () => {
        this._switching = false;
        this.activeIndex = this.activeIndex === 0 ? 1 : 0;
        this.persist();
        this.build();
        this.reducedMotionPreference = this.prefersReducedMotion();
        // 尊重系统减弱动态偏好：直接呈现，不做过渡动画
        if (this.reducedMotionPreference) {
          gsap.set(layers, { autoAlpha: 1, y: 0, clearProps: 'transform' });
          this.reducedMotionPreference = true;
          return;
        }
        this.$nextTick(() => {
          gsap.fromTo(
            layers,
            { autoAlpha: 0, y: 6 },
            {
              autoAlpha: 1,
              y: 0,
              duration: 0.45,
              ease: 'power2.out',
              stagger: 0.06,
              onComplete: () => {
                // 还原背景层给 CSS 托管（静止时隐藏、hover 时淡入），避免残留内联透明度
                const bg = this.layerEls.filter((_, li) => !this.layers[li].isMain);
                if (bg.length) gsap.set(bg, { clearProps: 'opacity,visibility' });
              }
            }
          );
        });
      };
      if (!layers || !layers.length) {
        finishSwap();
        return;
      }
      gsap.to(layers, {
        autoAlpha: 0,
        y: -4,
        duration: 0.16,
        ease: 'power2.in',
        onComplete: finishSwap
      });
    }
  }
};
</script>

<style lang="scss" scoped>
.brand-text {
  position: relative;
  display: inline-block;
  white-space: nowrap;
  line-height: 1.2;
  font-family: -apple-system, BlinkMacSystemFont, "SF Pro Display", "SF Pro Text", "Helvetica Neue", "PingFang SC", "Microsoft YaHei", "Segoe UI", sans-serif;
  color: var(--apple-text, #1d1d1f);
  cursor: pointer;
  user-select: none;
  -webkit-user-select: none;
  will-change: transform;
  overflow: hidden;
}

.brand-text-layer {
  position: absolute;
  left: 0;
  top: 0;
  right: 0;
  bottom: 0;
  display: flex;
  align-items: center;
  justify-content: center;
}

.brand-text-layer--main {
  // 与登录/注册页品牌文字同款渐变（global.scss .brand-gradient-text）
  background-image: linear-gradient(
    120deg,
    var(--apple-brand) 0%,
    #7d7aff 50%,
    var(--apple-brand) 100%
  );
  background-size: 250% 100%;
  background-position: 0% 50%;
  -webkit-background-clip: text;
  background-clip: text;
  -webkit-text-fill-color: transparent;
  color: transparent;
  animation: brand-gradient-flow 5s ease-in-out infinite;
}

.brand-text-layer:nth-child(1) {
  // 背景景深层：静止时隐藏，鼠标接近才淡入，保证平时文字清晰
  color: var(--apple-brand);
  opacity: 0;
  transition: opacity 0.45s cubic-bezier(0.4, 0, 0.2, 1);
}

.brand-text-layer:nth-child(2) {
  color: #7d7aff;
  opacity: 0;
  transition: opacity 0.45s cubic-bezier(0.4, 0, 0.2, 1);
}

.brand-text.is-hover .brand-text-layer:nth-child(1) {
  opacity: 0.3;
}

.brand-text.is-hover .brand-text-layer:nth-child(2) {
  opacity: 0.55;
}

.brand-text-content {
  white-space: nowrap;
}

@media (prefers-reduced-motion: reduce) {
  .brand-text-layer--main {
    animation: none;
  }
}
</style>
