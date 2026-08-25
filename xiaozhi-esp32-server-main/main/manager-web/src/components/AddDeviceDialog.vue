<template>
  <el-dialog :visible="visible" @close="handleClose" width="24%" center custom-class="add-device-dialog">
    <div class="add-device-title">
      <div class="add-device-icon">
        <img src="@/assets/home/equipment.png" alt="" class="add-device-icon-img" />
      </div>
      {{ $t('device.dialogTitle') }}
    </div>
    <div class="add-device-divider" />
    <div class="add-device-body">
      <div class="form-label">
        <span class="required">*</span>
        <span class="form-label-text"> {{ $t('device.verificationCode') }}</span>
      </div>
      <div class="input-46" style="margin-top: 12px;">
        <el-input :placeholder="$t('device.verificationCodePlaceholder')" v-model="deviceCode" @keyup.enter.native="confirm" />
      </div>
    </div>
    <div class="add-device-actions">
      <div class="dialog-btn dialog-btn--primary" @click="confirm">
        {{ $t('device.confirmButton') }}
      </div>
      <div class="dialog-btn dialog-btn--secondary" @click="cancel">
        {{ $t('device.cancelButton') }}
      </div>
    </div>
  </el-dialog>
</template>

<script>
import Api from '@/apis/api';

export default {
  name: 'AddDeviceDialog',
  props: {
    visible: { type: Boolean, required: true },
    agentId: { type: String, required: true }
  },
  data() {
    return {
      deviceCode: "",
      loading: false,
    }
  },
  methods: {
    confirm() {
      if (!/^\d{6}$/.test(this.deviceCode)) {
        this.$message.error(this.$t('device.input6DigitCode'));
        return;
      }
      this.loading = true;
      Api.device.bindDevice(
        this.agentId,
        this.deviceCode, ({ data }) => {
          this.loading = false;
          if (data.code === 0) {
            this.$emit('refresh');
            this.$message.success({
              message: this.$t('device.bindSuccess'),
              showClose: true
            });
            this.closeDialog();
          } else {
            this.$message.error({
              message: data.msg || this.$t('device.bindFailed'),
              showClose: true
            });
          }
        }
      );
    },
    closeDialog() {
      this.$emit('update:visible', false);
      this.deviceCode = '';

    },
    cancel() {
      this.$emit('update:visible', false)
      this.deviceCode = ""
    },
    handleClose() {
      this.$emit('update:visible', false);
    },
  }
}
</script>

<style scoped>
.add-device-title {
  margin: 0 10px 10px;
  display: flex;
  align-items: center;
  gap: 10px;
  font-weight: var(--apple-font-weight-semibold);
  font-size: var(--apple-font-size-md);
  text-align: left;
  color: var(--apple-text);
}

.add-device-icon {
  width: 36px;
  height: 36px;
  border-radius: 50%;
  background: var(--apple-brand);
  display: flex;
  align-items: center;
  justify-content: center;
}

.add-device-icon-img {
  width: 18px;
  height: 15px;
}

.add-device-divider {
  height: 1px;
  background: var(--apple-divider-soft);
  margin: 0 0 8px;
}

.add-device-body {
  margin: 22px 15px;
}

.form-label {
  font-weight: 400;
  font-size: 14px;
  text-align: left;
  color: var(--apple-text-2);
}

.form-label-text {
  font-size: 11px;
}

.required {
  color: var(--apple-danger);
  display: inline-block;
}

.input-46 {
  border: 1px solid transparent;
  background: var(--apple-surface-2);
  border-radius: var(--apple-radius-md);
  transition: border-color var(--apple-duration-fast) var(--apple-ease-standard),
              box-shadow var(--apple-duration-fast) var(--apple-ease-standard),
              background-color var(--apple-duration-fast) var(--apple-ease-standard);
}

.input-46:focus-within {
  border-color: var(--apple-brand);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.24);
  background-color: var(--apple-surface);
}

.add-device-actions {
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

::v-deep .add-device-dialog {
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
