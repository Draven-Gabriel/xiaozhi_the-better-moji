<template>
  <CustomDialog
    :title="title"
    :visible.sync="visible"
    width="600px"
    class="param-dialog-wrapper"
    @confirm="submit"
    @close="cancel"
    :confirmLoading="saving"
  >
    <div class="dialog-container">
      <el-form :model="form" :rules="rules" ref="form" label-width="auto" label-position="left" class="param-form">
        <el-form-item :label="$t('paramDialog.paramCode')" prop="paramCode" class="form-item">
          <el-input v-model="form.paramCode" :placeholder="$t('paramDialog.paramCodePlaceholder')"
            class="custom-input"></el-input>
        </el-form-item>

        <el-form-item :label="$t('paramDialog.valueType')" prop="valueType" class="form-item">
          <el-select
            v-model="form.valueType"
            :placeholder="$t('paramDialog.valueTypePlaceholder')"
            class="custom-select"
          >
            <el-option
              v-for="item in valueTypeOptions"
              :key="item.value"
              :label="$t(`paramDialog.${item.value}Type`)"
              :value="item.value"
            />
          </el-select>
        </el-form-item>

        <el-form-item :label="$t('paramDialog.paramValue')" prop="paramValue" class="form-item">
          <el-input
            v-if="form.valueType !== 'json' && form.valueType !== 'array'"
            v-model="form.paramValue"
            :placeholder="$t('paramDialog.paramValuePlaceholder')"
            class="custom-input"
          ></el-input>
          <el-input
            v-else
            type="textarea"
            v-model="form.paramValue"
            :placeholder="$t('paramDialog.paramValuePlaceholder')"
            :rows="6"
            class="custom-textarea"
          ></el-input>
        </el-form-item>

        <el-form-item :label="$t('paramDialog.remark')" prop="remark" class="form-item remark-item">
          <el-input type="textarea" v-model="form.remark" :placeholder="$t('paramDialog.remarkPlaceholder')" :rows="3"
            class="custom-textarea"></el-input>
        </el-form-item>
      </el-form>
    </div>
  </CustomDialog>
</template>

<script>
import CustomDialog from './CustomDialog.vue';
export default {
  props: {
    title: {
      type: String,
      default: '新增参数'
    },
    visible: {
      type: Boolean,
      default: false
    },
    form: {
      type: Object,
      default: () => ({
        id: null,
        paramCode: '',
        paramValue: '',
        valueType: 'string',
        remark: ''
      })
    }
  },
  components: {
    CustomDialog
  },
  data() {
    return {
      dialogKey: Date.now(),
      saving: false,
      valueTypeOptions: [
        { value: 'string' },
        { value: 'number' },
        { value: 'boolean' },
        { value: 'array' },
        { value: 'json' }
      ],
      rules: {
        paramCode: [
          { required: true, message: this.$t('paramDialog.requiredParamCode'), trigger: "blur" }
        ],
        paramValue: [
          { required: true, message: this.$t('paramDialog.requiredParamValue'), trigger: "blur" }
        ],
        valueType: [
          { required: true, message: this.$t('paramDialog.requiredValueType'), trigger: "change" }
        ]
      }
    };
  },
  methods: {
    submit() {
      this.$refs.form.validate((valid) => {
        if (valid) {
          const submitData = { ...this.form };

          // 如果是 array 类型，校验格式并转换
          if (submitData.valueType === 'array' && submitData.paramValue) {
            const lines = submitData.paramValue.split('\n').filter(line => line.trim());

            // 检查除最后一行外的每行是否以分号结尾
            for (let i = 0; i < lines.length - 1; i++) {
              if (!lines[i].trim().endsWith(';')) {
                this.$message.error('数组格式错误，需要使用英文分号结尾');
                return;
              }
            }

            const items = lines
              .map(item => item.trim().replace(/;$/, ''))
              .filter(item => item);
            submitData.paramValue = items.join(';');
          }
          // 如果是 json 类型，压缩 JSON 格式后再提交
          else if (submitData.valueType === 'json' && submitData.paramValue) {
            try {
              const parsed = JSON.parse(submitData.paramValue);
              submitData.paramValue = JSON.stringify(parsed);
            } catch (e) {
              // 如果解析失败，保持原值
            }
          }

          this.saving = true; // 开始加载
          this.$emit('submit', submitData);
        }
      });
    },
    cancel() {
      this.saving = false; // 取消时重置状态
      this.dialogKey = Date.now();
      this.$emit('cancel');
    },

    // 提供给父组件调用以重置saving状态
    resetSaving() {
      this.saving = false;
    }
  },
  watch: {
    visible(newVal) {
      if (newVal) {
        if (this.form.paramValue) {
          // 如果是 json 类型，格式化显示
          if (this.form.valueType === 'json') {
            try {
              const parsed = JSON.parse(this.form.paramValue);
              this.form.paramValue = JSON.stringify(parsed, null, 2);
            } catch (e) {
              // 如果解析失败，保持原值
            }
          }
          // 如果是 array 类型，将分号分隔的字符串转换为每行一个项目
          else if (this.form.valueType === 'array') {
            const items = this.form.paramValue.split(';').filter(item => item.trim());
            this.form.paramValue = items.join(';\n');
          }
        }
      } else {
        // 当对话框关闭时，重置saving状态
        this.saving = false;
      }
    }
  }
};
</script>

<style scoped lang="scss">
.param-dialog-wrapper {
  .param-form {
    .form-item {
      margin-bottom: 20px;
      :deep(.el-form-item__label) {
        color: var(--apple-text);
        font-weight: 500;
        padding-right: 12px;
        text-align: right;
        font-size: 14px;
        letter-spacing: 0.2px;
      }
    }

    .custom-input {
      :deep(.el-input__inner) {
        background-color: var(--apple-surface-2);
        border-radius: var(--apple-radius-md);
        border: 1px solid transparent;
        height: 42px;
        padding: 0 14px;
        transition: all var(--apple-duration-fast) var(--apple-ease-standard);
        font-size: 14px;
        color: var(--apple-text);
        box-shadow: none;

        &:focus {
          border-color: var(--apple-brand);
          box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.24);
          background-color: var(--apple-surface);
        }

        &::placeholder {
          color: var(--apple-text-3);
          font-weight: 400;
        }
      }
    }

    .custom-select {
      width: 100%;

      :deep(.el-input__inner) {
        background-color: var(--apple-surface-2);
        border-radius: var(--apple-radius-md);
        border: 1px solid transparent;
        height: 42px;
        transition: all var(--apple-duration-fast) var(--apple-ease-standard);
        font-size: 14px;
        color: var(--apple-text);
        box-shadow: none;

        &:focus {
          border-color: var(--apple-brand);
          box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.24);
          background-color: var(--apple-surface);
        }

        &::placeholder {
          color: var(--apple-text-3);
          font-weight: 400;
        }
      }
    }

    .custom-textarea {
      :deep(.el-textarea__inner) {
        background-color: var(--apple-surface-2);
        border-radius: var(--apple-radius-md);
        border: 1px solid transparent;
        padding: 12px 14px;
        transition: all var(--apple-duration-fast) var(--apple-ease-standard);
        font-size: 14px;
        color: var(--apple-text);
        box-shadow: none;
        line-height: 1.5;

        &:focus {
          border-color: var(--apple-brand);
          box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.24);
          background-color: var(--apple-surface);
        }

        &::placeholder {
          color: var(--apple-text-3);
          font-weight: 400;
        }
      }
    }

    .remark-item :deep(.el-form-item__label) {
      margin-top: -4px;
    }
  }
}
</style>
