<template>
  <CustomDialog
    :title="title"
    :visible.sync="dialogVisible"
    width="600px"
    @confirm="submit"
    @close="cancel"
    :confirmLoading="saving"
  >
    <el-form :model="form" :rules="rules" ref="form" label-width="auto" label-position="left">
      <el-form-item :label="$t('dictDataDialog.dictLabel')" prop="dictLabel" class="form-item">
        <el-input v-model="form.dictLabel" :placeholder="$t('dictDataDialog.dictLabelPlaceholder')" />
      </el-form-item>
      <el-form-item :label="$t('dictDataDialog.dictValue')" prop="dictValue" class="form-item">
        <el-input v-model="form.dictValue" :placeholder="$t('dictDataDialog.dictValuePlaceholder')" />
      </el-form-item>
      <el-form-item :label="$t('dictDataDialog.sort')" prop="sort" class="form-item">
        <el-input-number v-model="form.sort" :min="0" :max="999" class="custom-input-number"></el-input-number>
      </el-form-item>
    </el-form>
  </CustomDialog>
</template>

<script>
import CustomDialog from './CustomDialog.vue';
export default {
  name: 'DictDataDialog',
  props: {
    visible: {
      type: Boolean,
      default: false
    },
    title: {
      type: String,
      default: () => this.$t('dictDataDialog.addDictData')
    },
    dictData: {
      type: Object,
      default: () => ({})
    },
    dictTypeId: {
      type: [Number, String],
      default: null
    }
  },
  components: {
    CustomDialog
  },
  data() {
    return {
      dialogVisible: this.visible,
      saving: false,
      form: {
        id: null,
        dictTypeId: null,
        dictLabel: '',
        dictValue: '',
        sort: 0
      },
      rules: {
        dictLabel: [{ required: true, message: this.$t('dictDataDialog.requiredDictLabel'), trigger: 'blur' }],
        dictValue: [{ required: true, message: this.$t('dictDataDialog.requiredDictValue'), trigger: 'blur' }]
      }
    };
  },
  watch: {
    visible(val) {
      this.dialogVisible = val;
    },
    dialogVisible(val) {
      this.$emit('update:visible', val);
      if (!val) {
        this.saving = false;
      }
    },
    dictData: {
      handler(val) {
        if (val) {
          this.form = { ...val };
        }
      },
      immediate: true
    },
    dictTypeId: {
      handler(val) {
        if (val) {
          this.form.dictTypeId = val;
        }
      },
      immediate: true
    }
  },
  methods: {
    submit() {
      this.$refs.form.validate((valid) => {
        if (valid) {
          this.saving = true;
          this.$emit('save', this.form);
        }
      });
    },
    cancel() {
      this.saving = false;
      this.$emit('cancel');
    },
    resetSaving() {
      this.saving = false;
    }
  }
};
</script>

<style scoped lang="scss">
.custom-input-number {
  width: 100%;
}

::v-deep .el-form-item__label {
  color: var(--apple-text);
  font-weight: 500;
}

::v-deep .el-input__inner {
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

::v-deep .el-input-number {
  width: 100%;
  line-height: 38px;
  .el-input__inner {
    text-align: left;
  }
}

::v-deep .el-input-number__decrease,
::v-deep .el-input-number__increase {
  background: var(--apple-surface);
  border-color: var(--apple-divider-soft);
  color: var(--apple-text-2);
}

::v-deep .el-input-number__decrease:hover,
::v-deep .el-input-number__increase:hover {
  color: var(--apple-brand);
}

::v-deep .el-input-number.is-controls-right .el-input-number__decrease,
::v-deep .el-input-number.is-controls-right .el-input-number__increase {
  border-bottom-color: var(--apple-divider-soft);
}
</style>
