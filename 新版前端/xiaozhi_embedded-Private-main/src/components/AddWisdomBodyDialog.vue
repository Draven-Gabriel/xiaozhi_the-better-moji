<template>
  <el-dialog :visible="visible" @close="handleClose" width="25%" center append-to-body :close-on-click-modal="false" :close-on-press-escape="false" @open="handleOpen" custom-class="add-wisdom-body-dialog">
    <div class="dialog-title">
      <div class="dialog-icon">
        <img loading="lazy" src="@/assets/home/equipment.png" alt="" class="dialog-icon-img" />
      </div>
      {{ $t('addAgentDialog.title') }}
    </div>
    <div class="dialog-divider" />
    <div class="dialog-body">
      <div class="form-label">
        <span class="required">*</span> {{ $t('addAgentDialog.agentName') }}：
      </div>
      <div class="input-46" style="margin-top: 12px;">
        <el-input maxLength="64" ref="inputRef" :placeholder="$t('addAgentDialog.placeholder')" v-model="wisdomBodyName" @keyup.enter.native="confirm" />
      </div>
    </div>
    <div class="dialog-actions">
      <div class="dialog-btn dialog-btn--primary" @click="confirm">
        {{ $t('addAgentDialog.confirm') }}
      </div>
      <div class="dialog-btn dialog-btn--secondary" @click="cancel">
        {{ $t('addAgentDialog.cancel') }}
      </div>
    </div>
  </el-dialog>
</template>

<script>
import Api from '@/apis/api';

export default {
  name: 'AddWisdomBodyDialog',
  props: {
    visible: { type: Boolean, required: true }
  },
  data() {
    return {
      wisdomBodyName: "",
      inputRef: null
    }
  },
  methods: {
    handleOpen() {
      this.$nextTick(() => {
        this.$refs.inputRef.focus();
      });
    },
    confirm() {
      if (!this.wisdomBodyName.trim()) {
        this.$message.error(this.$t('addAgentDialog.nameRequired'));
        return;
      }
      Api.agent.addAgent(this.wisdomBodyName, (res) => {
        this.$message.success({
          message: this.$t('addAgentDialog.addSuccess'),
          showClose: true
        });
        this.$emit('confirm', res);
        this.$emit('update:visible', false);
        this.wisdomBodyName = "";
      });
    },
    cancel() {
      this.$emit('update:visible', false)
      this.wisdomBodyName = ""
    },
    handleClose() {
      this.$emit('update:visible', false);
    },
  }
}
</script>

<style scoped>
.dialog-title {
  margin: 0 10px 10px;
  display: flex;
  align-items: center;
  gap: 10px;
  font-weight: var(--apple-font-weight-semibold);
  font-size: var(--apple-font-size-md);
  text-align: left;
  color: var(--apple-text);
}

.dialog-icon {
  width: 36px;
  height: 36px;
  border-radius: 50%;
  background: var(--apple-brand);
  display: flex;
  align-items: center;
  justify-content: center;
}

.dialog-icon-img {
  width: 18px;
  height: 15px;
}

.dialog-divider {
  height: 1px;
  background: var(--apple-divider-soft);
  margin: 0;
}

.dialog-body {
  margin: 22px 15px;
}

.form-label {
  font-weight: 400;
  text-align: left;
  color: var(--apple-text-2);
}

.required {
  color: var(--apple-danger);
  display: inline-block;
}

.input-46 {
  border: 1px solid transparent;
  background: var(--apple-surface-2);
  border-radius: var(--apple-radius-md);
  height: 40px;
  display: flex;
  align-items: center;
  transition: border-color var(--apple-duration-fast) var(--apple-ease-standard),
              box-shadow var(--apple-duration-fast) var(--apple-ease-standard),
              background-color var(--apple-duration-fast) var(--apple-ease-standard);
}

.input-46:focus-within {
  border-color: var(--apple-brand);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.24);
  background-color: var(--apple-surface);
}

.dialog-actions {
  display: flex;
  margin: 8px 15px 4px;
  gap: 8px;
}

.dialog-btn {
  cursor: pointer;
  flex: 1;
  border-radius: var(--apple-radius-full);
  background: var(--apple-brand);
  height: 40px;
  font-weight: 500;
  font-size: 13px;
  color: #ffffff;
  line-height: 40px;
  text-align: center;
  border: 1px solid var(--apple-brand);
  transition: background-color var(--apple-duration-fast) var(--apple-ease-standard),
              border-color var(--apple-duration-fast) var(--apple-ease-standard),
              color var(--apple-duration-fast) var(--apple-ease-standard),
              transform var(--apple-duration-fast) var(--apple-ease-standard),
              box-shadow var(--apple-duration-fast) var(--apple-ease-standard);
}

.dialog-btn--primary:hover {
  background: var(--apple-brand-hover);
  border-color: var(--apple-brand-hover);
  transform: translateY(-1px);
  box-shadow: 0 4px 12px rgba(0, 113, 227, 0.28);
}

.dialog-btn--secondary {
  background: var(--apple-brand-tint);
  border: 1px solid transparent;
  color: var(--apple-brand);
}

.dialog-btn--secondary:hover {
  background: var(--apple-brand-tint);
  border-color: var(--apple-brand);
  color: var(--apple-brand-active);
  transform: translateY(-1px);
}

::v-deep .el-dialog {
  border-radius: var(--apple-radius-lg);
  box-shadow: var(--apple-shadow-overlay);
  background: var(--apple-surface);
}

::v-deep .el-dialog__headerbtn {
  display: none;
}

::v-deep .el-dialog__body {
  padding: 4px 6px;
  background: var(--apple-surface);
}

::v-deep .el-dialog__header {
  padding: 10px;
}

::v-deep .add-wisdom-body-dialog {
  border-radius: var(--apple-radius-lg);
  overflow: hidden;
}

::v-deep .el-input__inner {
  background: transparent !important;
  border: none !important;
  color: var(--apple-text) !important;
  height: 40px;
}

::v-deep .el-input__inner::placeholder {
  color: var(--apple-text-3);
}
</style>
