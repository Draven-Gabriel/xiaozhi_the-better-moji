<template>
  <el-dialog :visible.sync="visible" width="400px" center custom-class="view-password-dialog">
    <div class="dialog-title">
      <div class="dialog-icon">
        <img src="@/assets/login/shield.png" alt="" class="dialog-icon-img" />
      </div>
      用户新密码
    </div>
    <div class="dialog-divider" />
    <div class="dialog-body">
      <div class="form-label">
        <span class="required">*</span>
        用户新密码：
      </div>
      <div class="input-46" style="margin-top: 12px;">
        <el-input v-model="password" type="text" :readonly="true" style="font-weight: 600;" />
      </div>
    </div>
    <div class="dialog-actions">
      <div class="dialog-btn dialog-btn--secondary" @click="closeDialog">
        关闭
      </div>
      <div class="dialog-btn dialog-btn--primary" @click="copyPassword">
        复制密码
      </div>
    </div>
  </el-dialog>
</template>

<script>
export default {
  name: 'ViewPasswordDialog',
  props: {
    visible: { type: Boolean, required: true },
    password: { type: String, default: '' }
  },
  methods: {
    closeDialog() {
      this.$emit('update:visible', false)
    },
    copyPassword() {
      navigator.clipboard.writeText(this.password)
      this.$message.success({
        message: '密码已复制',
        showClose: true
      })
    }
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
  width: 19px;
  height: 23px;
  filter: brightness(0) invert(1);
}

.dialog-divider {
  height: 1px;
  background: var(--apple-divider-soft);
  margin: 0 0 4px;
}

.dialog-body {
  margin: 22px 15px;
}

.form-label {
  font-weight: 400;
  font-size: 14px;
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
  display: flex;
  align-items: center;
  height: 40px;
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
  height: 40px;
  font-weight: 500;
  font-size: 13px;
  line-height: 40px;
  text-align: center;
  border: 1px solid transparent;
  transition: background-color var(--apple-duration-fast) var(--apple-ease-standard),
              border-color var(--apple-duration-fast) var(--apple-ease-standard),
              color var(--apple-duration-fast) var(--apple-ease-standard),
              transform var(--apple-duration-fast) var(--apple-ease-standard),
              box-shadow var(--apple-duration-fast) var(--apple-ease-standard);
}

.dialog-btn--primary {
  background: var(--apple-brand);
  border-color: var(--apple-brand);
  color: #ffffff;
}

.dialog-btn--primary:hover {
  background: var(--apple-brand-hover);
  border-color: var(--apple-brand-hover);
  transform: translateY(-1px);
  box-shadow: 0 4px 12px rgba(0, 113, 227, 0.28);
}

.dialog-btn--secondary {
  background: var(--apple-brand-tint);
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

::v-deep .view-password-dialog {
  border-radius: var(--apple-radius-lg);
  overflow: hidden;
}

::v-deep .el-input__inner {
  background-color: transparent !important;
  cursor: default !important;
  color: var(--apple-text) !important;
  border: none !important;
  font-weight: 600;
  height: 40px;
}
</style>
