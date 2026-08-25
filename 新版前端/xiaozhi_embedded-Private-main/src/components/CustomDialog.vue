<template>
  <el-dialog
    :title="title"
    :visible.sync="dialogVisible"
    :width="width"
    :close-on-click-modal="closeOnClickModal"
    :close-on-press-escape="closeOnPressEscape"
    :show-close="showClose"
    :destroy-on-close="destroyOnClose"
    :append-to-body="true"
    :custom-class="customClass"
    class="custom-dialog"
    @close="handleClose"
    @open="handleOpen"
  >
    <template slot="title">
      <slot name="title">
        <div class="dialog-title">
          <img src="@/assets/knowledge-base/level.png" class="title-icon" />
          <span>{{ title }}</span>
        </div>
      </slot>
    </template>
    <slot></slot>
    <template slot="footer">
      <div v-if="footer" class="dialog-footer">
        <CustomButton @click="handleCancel">{{ cancelText }}</CustomButton>
        <CustomButton :loading="confirmLoading" type="confirm" @click="handleConfirm">
          <span class="confirm-inner">
            <img src="@/assets/knowledge-base/star.png" class="confirm-icon" />
            {{ confirmText }}
          </span>
        </CustomButton>
      </div>
    </template>
  </el-dialog>
</template>

<script>
import CustomButton from './CustomButton.vue';
import { gsap } from '@/utils/gsap';
export default {
  name: "CustomDialog",
  props: {
    title: {
      type: String,
      default: ""
    },
    visible: {
      type: Boolean,
      default: false
    },
    confirmLoading: {
      type: Boolean,
      default: false
    },
    width: {
      type: String,
      default: "600px"
    },
    footer: {
      type: Boolean,
      default: true
    },
    closeOnClickModal: {
      type: Boolean,
      default: false
    },
    closeOnPressEscape: {
      type: Boolean,
      default: true
    },
    showClose: {
      type: Boolean,
      default: true
    },
    destroyOnClose: {
      type: Boolean,
      default: true
    },
    customClass: {
      type: String,
      default: ""
    },
    cancelText: {
      type: String,
      default: "取消"
    },
    confirmText: {
      type: String,
      default: "确认保存"
    }
  },
  data() {
    return {
      dialogVisible: this.visible
    };
  },
  components: {
    CustomButton
  },
  watch: {
    visible(val) {
      this.dialogVisible = val;
    }
  },
  methods: {
    handleClose() {
      this.dialogVisible = false;
      this.$emit("update:visible", false);
      this.$emit("close");
    },
    handleOpen() {
      this.animateDialogOpen();
      this.$emit("open");
    },
    handleCancel() {
      this.dialogVisible = false;
      this.$emit("update:visible", false);
      this.$emit("cancel");
    },
    handleConfirm() {
      this.$emit("confirm");
    },
    animateDialogOpen() {
      this.$nextTick(() => {
        const dlg = this.$el && this.$el.querySelector && this.$el.querySelector('.el-dialog');
        if (!dlg) return;
        gsap.killTweensOf(dlg);
        gsap.from(dlg, {
          opacity: 0.001,
          duration: 0.22,
          ease: 'power2.out',
          clearProps: 'opacity'
        });
      });
    }
  },
  mounted() {
    if (this.dialogVisible) {
      this.animateDialogOpen();
    }
  },
  beforeDestroy() {
    const dlg = this.$el && this.$el.querySelector && this.$el.querySelector('.el-dialog');
    if (dlg) gsap.killTweensOf(dlg);
  }
};
</script>

<style lang="scss" scoped>
.custom-dialog {
  ::v-deep .el-dialog {
    border-radius: var(--apple-radius-lg);
    overflow: hidden;
    background: var(--apple-surface);
    box-shadow: var(--apple-shadow-overlay);
  }

  ::v-deep .el-dialog__header {
    padding: var(--apple-space-4) var(--apple-space-5) var(--apple-space-3);
    background: var(--apple-surface);
    border-bottom: 1px solid var(--apple-divider-soft);
    text-align: left;
  }

  ::v-deep .el-dialog__title {
    font-size: var(--apple-font-size-md);
    font-weight: var(--apple-font-weight-semibold);
    color: var(--apple-text);
  }

  .dialog-title {
    font-size: var(--apple-font-size-md);
    display: inline-flex;
    align-items: center;
    color: var(--apple-text);
    > span {
      line-height: 1.2;
      font-weight: var(--apple-font-weight-semibold);
    }
  }

  .title-icon {
    width: 24px;
    height: 24px;
    margin-right: var(--apple-space-2);
  }

  ::v-deep .el-dialog__headerbtn {
    top: var(--apple-space-3);
    right: var(--apple-space-4);
    width: 32px;
    height: 32px;
    border: none;
    border-radius: 50%;
    background: var(--apple-surface-2);
    display: flex;
    align-items: center;
    justify-content: center;
    transition:
      background-color var(--apple-duration-fast) var(--apple-ease-standard),
      color var(--apple-duration-fast) var(--apple-ease-standard);

    .el-dialog__close {
      font-size: 18px;
      color: var(--apple-text-2);
      position: static;
      transform: none;
      transition: color var(--apple-duration-fast) var(--apple-ease-standard);
    }

    &:hover {
      background: var(--apple-surface-2);

      .el-dialog__close {
        color: var(--apple-text);
      }
    }
  }

  ::v-deep .el-dialog__body {
    padding: var(--apple-space-5);
    background: var(--apple-surface);
    color: var(--apple-text);
  }

  ::v-deep .el-dialog__footer {
    padding: var(--apple-space-3) var(--apple-space-5) var(--apple-space-4);
    background: var(--apple-surface);
    border-top: 1px solid var(--apple-divider-soft);
  }

  .dialog-footer {
    display: flex;
    justify-content: flex-end;
    gap: var(--apple-space-2);

    .el-button {
      padding: 10px 20px;
      display: flex;
      align-items: center;
    }

    .el-button--primary {
      background: var(--apple-brand);
      border-color: var(--apple-brand);
      color: #ffffff;

      &:hover,
      &:focus {
        background: var(--apple-brand-hover);
        border-color: var(--apple-brand-hover);
        color: #ffffff;
      }
    }

    .confirm-inner {
      display: inline-flex;
      align-items: center;
    }

    .confirm-icon {
      width: 16px;
      height: 16px;
      margin-right: var(--apple-space-1);
    }
  }
}
</style>
