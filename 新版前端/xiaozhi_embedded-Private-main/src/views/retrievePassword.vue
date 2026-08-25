<template>
  <div class="welcome" @keyup.enter="retrievePassword">
    <el-container style="height: 100%;">
      <!-- 保持相同的头部 -->
      <el-header>
        <div class="register-brand-wrap">
          <span class="register-brand-text brand-gradient-text">Console</span>
        </div>
      </el-header>
      <div class="login-person">
        <img loading="lazy" alt="" src="@/assets/login/register-person.png" style="width: 100%;" />
      </div>
      <el-main style="position: relative;">
        <form @submit.prevent="retrievePassword">
          <div class="login-box">
            <!-- 修改标题部分 -->
            <div style="display: flex;align-items: center;gap: 20px;margin-bottom: 39px;padding: 0 30px;">
              <img loading="lazy" alt="" src="@/assets/login/hi.png" style="width: 34px;height: 34px;" />
              <div class="login-text">{{ $t('retrievePassword.title') }}</div>
              <div class="login-welcome">
                {{ $t('retrievePassword.subtitle') }}
              </div>
            </div>

            <div style="padding: 0 30px;">
              <!-- 手机号输入 -->
              <div class="input-box">
                <div style="display: flex; align-items: center; width: 100%;">
                  <el-select v-model="form.areaCode" style="width: 220px; margin-right: 10px;">
                    <el-option v-for="item in mobileAreaList" :key="item.key" :label="`${item.name} (${item.key})`"
                      :value="item.key" />
                  </el-select>
                  <el-input v-model="form.mobile" :placeholder="$t('retrievePassword.mobilePlaceholder')" />
                </div>
              </div>

              <div style="display: flex; align-items: center; margin-top: 20px; width: 100%; gap: 10px;">
                <div class="input-box" style="width: calc(100% - 130px); margin-top: 0;">
                  <img loading="lazy" alt="" class="input-icon" src="@/assets/login/shield.png" />
                  <el-input v-model="form.captcha" :placeholder="$t('retrievePassword.captchaPlaceholder')" style="flex: 1;" />
                </div>
                <img loading="lazy" v-if="captchaUrl" :src="captchaUrl" alt="验证码"
                  style="width: 150px; height: 40px; cursor: pointer;" @click="fetchCaptcha" />
              </div>

              <!-- 手机验证码 -->
              <div style="display: flex; align-items: center; margin-top: 20px; width: 100%; gap: 10px;">
                <div class="input-box" style="width: calc(100% - 130px); margin-top: 0;">
                  <img loading="lazy" alt="" class="input-icon" src="@/assets/login/phone.png" />
                  <el-input v-model="form.mobileCaptcha" :placeholder="$t('retrievePassword.mobileCaptchaPlaceholder')" style="flex: 1;" maxlength="6" />
                </div>
                <el-button type="primary" class="send-captcha-btn" :disabled="!canSendMobileCaptcha"
                  @click="sendMobileCaptcha">
                  <span>
                    {{ countdown > 0 ? `${countdown}${$t('register.secondsLater')}` : $t('retrievePassword.getMobileCaptcha') }}
                  </span>
                </el-button>
              </div>

              <!-- 新密码 -->
              <div class="input-box">
                <img loading="lazy" alt="" class="input-icon" src="@/assets/login/password.png" />
                <el-input v-model="form.newPassword" :placeholder="$t('retrievePassword.newPasswordPlaceholder')" type="password" show-password />
              </div>

              <!-- 确认新密码 -->
              <div class="input-box">
                <img loading="lazy" alt="" class="input-icon" src="@/assets/login/password.png" />
                <el-input v-model="form.confirmPassword" :placeholder="$t('retrievePassword.confirmNewPasswordPlaceholder')" type="password" show-password />
              </div>

              <div class="go-login-row">
                <div class="go-login-link" @click="goToLogin">{{ $t('retrievePassword.goToLogin') }}</div>
              </div>
            </div>

            <div class="login-btn" @click="retrievePassword">{{ $t('retrievePassword.resetButton') }}</div>

            <div class="agreement">
              {{ $t('retrievePassword.agreeTo') }}
              <span class="agreement-link" @click="openPage('/user-agreement.html')">{{ $t('register.userAgreement') }}</span>
              {{ $t('login.and') }}
              <span class="agreement-link" @click="openPage('/privacy-policy.html')">{{ $t('register.privacyPolicy') }}</span>
            </div>
          </div>
        </form>
      </el-main>

      <!-- 保持相同的页脚 -->
      <el-footer>
        <version-footer />
      </el-footer>
    </el-container>
  </div>
</template>

<script>
import Api from '@/apis/api';
import VersionFooter from '@/components/VersionFooter.vue';
import { getUUID, goToPage, showDanger, showSuccess, validateMobile, sm2Encrypt } from '@/utils';
import { mapState } from 'vuex';
import i18n from '@/i18n';

// 导入语言切换功能
import { changeLanguage } from '@/i18n';

export default {
  name: 'retrieve',
  components: {
    VersionFooter
  },
  computed: {
    ...mapState({
      allowUserRegister: state => state.pubConfig.allowUserRegister,
      mobileAreaList: state => state.pubConfig.mobileAreaList,
      sm2PublicKey: state => state.pubConfig.sm2PublicKey
    }),
    // 获取当前语言
    currentLanguage() {
      return i18n.locale || "zh_CN";
    },
    canSendMobileCaptcha() {
      return this.countdown === 0 && validateMobile(this.form.mobile, this.form.areaCode);
    }
  },
  data() {
    return {
      form: {
        areaCode: '+86',
        mobile: '',
        captcha: '',
        captchaId: '',
        mobileCaptcha: '',
        newPassword: '',
        confirmPassword: ''
      },
      captchaUrl: '',
      countdown: 0,
      timer: null
    }
  },
  mounted() {
    this.fetchCaptcha();
  },
  methods: {
    openPage(url) {
      const lang = this.$i18n ? this.$i18n.locale : 'zh_CN';
      if (!lang.startsWith('zh')) {
        url = url.replace('.html', '-en.html');
      }
      window.open(url, '_blank');
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
        showDanger(this.$t('retrievePassword.inputCorrectMobile'));
        return;
      }

      // 验证图形验证码
      if (!this.validateInput(this.form.captcha, this.$t('retrievePassword.captchaRequired'))) {
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
        showSuccess(this.$t('retrievePassword.captchaSendSuccess'));
      }, (err) => {
        showDanger(err.data.msg || this.$t('register.captchaSendFailed'));
        this.countdown = 0;
        this.fetchCaptcha();
      });
    },

    // 修改逻辑
    retrievePassword() {
      // 验证逻辑
      if (!validateMobile(this.form.mobile, this.form.areaCode)) {
        showDanger(this.$t('retrievePassword.inputCorrectMobile'));
        return;
      }
      if (!this.form.captcha) {
        showDanger(this.$t('retrievePassword.captchaRequired'));
        return;
      }
      if (!this.form.mobileCaptcha) {
        showDanger(this.$t('retrievePassword.mobileCaptchaRequired'));
        return;
      }
      if (this.form.newPassword !== this.form.confirmPassword) {
        showDanger(this.$t('retrievePassword.passwordsNotMatch'));
        return;
      }

      // 加密密码
      let encryptedPassword;
      try {
        // 拼接图形验证码和新密码进行加密
        const captchaAndPassword = this.form.captcha + this.form.newPassword;
        encryptedPassword = sm2Encrypt(this.sm2PublicKey, captchaAndPassword);
      } catch (error) {
        console.error("密码加密失败:", error);
        showDanger(this.$t('sm2.encryptionFailed'));
        return;
      }

      Api.user.retrievePassword({
        phone: this.form.areaCode + this.form.mobile,
        password: encryptedPassword,
        code: this.form.mobileCaptcha,
        captchaId: this.form.captchaId
      }, (res) => {
        showSuccess(this.$t('retrievePassword.passwordUpdateSuccess'));
        goToPage('/login');
      }, (err) => {
        showDanger(err.data.msg || this.$t('message.error'));
        if (err.data != null && err.data.msg != null && (err.data.msg.indexOf('图形验证码') > -1 || err.data.msg.indexOf('Captcha') > -1)) {
          this.fetchCaptcha()
        }
      });
    },

    goToLogin() {
      goToPage('/login')
    }
  },
  beforeDestroy() {
    if (this.timer) {
      clearInterval(this.timer);
    }
  }
}
</script>

<style lang="scss" scoped>
@import './auth.scss';

.register-brand-wrap {
  display: flex;
  align-items: center;
  margin-top: 15px;
  margin-left: 10px;
  gap: 10px;
}

.register-brand-text {
  font-family: -apple-system, BlinkMacSystemFont, "SF Pro Display", "PingFang SC", "Microsoft YaHei", sans-serif;
  font-size: 19px;
  font-weight: 600;
  color: transparent;
  -webkit-text-fill-color: transparent;
  letter-spacing: 0.04em;
  line-height: 45px;
  white-space: nowrap;
}

.send-captcha-btn {
  margin-right: -5px;
  min-width: 100px;
  height: 40px;
  line-height: 40px;
  border-radius: var(--apple-radius-md);
  font-size: 14px;
  background: var(--apple-brand);
  border: none;
  padding: 0;
  color: #ffffff;
  font-weight: var(--apple-font-weight-medium);
  transition:
    background-color var(--apple-duration-fast) var(--apple-ease-standard),
    box-shadow var(--apple-duration-fast) var(--apple-ease-standard),
    transform var(--apple-duration-fast) var(--apple-ease-standard);

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
    color: #ffffff;
    transform: translateY(0);
  }

  &:disabled,
  &:disabled:hover {
    background: var(--apple-surface-2);
    color: var(--apple-text-disabled);
    border: 1px solid var(--apple-divider-soft);
    cursor: not-allowed;
    box-shadow: none;
    transform: none;
  }
}

::v-deep .send-captcha-btn.el-button {
  background: var(--apple-brand);
  border-color: var(--apple-brand);
  color: #ffffff;
}

::v-deep .send-captcha-btn.el-button:hover,
::v-deep .send-captcha-btn.el-button:focus {
  background: var(--apple-brand-hover);
  border-color: var(--apple-brand-hover);
  color: #ffffff;
}

.go-login-row {
  font-weight: var(--apple-font-weight-regular);
  font-size: 14px;
  text-align: left;
  color: var(--apple-brand);
  margin-top: 20px;
}

.go-login-link {
  cursor: pointer;
  transition: color var(--apple-duration-fast) var(--apple-ease-standard);

  &:hover {
    color: var(--apple-brand-hover);
  }

  &:active {
    color: var(--apple-brand-active);
  }
}

.agreement {
  font-size: 14px;
  color: var(--apple-text-3);
  text-align: center;
  line-height: 1.7;
}

.agreement-link {
  display: inline-block;
  color: var(--apple-brand);
  cursor: pointer;
  font-weight: var(--apple-font-weight-medium);
  transition: color var(--apple-duration-fast) var(--apple-ease-standard);

  &:hover {
    color: var(--apple-brand-hover);
    text-decoration: underline;
  }

  &:active {
    color: var(--apple-brand-active);
  }
}
</style>
