<template>
  <el-drawer
    :visible.sync="drawerVisible"
    :before-close="handleClose"
    direction="rtl"
    size="400px"
    :modal="true"
    :show-close="false"
    custom-class="tts-advanced-drawer"
  >
    <div class="drawer-header" slot="title">
      <span class="drawer-title">{{ $t('roleConfig.advancedSettings') }}</span>
      <button class="drawer-close-btn" @click="handleClose">×</button>
    </div>

    <div class="drawer-content">
      <el-form label-position="top">
        <!-- 音量 -->
        <el-form-item :label="$t('roleConfig.ttsVolume')">
          <div class="slider-container">
            <el-slider
              v-model="localSettings.volume"
              :min="-100"
              :max="100"
              :step="1"
              :format-tooltip="formatTooltip"
              class="tts-slider"
              @change="markTtsSettingChanged('volume')"
            />
            <span class="slider-hint">{{ $t('roleConfig.volumeHint') }}</span>
          </div>
        </el-form-item>

        <!-- 语速 -->
        <el-form-item :label="$t('roleConfig.ttsRate')">
          <div class="slider-container">
            <el-slider
              v-model="localSettings.speed"
              :min="-100"
              :max="100"
              :step="1"
              :format-tooltip="formatTooltip"
              class="tts-slider"
              @change="markTtsSettingChanged('speed')"
            />
            <span class="slider-hint">{{ $t('roleConfig.speedHint') }}</span>
          </div>
        </el-form-item>

        <!-- 音调 -->
        <el-form-item :label="$t('roleConfig.ttsPitch')">
          <div class="slider-container">
            <el-slider
              v-model="localSettings.pitch"
              :min="-100"
              :max="100"
              :step="1"
              :format-tooltip="formatTooltip"
              class="tts-slider"
              @change="markTtsSettingChanged('pitch')"
            />
            <span class="slider-hint">{{ $t('roleConfig.pitchHint') }}</span>
        </div>
        </el-form-item>

      </el-form>
      <!-- 关联替换词 -->
      <div>
        <h4 class="replacement-label">
          {{ $t('roleConfig.replacementWordLabel') }}
          <el-tooltip popper-class="tts-tooltip" :content="$t('roleConfig.replacementWordTip')" effect="light" placement="top">
            <i class="el-icon-question"></i>
          </el-tooltip>
        </h4>
        <el-select
          v-model="replacementWordIds"
          multiple
          :placeholder="$t('replacementDialog.selectPlaceholder')"
          class="replacement-word-select"
        >
          <el-option
            v-for="item in replacementWordList"
            :key="item.id"
            :label="item.fileName"
            :value="item.id"
          />
        </el-select>
      </div>
    </div>

    <div class="drawer-footer">
      <el-button @click="handleCancel">{{ $t('button.cancel') }}</el-button>
      <el-button type="primary" @click="handleSave">{{ $t('button.save') }}</el-button>
    </div>
  </el-drawer>
</template>

<script>
import correctWord from '@/apis/module/correctWord';

export default {
  name: 'TtsAdvancedSettings',
  props: {
    visible: {
      type: Boolean,
      default: false
    },
    settings: {
      type: Object,
      default: () => ({
        volume: 0,
        speed: 0,
        pitch: 0
      })
    },
    checkedReplacementWordIds: {
      type: Array,
      default: () => []
    },
  },
  data() {
    return {
      localSettings: {
        volume: 0,
        speed: 0,
        pitch: 0,
      },
      changedTtsFields: {
        volume: false,
        speed: false,
        pitch: false,
      },
      replacementWordIds: [],
      replacementWordList: []
    };
  },
  computed: {
    drawerVisible: {
      get() {
        return this.visible;
      },
      set(val) {
        this.$emit('update:visible', val);
      }
    }
  },
  watch: {
    visible(newVal) {
      if (newVal) {
        // 当抽屉打开时，复制当前设置到本地
        this.localSettings = { ...this.settings };
        this.changedTtsFields = {
          volume: false,
          speed: false,
          pitch: false,
        };
        this.replacementWordIds = [...this.checkedReplacementWordIds];
        this.fetchReplacementWordList();
      }
    }
  },
  methods: {
    handleClose() {
      this.$emit('update:visible', false);
    },
    handleCancel() {
      // 取消时不保存，直接关闭
      this.handleClose();
    },
    handleSave() {
      // 保存设置并关闭
      const changedTtsFields = Object.keys(this.changedTtsFields)
        .filter((field) => this.changedTtsFields[field]);
      this.$emit('save', {
        ...this.localSettings,
        changedTtsFields,
        replacementWordIds: this.replacementWordIds
      });
      this.handleClose();
    },
    markTtsSettingChanged(field) {
      this.$set(this.changedTtsFields, field, true);
    },
    formatTooltip(val) {
      return `${val}%`;
    },
    fetchReplacementWordList() {
      correctWord.selectAll(({ data }) => {
        if (data.code === 0) {
          this.replacementWordList = data.data;
        }
      });
    },
    getTagName(id) {
      const item = this.replacementWordList.find(item => item.id === id);
      return item ? item.fileName : '';
    }
  }
};
</script>

<style scoped>
.drawer-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 20px 24px;
  border-bottom: 1px solid var(--apple-divider-soft);
  background-color: var(--apple-surface);
}

.drawer-title {
  font-size: var(--apple-font-size-md);
  font-weight: var(--apple-font-weight-semibold);
  color: var(--apple-text);
}

.drawer-close-btn {
  width: 32px;
  height: 32px;
  border-radius: 50%;
  border: none;
  background-color: var(--apple-surface-2);
  font-size: 24px;
  font-weight: var(--apple-font-weight-regular);
  color: var(--apple-text-3);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 0;
  outline: none;
  transition: all var(--apple-duration-fast) var(--apple-ease-standard);
}

.drawer-close-btn:hover {
  color: var(--apple-brand);
  background-color: var(--apple-brand-tint);
}

.drawer-content {
  padding: 24px;
  flex: 1;
  overflow-y: auto;
  background-color: var(--apple-bg);
}

.slider-container {
  width: 100%;
}

.slider-hint {
  display: block;
  font-size: 12px;
  color: var(--apple-text-2);
  margin-top: 8px;
  line-height: 1.5;
}

.tts-slider {
  width: 100%;
}

.tts-slider ::v-deep .el-slider__input {
  width: 80px;
}

.tts-slider ::v-deep .el-input__inner {
  text-align: center;
  padding: 0 8px;
  background-color: var(--apple-surface-2);
  border-radius: var(--apple-radius-sm);
  border: 1px solid transparent;
  color: var(--apple-text);
}

.tts-slider ::v-deep .el-slider__runway {
  background-color: var(--apple-divider);
}

.tts-slider ::v-deep .el-slider__bar {
  background-color: var(--apple-brand);
}

.tts-slider ::v-deep .el-slider__button-wrapper {
  z-index: 1;
}

.tts-slider ::v-deep .el-slider__button {
  border: 2px solid var(--apple-brand);
  background-color: var(--apple-surface);
}

.drawer-footer {
  padding: 16px 24px;
  border-top: 1px solid var(--apple-divider-soft);
  display: flex;
  justify-content: flex-end;
  gap: 12px;
  background-color: var(--apple-surface);
}

.drawer-footer .el-button {
  min-width: 80px;
  border-radius: var(--apple-radius-md);
}

.drawer-footer .el-button:hover {
  transform: translateY(-1px);
}

.drawer-footer .el-button--primary {
  background-color: var(--apple-brand);
  border-color: var(--apple-brand);
}

.drawer-footer .el-button--primary:hover {
  background-color: var(--apple-brand-hover);
  border-color: var(--apple-brand-hover);
}

::v-deep .el-form-item__label {
  font-size: 14px !important;
  color: var(--apple-text) !important;
  font-weight: var(--apple-font-weight-medium);
  padding-bottom: 8px;
}

::v-deep .el-form-item {
  margin-bottom: 24px;
}
.replacement-label {
  color: var(--apple-text);
  font-size: 14px;
  font-weight: var(--apple-font-weight-semibold);
}
.replacement-label i {
  margin-left: 4px;
  color: var(--apple-text-3);
  cursor: pointer;
  font-size: 14px;
  transition: color var(--apple-duration-fast) var(--apple-ease-standard);
}
.replacement-label i:hover {
  color: var(--apple-brand);
}
.replacement-word-select ::v-deep .el-tag {
  background-color: var(--apple-brand-tint);
  color: var(--apple-brand);
  border-radius: var(--apple-radius-sm);
  font-size: 12px;
  font-weight: var(--apple-font-weight-regular);
  border: none;
}

.replacement-word-select ::v-deep .el-tag__close {
  color: var(--apple-brand);
}
.replacement-word-select ::v-deep .el-icon-close {
  background-color: var(--apple-surface);
}
</style>

<style>
.tts-advanced-drawer .el-drawer__header {
  margin-bottom: 0;
  padding: 0;
}

.tts-advanced-drawer .el-drawer__body {
  display: flex;
  flex-direction: column;
  padding: 0;
  background-color: var(--apple-bg);
}

.replacement-word-select {
  width: 100%;
  margin-bottom: 12px;
}

.replacement-label {
  text-align: left;
}

.selected-tags {
  display: flex;
  flex-wrap: wrap;
  gap: 8px;
}

.replacement-tag {
  background-color: var(--apple-brand);
  border-color: var(--apple-brand);
  color: #fff;
}

.tts-tooltip {
  max-height: 60vh !important;
  max-width: 400px !important;
  overflow-y: auto !important;
  scrollbar-width: thin;
  word-break: break-word;
  background-color: var(--apple-surface) !important;
  color: var(--apple-text) !important;
  border: 1px solid var(--apple-divider-soft) !important;
  border-radius: var(--apple-radius-md) !important;
  box-shadow: var(--apple-shadow-raised) !important;
}

.tts-tooltip .popper__arrow {
  display: none !important;
}

.tts-tooltip[x-placement^="top"] .popper__arrow {
  border-top-color: transparent !important;
}

.tts-tooltip[x-placement^="bottom"] .popper__arrow {
  border-bottom-color: transparent !important;
}
</style>
