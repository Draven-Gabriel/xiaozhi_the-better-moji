<template>
  <div class="ap-login-page" ref="pageRef">
    <!-- 海洋背景（与登录页一致的 GradientWaves / ogl raymarch 海面） -->
    <gradient-waves
      ref="oceanRef"
      class="ap-ocean"
      horizon-color="#92B6DF"
      wave-color="#184E9E"
      crest-color="#DCEEFB"
      :speed="prefersReducedMotion ? 0 : 0.5"
      :amplitude="3.0"
      :wave-scale="0.72"
      :wave-ratio="0.9"
      :swell="38"
      :turbulence="24"
      :tilt="1.06"
      :zoom="0.98"
      :height="6.0"
      :fog-depth="17"
      detail="medium"
      :brightness="1.0"
      :opacity="1"
      :mouse-interaction="!prefersReducedMotion"
      :parallax-strength="0.4"
      :grain="!prefersReducedMotion"
      :grain-intensity="0.05"
    />
    <div class="ap-ocean-veil" aria-hidden="true"></div>

    <div class="ap-bg-glows" ref="glowsRef" aria-hidden="true">
      <div class="ap-glow ap-glow--blue"></div>
      <div class="ap-glow ap-glow--purple"></div>
      <div class="ap-glow ap-glow--cyan"></div>
    </div>

    <header class="ap-login-brand" ref="brandRef">
      <div class="ap-brand-wrap">
        <span class="ap-brand-text brand-gradient-text">Console</span>
      </div>

      <el-dropdown
        trigger="click"
        class="ap-lang-dd"
        @visible-change="handleLanguageDropdownVisibleChange"
      >
        <span class="ap-lang-trigger">
          <svg viewBox="0 0 16 16" width="14" height="14" class="ap-lang-globe" aria-hidden="true">
            <path
              d="M8 14.5A6.5 6.5 0 1 1 8 1.5a6.5 6.5 0 0 1 0 13zM2 8h12M8 1.5c1.7 1.7 2.6 4 2.6 6.5S9.7 12.8 8 14.5M8 1.5C6.3 3.2 5.4 5.5 5.4 8s.9 4.8 2.6 6.5"
              fill="none"
              stroke="currentColor"
              stroke-width="1.2"
              stroke-linecap="round"
            />
          </svg>
          <span class="ap-lang-text">{{ currentLanguageText }}</span>
          <i class="el-icon-arrow-down ap-lang-arrow" :class="{ 'is-open': languageDropdownVisible }"></i>
        </span>
        <el-dropdown-menu slot="dropdown">
          <el-dropdown-item @click.native="changeLanguage('zh_CN')">
            {{ $t("language.zhCN") }}
          </el-dropdown-item>
          <el-dropdown-item @click.native="changeLanguage('zh_TW')">
            {{ $t("language.zhTW") }}
          </el-dropdown-item>
          <el-dropdown-item @click.native="changeLanguage('en')">
            {{ $t("language.en") }}
          </el-dropdown-item>
          <el-dropdown-item @click.native="changeLanguage('de')">
            {{ $t("language.de") }}
          </el-dropdown-item>
          <el-dropdown-item @click.native="changeLanguage('vi')">
            {{ $t("language.vi") }}
          </el-dropdown-item>
          <el-dropdown-item @click.native="changeLanguage('pt_BR')">
            {{ $t("language.ptBR") }}
          </el-dropdown-item>
        </el-dropdown-menu>
      </el-dropdown>
    </header>

    <main class="ap-login-stage">
      <section class="ap-login-card" ref="cardRef" @keyup.enter="register">
        <div class="ap-card-head" ref="headRef">
          <h1 class="ap-title">{{ $t("register.title") }}</h1>
          <p class="ap-welcome">{{ $t("register.welcome") }}</p>
        </div>

        <div class="ap-card-body">
          <!-- 用户名/手机号输入框 -->
          <div v-if="!enableMobileRegister" class="ap-field" data-anim>
            <ui-input
              v-model="form.username"
              size="large"
              autocomplete="username"
              :placeholder="$t('register.usernamePlaceholder')"
            >
              <template #prefix>
                <img loading="lazy" alt="" src="@/assets/login/username.png" class="ap-field-icon" />
              </template>
            </ui-input>
          </div>

          <!-- 手机号注册部分 -->
          <template v-if="enableMobileRegister">
            <div class="ap-field" data-anim>
              <div class="ap-mobile-row">
                <div class="ap-area">
                  <el-select v-model="form.areaCode" size="medium">
                    <el-option
                      v-for="item in mobileAreaList"
                      :key="item.key"
                      :label="`${item.name} (${item.key})`"
                      :value="item.key"
                    />
                  </el-select>
                </div>
                <div class="ap-mobile-input">
                  <ui-input
                    v-model="form.mobile"
                    size="large"
                    autocomplete="tel"
                    :placeholder="$t('register.mobilePlaceholder')"
                  >
                    <template #prefix>
                      <img loading="lazy" alt="" src="@/assets/login/phone.png" class="ap-field-icon" />
                    </template>
                  </ui-input>
                </div>
              </div>
            </div>

            <div class="ap-field" data-anim>
              <div class="ap-captcha-row">
                <ui-input
                  v-model="form.captcha"
                  size="large"
                  autocomplete="off"
                  :placeholder="$t('register.captchaPlaceholder')"
                >
                  <template #prefix>
                    <img loading="lazy" alt="" src="@/assets/login/shield.png" class="ap-field-icon" />
                  </template>
                </ui-input>
                <img
                  v-if="captchaUrl"
                  :src="captchaUrl"
                  alt="captcha"
                  class="ap-captcha-img"
                  @click="fetchCaptcha"
                />
              </div>
            </div>

            <div class="ap-field" data-anim>
              <div class="ap-captcha-row">
                <ui-input
                  v-model="form.mobileCaptcha"
                  size="large"
                  autocomplete="off"
                  maxlength="6"
                  :placeholder="$t('register.mobileCaptchaPlaceholder')"
                >
                  <template #prefix>
                    <img loading="lazy" alt="" src="@/assets/login/phone.png" class="ap-field-icon" />
                  </template>
                </ui-input>
                <el-button
                  type="primary"
                  class="ap-sms-btn"
                  :disabled="!canSendMobileCaptcha"
                  @click="sendMobileCaptcha"
                >
                  {{ countdown > 0 ? `${countdown}${$t('register.secondsLater')}` : $t('register.sendCaptcha') }}
                </el-button>
              </div>
            </div>
          </template>

          <!-- 密码输入框 -->
          <div class="ap-field" data-anim>
            <ui-input
              v-model="form.password"
              size="large"
              type="password"
              autocomplete="new-password"
              :placeholder="$t('register.passwordPlaceholder')"
            >
              <template #prefix>
                <img loading="lazy" alt="" src="@/assets/login/password.png" class="ap-field-icon" />
              </template>
            </ui-input>
          </div>

          <!-- 确认密码 -->
          <div class="ap-field" data-anim>
            <ui-input
              v-model="form.confirmPassword"
              size="large"
              type="password"
              autocomplete="new-password"
              :placeholder="$t('register.confirmPasswordPlaceholder')"
            >
              <template #prefix>
                <img loading="lazy" alt="" src="@/assets/login/password.png" class="ap-field-icon" />
              </template>
            </ui-input>
          </div>

          <!-- 图形验证码（用户名注册） -->
          <div v-if="!enableMobileRegister" class="ap-field" data-anim>
            <div class="ap-captcha-row">
              <ui-input
                v-model="form.captcha"
                size="large"
                autocomplete="off"
                :placeholder="$t('register.captchaPlaceholder')"
              >
                <template #prefix>
                  <img loading="lazy" alt="" src="@/assets/login/shield.png" class="ap-field-icon" />
                </template>
              </ui-input>
              <img
                v-if="captchaUrl"
                :src="captchaUrl"
                alt="captcha"
                class="ap-captcha-img"
                @click="fetchCaptcha"
              />
            </div>
          </div>
        </div>

        <div class="ap-actions">
          <ui-button
            type="primary"
            size="large"
            round
            :loading="registering"
            class="ap-submit"
            @click="register"
            @mousemove="handleMagnetMove"
            @mouseleave="handleMagnetLeave"
          >
            {{ $t("register.registerButton") }}
          </ui-button>

          <div class="ap-links">
            <span class="ap-link" @click="goToLogin">
              {{ $t("register.goToLogin") }}
            </span>
          </div>
        </div>

        <div class="ap-agreement">
          {{ $t("register.agreeTo") }}
          <span class="ap-agreement-link" @click="openPage('/user-agreement.html')">
            {{ $t("register.userAgreement") }}
          </span>
          {{ $t("login.and") }}
          <span class="ap-agreement-link" @click="openPage('/privacy-policy.html')">
            {{ $t("register.privacyPolicy") }}
          </span>
        </div>
      </section>
    </main>

    <footer class="ap-login-footer" ref="footerRef">
      <version-footer />
    </footer>
  </div>
</template>

<script>
import Api from '@/apis/api';
import VersionFooter from '@/components/VersionFooter.vue';
import i18n, { changeLanguage } from "@/i18n";
import { getUUID, goToPage, showDanger, showSuccess, sm2Encrypt, validateMobile } from '@/utils';
import { mapState } from 'vuex';
import { gsap } from '@/utils/gsap';
import UiInput from '@/components/ui/UiInput.vue';
import UiButton from '@/components/ui/UiButton.vue';
import GradientWaves from '@/components/GradientWaves.vue';

export default {
  name: 'register',
  components: {
    VersionFooter,
    UiInput,
    UiButton,
    GradientWaves,
  },
  computed: {
    ...mapState({
      allowUserRegister: state => state.pubConfig.allowUserRegister,
      enableMobileRegister: state => state.pubConfig.enableMobileRegister,
      mobileAreaList: state => state.pubConfig.mobileAreaList,
      sm2PublicKey: state => state.pubConfig.sm2PublicKey,
    }),
    // 获取当前语言
    currentLanguage() {
      return i18n.locale || "zh_CN";
    },
    currentLanguageText() {
      const currentLang = this.currentLanguage;
      switch (currentLang) {
        case "zh_CN":
          return this.$t("language.zhCN");
        case "zh_TW":
          return this.$t("language.zhTW");
        case "en":
          return this.$t("language.en");
        case "de":
          return this.$t("language.de");
        case "vi":
          return this.$t("language.vi");
        case "pt_BR":
          return this.$t("language.ptBR");
        default:
          return this.$t("language.zhCN");
      }
    },
    canSendMobileCaptcha() {
      return this.countdown === 0 && validateMobile(this.form.mobile, this.form.areaCode);
    }
  },
  data() {
    return {
      form: {
        username: '',
        password: '',
        confirmPassword: '',
        captcha: '',
        captchaId: '',
        areaCode: '+86',
        mobile: '',
        mobileCaptcha: ''
      },
      captchaUrl: '',
      countdown: 0,
      timer: null,
      registering: false,
      languageDropdownVisible: false,
      prefersReducedMotion:
        typeof window !== 'undefined' &&
        typeof window.matchMedia === 'function' &&
        window.matchMedia('(prefers-reduced-motion: reduce)').matches,
    }
  },
  mounted() {
    this.$store.dispatch('fetchPubConfig').then(() => {
      if (!this.allowUserRegister) {
        showDanger(this.$t('register.notAllowRegister'));
        setTimeout(() => {
          goToPage('/login');
        }, 1500);
      }
    });
    this.fetchCaptcha();
    this.playEnterAnimation();
  },
  beforeDestroy() {
    if (this.timer) {
      clearInterval(this.timer);
    }
    this.killEnterAnimation();
  },
  methods: {
    openPage(url) {
      const lang = this.$i18n ? this.$i18n.locale : 'zh_CN';
      if (!lang.startsWith('zh')) {
        url = url.replace('.html', '-en.html');
      }
      window.open(url, '_blank');
    },
    handleLanguageDropdownVisibleChange(visible) {
      this.languageDropdownVisible = visible;
    },
    changeLanguage(lang) {
      changeLanguage(lang);
      this.languageDropdownVisible = false;
      this.$message.success({
        message: this.$t("message.success"),
        showClose: true,
      });
    },
    // 复用验证码获取方法
    fetchCaptcha() {
      this.form.captchaId = getUUID();
      Api.user.getCaptcha(this.form.captchaId, (res) => {
        if (res.status === 200) {
          const blob = new Blob([res.data], { type: res.data.type });
          this.captchaUrl = URL.createObjectURL(blob);

        } else {
          console.error('验证码加载异常:', res);
          showDanger(this.$t('register.captchaLoadFailed'));
        }
      });
    },

    // 封装输入验证逻辑
    validateInput(input, message) {
      if (!input.trim()) {
        showDanger(message);
        return false;
      }
      return true;
    },

    // 发送手机验证码
    sendMobileCaptcha() {
      if (!validateMobile(this.form.mobile, this.form.areaCode)) {
        showDanger(this.$t('register.inputCorrectMobile'));
        return;
      }

      // 验证图形验证码
      if (!this.validateInput(this.form.captcha, this.$t('register.inputCaptcha'))) {
        this.fetchCaptcha();
        return;
      }

      // 清除可能存在的旧定时器
      if (this.timer) {
        clearInterval(this.timer);
        this.timer = null;
      }

      // 开始倒计时
      this.countdown = 60;
      this.timer = setInterval(() => {
        if (this.countdown > 0) {
          this.countdown--;
        } else {
          clearInterval(this.timer);
          this.timer = null;
        }
      }, 1000);

      // 调用发送验证码接口
      Api.user.sendSmsVerification({
        phone: this.form.areaCode + this.form.mobile,
        captcha: this.form.captcha,
        captchaId: this.form.captchaId
      }, (res) => {
        showSuccess(this.$t('register.captchaSendSuccess'));
      }, (err) => {
        showDanger(err.data.msg || this.$t('register.captchaSendFailed'));
        this.countdown = 0;
        this.fetchCaptcha();
      });
    },

    magnetClamp(value, max) {
      return Math.min(Math.max(value, -max), max);
    },

    handleMagnetMove(e) {
      const el = e.currentTarget;
      if (!el) return;
      const rect = el.getBoundingClientRect();
      const dx = e.clientX - (rect.left + rect.width / 2);
      const dy = e.clientY - (rect.top + rect.height / 2);
      gsap.to(el, {
        x: this.magnetClamp(dx / 8, 4),
        y: this.magnetClamp(dy / 8, 2.5),
        duration: 0.35,
        ease: 'power2.out',
      });
    },

    handleMagnetLeave(e) {
      const el = e.currentTarget;
      if (!el) return;
      gsap.to(el, {
        x: 0,
        y: 0,
        duration: 0.4,
        ease: 'power3.out',
      });
    },

    // 注册逻辑
    async register() {
      if (this.registering) return;
      if (this.enableMobileRegister) {
        // 手机号注册验证
        if (!validateMobile(this.form.mobile, this.form.areaCode)) {
          showDanger(this.$t('register.inputCorrectMobile'));
          return;
        }
        if (!this.form.mobileCaptcha) {
          showDanger(this.$t('register.requiredMobileCaptcha'));
          return;
        }
      } else {
        // 用户名注册验证
        if (!this.validateInput(this.form.username, this.$t('register.requiredUsername'))) {
          return;
        }
      }

      // 验证密码
      if (!this.validateInput(this.form.password, this.$t('register.requiredPassword'))) {
        return;
      }
      if (this.form.password !== this.form.confirmPassword) {
        showDanger(this.$t('register.passwordsNotMatch'))
        return
      }
      // 验证验证码
      if (!this.validateInput(this.form.captcha, this.$t('register.requiredCaptcha'))) {
        return;
      }
      // 加密
      let encryptedPassword;
      try {
        // 拼接验证码和密码
        const captchaAndPassword = this.form.captcha + this.form.password;
        encryptedPassword = sm2Encrypt(this.sm2PublicKey, captchaAndPassword);
      } catch (error) {
        console.error("密码加密失败:", error);
        showDanger(this.$t('sm2.encryptionFailed'));
        return;
      }

      let plainUsername;
      if (this.enableMobileRegister) {
        plainUsername = this.form.areaCode + this.form.mobile;
      } else {
        plainUsername = this.form.username;
      }

      // 准备注册数据
      const registerData = {
        username: plainUsername,
        password: encryptedPassword,
        captchaId: this.form.captchaId,
        mobileCaptcha: this.form.mobileCaptcha
      };

      this.registering = true;
      Api.user.register(registerData, ({ data }) => {
        this.registering = false;
        showSuccess(this.$t('register.registerSuccess'))
        goToPage('/login')
      }, (err) => {
        this.registering = false;
        showDanger(err.data.msg || this.$t('register.registerFailed'))
        if (err.data != null && err.data.msg != null && err.data.msg.indexOf('图形验证码') > -1) {
          this.fetchCaptcha()
        }
      })
    },

    goToLogin() {
      goToPage('/login')
    },

    playEnterAnimation() {
      const brand = this.$refs.brandRef;
      const card = this.$refs.cardRef;
      const footer = this.$refs.footerRef;
      const head = this.$refs.headRef;
      const glows = this.$refs.glowsRef
        ? this.$refs.glowsRef.querySelectorAll('.ap-glow')
        : [];
      const fields = card ? card.querySelectorAll('[data-anim]') : [];

      gsap.set(brand, { autoAlpha: 0, y: -12 });
      gsap.set(card, { autoAlpha: 0, y: 40, scale: 0.98 });
      if (head) gsap.set(head, { autoAlpha: 0, y: 12 });
      if (footer) gsap.set(footer, { autoAlpha: 0 });
      fields.forEach(el => gsap.set(el, { autoAlpha: 0, y: 16 }));

      if (brand) {
        gsap.to(brand, {
          autoAlpha: 1,
          y: 0,
          duration: 0.6,
          ease: 'power3.out',
          delay: 0,
        });
      }
      if (card) {
        gsap.to(card, {
          autoAlpha: 1,
          y: 0,
          scale: 1,
          duration: 0.7,
          ease: 'power3.out',
          delay: 0.1,
        });
      }
      if (head) {
        gsap.to(head, {
          autoAlpha: 1,
          y: 0,
          duration: 0.5,
          ease: 'power3.out',
          delay: 0.3,
        });
      }
      if (fields.length) {
        gsap.to(fields, {
          autoAlpha: 1,
          y: 0,
          duration: 0.5,
          ease: 'power3.out',
          stagger: 0.07,
          delay: 0.4,
        });
      }
      if (footer) {
        gsap.to(footer, {
          autoAlpha: 1,
          duration: 0.6,
          ease: 'power3.out',
          delay: 0.9,
        });
      }
      if (glows.length && !this.prefersReducedMotion) {
        glows.forEach((glow, i) => {
          gsap.to(glow, {
            x: `+=${24 + i * 8}`,
            y: `+=${20 + i * 6}`,
            rotation: `+=${(i % 2 === 0 ? 1 : -1) * (16 + i * 4)}`,
            duration: 16 + i * 1.6,
            ease: 'sine.inOut',
            repeat: -1,
            yoyo: true,
            delay: i * 0.8,
          });
        });
      }
    },

    killEnterAnimation() {
      const refs = ['brandRef', 'cardRef', 'headRef', 'footerRef', 'glowsRef', 'pageRef'];
      refs.forEach((key) => {
        const el = this.$refs[key];
        if (!el) return;
        gsap.killTweensOf(el);
        if (el.querySelectorAll) {
          gsap.killTweensOf(el.querySelectorAll('[data-anim]'));
          gsap.killTweensOf(el.querySelectorAll('.ap-glow'));
          const magnet = el.querySelector('.ap-submit');
          if (magnet) gsap.killTweensOf(magnet);
        }
      });
    },
  }
}
</script>

<style lang="scss" scoped>
@import "@/styles/tokens.scss";

.ap-login-page {
  position: relative;
  min-height: 100vh;
  background: linear-gradient(180deg, #f2f9ff 0%, #e6f2fc 45%, #ddebfa 100%);
  font-family: $apple-font-family;
  color: $apple-text;
  overflow-x: hidden;
  display: flex;
  flex-direction: column;
}

.ap-ocean {
  position: fixed;
  inset: 0;
  z-index: 0;
}

.ap-ocean-veil {
  position: fixed;
  inset: 0;
  z-index: 0;
  pointer-events: none;
  // 顶部衔接海平线的淡雾 + 卡片区域柔光，保证卡片可读且不压掉浪花
  background:
    radial-gradient(95% 60% at 50% 42%, rgba(255, 255, 255, 0.6) 0%, rgba(255, 255, 255, 0.18) 55%, rgba(255, 255, 255, 0) 78%),
    linear-gradient(180deg, rgba(255, 255, 255, 0.35) 0%, rgba(255, 255, 255, 0) 30%);
}

.ap-bg-glows {
  position: fixed;
  inset: 0;
  pointer-events: none;
  overflow: hidden;
  z-index: 0;
}

.ap-glow {
  position: absolute;
  border-radius: 50%;
  filter: blur(110px);
  will-change: transform;
  transform-origin: 50% 50%;

  &--blue {
    width: 720px;
    height: 540px;
    top: -200px;
    left: -200px;
    background:
      radial-gradient(ellipse 55% 70% at 30% 40%, rgba(168, 216, 255, 0.55) 0%, rgba(168, 216, 255, 0) 65%),
      radial-gradient(ellipse 80% 60% at 75% 70%, rgba(125, 122, 255, 0.32) 0%, rgba(125, 122, 255, 0) 70%);
  }

  &--purple {
    width: 680px;
    height: 480px;
    top: 28%;
    right: -200px;
    background:
      radial-gradient(ellipse 60% 75% at 30% 50%, rgba(195, 182, 255, 0.45) 0%, rgba(195, 182, 255, 0) 70%),
      radial-gradient(ellipse 70% 50% at 70% 60%, rgba(125, 122, 255, 0.28) 0%, rgba(125, 122, 255, 0) 75%);
  }

  &--cyan {
    width: 760px;
    height: 580px;
    bottom: -240px;
    left: 24%;
    background:
      radial-gradient(ellipse 60% 70% at 35% 50%, rgba(181, 240, 236, 0.45) 0%, rgba(181, 240, 236, 0) 70%),
      radial-gradient(ellipse 75% 55% at 70% 70%, rgba(64, 156, 255, 0.22) 0%, rgba(64, 156, 255, 0) 75%);
  }
}

@media (prefers-reduced-motion: reduce) {
  .ap-glow {
    transform: none !important;
  }
}

.ap-login-brand {
  position: relative;
  z-index: 2;
  display: flex;
  align-items: center;
  gap: 12px;
  padding: 22px 36px;
  width: 100%;
  box-sizing: border-box;
}

.ap-brand-wrap {
  display: inline-flex;
  align-items: center;
  gap: 10px;
}

.ap-brand-text {
  font-family: -apple-system, BlinkMacSystemFont, "SF Pro Display", "PingFang SC", "Microsoft YaHei", sans-serif;
  font-size: 19px;
  font-weight: 600;
  color: transparent;
  -webkit-text-fill-color: transparent;
  letter-spacing: 0.04em;
  line-height: 1;
  display: inline-block;
}

.ap-lang-dd {
  margin-left: auto;
}

.ap-lang-trigger {
  display: inline-flex;
  align-items: center;
  gap: 8px;
  height: 34px;
  padding: 0 14px;
  border-radius: $apple-radius-full;
  background: rgba(255, 255, 255, 0.65);
  -webkit-backdrop-filter: blur(20px) saturate(180%);
  backdrop-filter: blur(20px) saturate(180%);
  border: 1px solid $apple-divider-soft;
  font-size: $apple-font-size-xs;
  color: $apple-text;
  cursor: pointer;
  user-select: none;
  outline: none;
  transition:
    background-color $apple-duration-fast $apple-ease-standard,
    border-color $apple-duration-fast $apple-ease-standard,
    box-shadow $apple-duration-fast $apple-ease-standard;

  &:hover {
    background: rgba(255, 255, 255, 0.92);
    border-color: $apple-divider;
  }

  &:focus-visible {
    box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.24);
  }
}

.ap-lang-globe {
  color: $apple-text-2;
  flex-shrink: 0;
}

.ap-lang-text {
  color: $apple-text-2;
  font-weight: $apple-font-weight-medium;
  letter-spacing: 0.02em;
}

.ap-lang-arrow {
  font-size: 10px;
  color: $apple-text-3;
  transition: transform $apple-duration-fast $apple-ease-standard;

  &.is-open {
    transform: rotate(180deg);
  }
}

.ap-login-stage {
  position: relative;
  z-index: 1;
  flex: 1 1 auto;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 24px 0;
  width: 100%;
}

.ap-login-card {
  position: relative;
  width: 100%;
  max-width: 420px;
  background: $apple-surface;
  border-radius: $apple-radius-2xl;
  box-shadow: $apple-shadow-overlay;
  padding: 40px 36px 28px;
  box-sizing: border-box;
}

.ap-card-head {
  margin-bottom: 24px;
}

.ap-title {
  margin: 0;
  font-size: $apple-font-size-2xl;
  font-weight: $apple-font-weight-bold;
  color: $apple-text;
  letter-spacing: -0.02em;
  line-height: 1.15;
}

.ap-welcome {
  margin: 8px 0 0;
  font-size: $apple-font-size-xs;
  color: $apple-text-2;
  font-weight: $apple-font-weight-semibold;
  letter-spacing: 0.08em;
  text-transform: uppercase;
}

.ap-card-body {
  display: block;
}

.ap-field {
  margin-top: 14px;

  &:first-child {
    margin-top: 0;
  }
}

::v-deep .ui-input__prefix .ap-field-icon {
  width: 18px;
  height: 18px;
  display: block;
  opacity: 0.55;
  filter: grayscale(1) brightness(0.6);
}

.ap-mobile-row {
  display: flex;
  align-items: stretch;
  gap: 10px;
  width: 100%;
}

.ap-area {
  flex: 0 0 auto;
  width: 110px;
  display: flex;
  align-items: stretch;
}

::v-deep .ap-area .el-input {
  width: 100%;
}

::v-deep .ap-area .el-input__inner {
  height: 44px;
  line-height: 44px;
  padding: 0 $apple-space-3;
  font-size: $apple-font-size-base;
  background: $apple-surface-2;
  border: 1px solid transparent;
  border-radius: $apple-radius-sm;
  color: $apple-text;
  transition:
    background-color $apple-duration-fast $apple-ease-standard,
    border-color $apple-duration-fast $apple-ease-standard,
    box-shadow $apple-duration-fast $apple-ease-standard;
}

::v-deep .ap-area .el-input__inner:hover {
  background: $apple-surface;
  border-color: $apple-divider;
}

::v-deep .ap-area .el-input.is-focus .el-input__inner {
  background: $apple-surface;
  border-color: var(--apple-brand);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.12);
}

.ap-mobile-input {
  flex: 1 1 auto;
  min-width: 0;
}

.ap-captcha-row {
  display: flex;
  align-items: stretch;
  gap: 10px;
  width: 100%;
}

.ap-captcha-img {
  flex: 0 0 auto;
  width: 150px;
  height: 40px;
  object-fit: contain;
  border-radius: $apple-radius-sm;
  background: $apple-surface-2;
  border: 1px solid $apple-divider-soft;
  cursor: pointer;
  transition:
    border-color $apple-duration-fast $apple-ease-standard,
    transform $apple-duration-fast $apple-ease-standard;

  &:hover {
    border-color: var(--apple-brand);
    transform: translateY(-1px);
  }
}

.ap-actions {
  margin-top: 24px;
}

::v-deep .ap-submit {
  width: 100%;
  height: 48px;
  font-size: $apple-font-size-base;
  font-weight: $apple-font-weight-semibold;
  letter-spacing: 0.02em;
  display: inline-flex;
}

.ap-links {
  display: flex;
  justify-content: flex-start;
  align-items: center;
  margin-top: 16px;
  font-size: $apple-font-size-sm;
  min-height: 20px;
}

.ap-link {
  color: var(--apple-brand);
  cursor: pointer;
  font-weight: $apple-font-weight-medium;
  user-select: none;
  transition: color $apple-duration-fast $apple-ease-standard;

  &:hover {
    color: var(--apple-brand-hover);
  }

  &:active {
    color: var(--apple-brand-active);
  }
}

.ap-agreement {
  margin-top: 20px;
  font-size: $apple-font-size-xs;
  color: $apple-text-3;
  text-align: center;
  line-height: 1.7;
}

.ap-agreement-link {
  color: var(--apple-brand);
  cursor: pointer;
  font-weight: $apple-font-weight-medium;
  transition: color $apple-duration-fast $apple-ease-standard;

  &:hover {
    color: var(--apple-brand-hover);
    text-decoration: underline;
  }
}

.ap-login-footer {
  position: relative;
  z-index: 2;
  padding: 14px 24px 24px;
  text-align: center;
  font-size: $apple-font-size-xs;
  color: $apple-text-3;
}

::v-deep .ap-login-footer .copyright,
::v-deep .ap-login-footer .footer-content {
  color: $apple-text-3;
}

::v-deep .ap-login-footer .beian-text {
  color: $apple-text-3;
}

::v-deep .el-dropdown-menu {
  border-radius: $apple-radius-md;
  border: 1px solid $apple-divider-soft;
  box-shadow: $apple-shadow-raised;
  padding: 6px;
  background: rgba(255, 255, 255, 0.94);
  -webkit-backdrop-filter: blur(40px);
  backdrop-filter: blur(40px);
}

::v-deep .el-dropdown-menu__item {
  padding: 8px 16px;
  border-radius: $apple-radius-sm;
  font-size: $apple-font-size-sm;
  color: $apple-text;
  line-height: 1.5;
}

::v-deep .el-dropdown-menu__item:hover,
::v-deep .el-dropdown-menu__item:focus {
  background: $apple-brand-tint;
  color: var(--apple-brand);
}

::v-deep .el-tooltip__popper.is-dark {
  background: rgba(29, 29, 31, 0.92);
  color: #ffffff;
  border-radius: $apple-radius-sm;
  font-size: $apple-font-size-xs;
  padding: 4px 10px;
}

// 发送短信验证码按钮（与 ap-captcha-img 同高，视觉接轨登录页按钮风格）
::v-deep .ap-sms-btn {
  flex: 0 0 112px;
  margin-left: 0;
  height: 44px;
  line-height: 44px;
  padding: 0;
  border-radius: var(--apple-radius-md);
  font-size: $apple-font-size-sm;
  font-weight: $apple-font-weight-medium;
  background: var(--apple-brand);
  border-color: var(--apple-brand);
  color: #ffffff;
  transition:
    background-color $apple-duration-fast $apple-ease-standard,
    border-color $apple-duration-fast $apple-ease-standard,
    box-shadow $apple-duration-fast $apple-ease-standard,
    transform $apple-duration-fast $apple-ease-standard;

  &:hover,
  &:focus {
    background: var(--apple-brand-hover);
    border-color: var(--apple-brand-hover);
    color: #ffffff;
    box-shadow: 0 4px 12px rgba(0, 113, 227, 0.28);
    transform: translateY(-1px);
  }

  &:active {
    background: var(--apple-brand-active);
    border-color: var(--apple-brand-active);
    color: #ffffff;
    transform: translateY(0);
  }

  &.is-disabled,
  &.is-disabled:hover,
  &.is-disabled:focus {
    background: $apple-surface-2;
    border-color: $apple-divider-soft;
    color: $apple-text-disabled;
    cursor: not-allowed;
    box-shadow: none;
    transform: none;
  }
}

@media (max-width: 480px) {
  .ap-login-brand {
    padding: 16px 18px;
  }
  .ap-login-card {
    margin: 0 12px;
    padding: 32px 22px 22px;
    border-radius: $apple-radius-xl;
  }
  .ap-area {
    width: 96px;
  }
  .ap-captcha-img {
    width: 108px;
    height: auto;
  }
  .ap-title {
    font-size: $apple-font-size-xl;
  }
  .ap-sms-btn {
    flex-basis: 96px;
    font-size: $apple-font-size-xs;
  }
}
</style>
