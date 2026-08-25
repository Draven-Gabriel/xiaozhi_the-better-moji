// 全局极光背景：登录页（login / welcome）由页面专属背景接管时自动隐藏
<template>
  <div v-show="isVisible" class="aurora-bg" aria-hidden="true">
    <span class="aurora-bg__orb aurora-bg__orb--1"></span>
    <span class="aurora-bg__orb aurora-bg__orb--2"></span>
    <span class="aurora-bg__orb aurora-bg__orb--3"></span>
    <span class="aurora-bg__beam"></span>
  </div>
</template>

<script>
import { gsap } from '@/utils/gsap';

export default {
  name: 'AuroraBackground',
  computed: {
    isVisible() {
      // login 与 welcome（站点根路径指向登录页）使用专属海洋背景，隐藏全局极光
      return this.$route.name !== 'login' && this.$route.name !== 'welcome';
    }
  },
  watch: {
    isVisible(val) {
      if (val) {
        this.startAnimation();
      } else {
        this.stopAnimation();
      }
    }
  },
  mounted() {
    this.startAnimation();
  },
  beforeDestroy() {
    this.stopAnimation();
  },
  methods: {
    prefersReducedMotion() {
      if (typeof window === 'undefined' || !window.matchMedia) return false;
      return window.matchMedia('(prefers-reduced-motion: reduce)').matches;
    },
    startAnimation() {
      const root = this.$el;
      if (!root) return;
      const reducedMotion = this.prefersReducedMotion();
      const orbs = root.querySelectorAll('.aurora-bg__orb');
      const beam = root.querySelector('.aurora-bg__beam');

      gsap.killTweensOf([orbs, beam]);

      if (reducedMotion) {
        gsap.set(root, { autoAlpha: 0.9 });
        return;
      }

      gsap.fromTo(root, { autoAlpha: 0 }, { autoAlpha: 1, duration: 1.4, ease: 'power2.out', delay: 0.3 });

      const drift = [
        { x: -6, y: -8, scale: 1.12 },
        { x: 8, y: 6, scale: 0.92 },
        { x: 5, y: -9, scale: 1.06 }
      ];
      orbs.forEach((orb, i) => {
        const d = drift[i % drift.length];
        gsap.to(orb, {
          x: d.x,
          y: d.y,
          scale: d.scale,
          duration: 14 + i * 2,
          ease: 'sine.inOut',
          repeat: -1,
          yoyo: true,
          delay: i * 0.8
        });
      });
      if (beam) {
        gsap.fromTo(
          beam,
          { autoAlpha: 0.25, rotate: -4 },
          {
            autoAlpha: 0.8,
            rotate: 4,
            scaleY: 1.08,
            duration: 12,
            ease: 'sine.inOut',
            repeat: -1,
            yoyo: true
          }
        );
      }
    },
    stopAnimation() {
      const root = this.$el;
      if (!root) return;
      gsap.killTweensOf(root);
      gsap.killTweensOf(root.querySelectorAll('.aurora-bg__orb, .aurora-bg__beam'));
    }
  }
};
</script>

<style lang="scss" scoped>
.aurora-bg {
  position: fixed;
  inset: 0;
  z-index: 0;
  overflow: hidden;
  pointer-events: none;
}

.aurora-bg__orb {
  position: absolute;
  border-radius: 50%;
  filter: blur(110px);
  will-change: transform;
  transform-origin: 50% 50%;

  &--1 {
    width: 760px;
    height: 560px;
    top: -220px;
    left: -180px;
    background:
      radial-gradient(ellipse 55% 70% at 30% 40%, rgba(168, 216, 255, 0.55) 0%, rgba(168, 216, 255, 0) 65%),
      radial-gradient(ellipse 80% 60% at 75% 70%, rgba(125, 122, 255, 0.32) 0%, rgba(125, 122, 255, 0) 70%);
  }

  &--2 {
    width: 700px;
    height: 500px;
    top: 24%;
    right: -200px;
    background:
      radial-gradient(ellipse 60% 75% at 30% 50%, rgba(195, 182, 255, 0.45) 0%, rgba(195, 182, 255, 0) 70%),
      radial-gradient(ellipse 70% 50% at 70% 60%, rgba(125, 122, 255, 0.28) 0%, rgba(125, 122, 255, 0) 75%);
  }

  &--3 {
    width: 780px;
    height: 600px;
    bottom: -260px;
    left: 22%;
    background:
      radial-gradient(ellipse 60% 70% at 35% 50%, rgba(181, 240, 236, 0.45) 0%, rgba(181, 240, 236, 0) 70%),
      radial-gradient(ellipse 75% 55% at 70% 70%, rgba(64, 156, 255, 0.22) 0%, rgba(64, 156, 255, 0) 75%);
  }
}

.aurora-bg__beam {
  position: absolute;
  left: 50%;
  top: -30%;
  width: 340px;
  height: 160%;
  transform: translateX(-50%) rotate(-4deg);
  background: linear-gradient(
    180deg,
    rgba(255, 255, 255, 0) 0%,
    rgba(220, 232, 255, 0.5) 40%,
    rgba(203, 224, 255, 0.42) 55%,
    rgba(255, 255, 255, 0) 100%
  );
  filter: blur(60px);
  will-change: transform, opacity;
}

@media (prefers-reduced-motion: reduce) {
  .aurora-bg__beam {
    opacity: 0.5;
  }
}
</style>
