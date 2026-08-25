<template>
  <el-dialog :title="title" :visible="dialogVisible" width="600px" class="address-book-dialog" @close="handleClose">
    <el-form ref="addressBookForm" :model="form" :rules="rules" label-width="100px" size="medium">
      <el-form-item :label="$t('addressBookDialog.name')" prop="name">
        <el-input v-model="form.name" :placeholder="$t('addressBookDialog.namePlaceholder')" clearable></el-input>
      </el-form-item>
      <el-form-item :label="$t('addressBookDialog.description')" prop="description">
        <el-input v-model="form.description" :placeholder="$t('addressBookDialog.descriptionPlaceholder')"
          type="textarea" :rows="4" maxlength="300" show-word-limit></el-input>
      </el-form-item>
      <el-form-item :label="$t('addressBookDialog.status')" prop="status">
        <el-switch
          v-model="form.status"
          :active-value="1"
          :inactive-value="0"
          active-color="#0071e3"
          inactive-color="#d2d2d7"
        />
      </el-form-item>
    </el-form>
    <div slot="footer" class="dialog-footer">
      <el-button @click="handleClose">{{ $t('addressBookDialog.cancel') }}</el-button>
      <el-button type="primary" @click="handleSubmit">{{ $t('addressBookDialog.confirm') }}</el-button>
    </div>
  </el-dialog>
</template>

<script>
export default {
  name: "AddressBookDialog",
  props: {
    title: {
      type: String,
      default: ""
    },
    visible: {
      type: Boolean,
      default: false
    },
    form: {
      type: Object,
      default: () => ({
        id: null,
        name: "",
        description: "",
        status: 1
      })
    }
  },
  data() {
    return {
      dialogVisible: this.visible,
      rules: {
        name: [
          {
            required: true,
            message: this.$t("addressBookDialog.nameRequired"),
            trigger: "blur"
          },
          {
            min: 1,
            max: 50,
            message: this.$t("addressBookDialog.nameLength"),
            trigger: "blur"
          },
          {
            pattern: /^[一-龥a-zA-Z0-9\s-_]+$/,
            message: this.$t("addressBookDialog.namePattern"),
            trigger: "blur"
          }
        ],
        description: [
          {
            required: true,
            message: this.$t("addressBookDialog.descriptionRequired"),
            trigger: "blur"
          },
          {
            max: 300,
            message: this.$t("addressBookDialog.descriptionLength"),
            trigger: "blur"
          }
        ]
      }
    };
  },
  watch: {
    visible(val) {
      this.dialogVisible = val;
      if (val) {
        if (this.$refs.addressBookForm) {
          this.$refs.addressBookForm.clearValidate();
        }
      }
    }
  },
  methods: {
    handleClose() {
      if (this.$refs.addressBookForm) {
        this.$refs.addressBookForm.clearValidate();
      }
      this.dialogVisible = false;
      this.$emit("update:visible", false);
    },
    handleSubmit() {
      this.$refs.addressBookForm.validate(valid => {
        if (valid) {
          this.$emit("submit", {
            ...this.form
          });
        }
      });
    }
  }
};
</script>

<style lang="scss" scoped>
.address-book-dialog {
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
  }

  ::v-deep .el-dialog__title {
    color: var(--apple-text);
    font-weight: var(--apple-font-weight-semibold);
    font-size: var(--apple-font-size-md);
  }

  ::v-deep .el-dialog__body {
    padding: 20px 30px;
    background: var(--apple-surface);
  }

  ::v-deep .el-dialog__footer {
    padding: var(--apple-space-3) var(--apple-space-5) var(--apple-space-4);
    background: var(--apple-surface);
    border-top: 1px solid var(--apple-divider-soft);
  }

  ::v-deep .el-form-item {
    margin-bottom: 20px;
  }

  ::v-deep .el-form-item__label {
    font-weight: 500;
    color: var(--apple-text);
    font-size: 14px;
  }

  ::v-deep .el-input {
    .el-input__inner {
      height: 38px;
      font-size: 14px;
      background-color: var(--apple-surface-2);
      border-radius: var(--apple-radius-md);
      border: 1px solid transparent;
      color: var(--apple-text);
      transition: all var(--apple-duration-fast) var(--apple-ease-standard);

      &:focus {
        border-color: var(--apple-brand);
        box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.24);
        background-color: var(--apple-surface);
      }

      &::placeholder {
        color: var(--apple-text-3);
      }
    }
  }

  ::v-deep .el-textarea {
    .el-textarea__inner {
      font-size: 14px;
      background-color: var(--apple-surface-2);
      border-radius: var(--apple-radius-md);
      border: 1px solid transparent;
      color: var(--apple-text);
      transition: all var(--apple-duration-fast) var(--apple-ease-standard);

      &:focus {
        border-color: var(--apple-brand);
        box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.24);
        background-color: var(--apple-surface);
      }

      &::placeholder {
        color: var(--apple-text-3);
      }
    }
  }

  .dialog-footer {
    display: flex;
    justify-content: flex-end;
    gap: 10px;
  }

  /deep/ .el-button {
    border-radius: var(--apple-radius-md);
    transition: background-color var(--apple-duration-fast) var(--apple-ease-standard),
                border-color var(--apple-duration-fast) var(--apple-ease-standard),
                color var(--apple-duration-fast) var(--apple-ease-standard),
                transform var(--apple-duration-fast) var(--apple-ease-standard);
  }

  /deep/ .el-button--default {
    background: var(--apple-surface);
    border-color: var(--apple-divider);
    color: var(--apple-text);

    &:hover {
      background: var(--apple-surface-2);
      border-color: var(--apple-divider);
      color: var(--apple-text);
      transform: translateY(-1px);
    }
  }

  /deep/ .el-button--primary {
    background: var(--apple-brand);
    border-color: var(--apple-brand);
    color: #ffffff;

    &:hover,
    &:focus {
      background: var(--apple-brand-hover);
      border-color: var(--apple-brand-hover);
      color: #ffffff;
      transform: translateY(-1px);
      box-shadow: 0 4px 12px rgba(0, 113, 227, 0.28);
    }
  }
}
</style>