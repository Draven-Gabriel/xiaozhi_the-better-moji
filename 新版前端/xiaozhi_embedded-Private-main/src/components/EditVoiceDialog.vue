<template>
  <el-dialog :visible.sync="dialogVisible" width="900px" @close="handleClose" class="compact-dialog" :append-to-body="true">
    <el-form :model="voiceForm" :rules="rules" ref="voiceForm" label-width="auto">
      <el-row :gutter="20">
        <el-col :span="12">
          <el-form-item :label="$t('editVoiceDialog.voiceCode')" prop="voiceCode">
            <el-input v-model="voiceForm.voiceCode" :placeholder="$t('editVoiceDialog.voiceCodePlaceholder')" class="compact-input"></el-input>
          </el-form-item>
        </el-col>
        <el-col :span="12">
          <el-form-item :label="$t('editVoiceDialog.voiceName')" prop="voiceName">
            <el-input v-model="voiceForm.voiceName" :placeholder="$t('editVoiceDialog.voiceNamePlaceholder')" class="compact-input"></el-input>
          </el-form-item>
        </el-col>
      </el-row>
      <el-row :gutter="20">
        <el-col :span="12">
          <el-form-item :label="$t('editVoiceDialog.languageType')" prop="languageType">
            <el-input v-model="voiceForm.languageType" :placeholder="$t('editVoiceDialog.languageTypePlaceholder')" class="compact-input"></el-input>
          </el-form-item>
        </el-col>
        <el-col :span="12">
          <el-form-item :label="$t('editVoiceDialog.sortNumber')" prop="sortNumber">
            <el-input-number v-model="voiceForm.sortNumber" :min="1" :controls="false" class="compact-number"></el-input-number>
          </el-form-item>
        </el-col>
      </el-row>

      <el-form-item :label="$t('editVoiceDialog.remark')" prop="remark">
        <el-input v-model="voiceForm.remark" type="textarea" :rows="2" :placeholder="$t('editVoiceDialog.remarkPlaceholder')" class="compact-textarea"
        ></el-input>
        <div class="audio-controls">
           <div class="audio-player">
      <audio
        :src="audioUrl"
        controls
        preload="metadata"
        class="custom-audio"
      ></audio>
    </div>
          <el-button type="primary" size="mini" class="preview-btn">{{ $t('editVoiceDialog.generatePreview') }}</el-button>
        </div>
      </el-form-item>
    </el-form>

    <div slot="footer" class="dialog-footer">
      <el-button type="primary" @click="handleSave">{{ $t('button.save') }}</el-button>
      <el-button @click="handleClose">{{ $t('button.close') }}</el-button>
    </div>
  </el-dialog>
</template>

<script>
export default {
  name: 'EditVoiceDialog',
  props: {
    showDialog: Boolean,
    voiceData: {
      type: Object,
      default: () => ({
        voiceCode: 'wawaxiaohe',
        voiceName: this.$t('editVoiceDialog.defaultVoiceName'),
        languageType: this.$t('editVoiceDialog.defaultLanguageType'),
        sortNumber: 123
      })
    }
  },
  data() {
    return {
      dialogVisible: this.showDialog,
      voiceForm: { ...this.voiceData },
      audioUrl: 'http://music.163.com/song/media/outer/url?id=447925558.mp3',
      generatedAudio: null,
      rules: {
        voiceCode: [{ required: true, message: this.$t('editVoiceDialog.requiredVoiceCode'), trigger: 'blur' }],
        voiceName: [{ required: true, message: this.$t('editVoiceDialog.requiredVoiceName'), trigger: 'blur' }]
      }
    }
  },
  watch: {
    showDialog(newVal) {
      this.dialogVisible = newVal
      if (newVal) this.voiceForm = { ...this.voiceData }
    }
  },
  methods: {
    handleClose() {
      this.dialogVisible = false
      this.$emit('update:showDialog', false)
    },
    handleSave() {
      this.$refs.voiceForm.validate(valid => {
        if (valid) this.$emit('save', this.voiceForm)
      })
    },
  }
}
</script>

<style scoped>
.compact-dialog {
  /deep/ .el-dialog {
    border-radius: var(--apple-radius-lg);
    overflow: hidden;
    background: var(--apple-surface);
    box-shadow: var(--apple-shadow-overlay);
  }

  /deep/ .el-dialog__header {
    padding: var(--apple-space-4) var(--apple-space-5) var(--apple-space-3);
    background: var(--apple-surface);
    border-bottom: 1px solid var(--apple-divider-soft);
  }

  /deep/ .el-dialog__title {
    color: var(--apple-text);
    font-weight: var(--apple-font-weight-semibold);
    font-size: var(--apple-font-size-md);
  }

  /deep/ .el-dialog__body {
    padding: 20px;
    background: var(--apple-surface);
  }

  .el-form-item {
    margin-bottom: 16px;
  }

  /deep/ .el-form-item__label {
    color: var(--apple-text);
    font-weight: 500;
  }

  .compact-input {
    width: 100%;
  }

  /deep/ .el-input__inner {
    background-color: var(--apple-surface-2);
    border-radius: var(--apple-radius-md);
    border: 1px solid transparent;
    color: var(--apple-text);
    height: 40px;
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

  /deep/ .el-textarea__inner {
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

  .compact-number {
    width: 100%;
    /deep/ .el-input__inner {
      padding-right: 10px;
    }
  }

  .compact-textarea {
    width: 100%;
    margin-bottom: 8px;
  }

  .audio-controls {
    display: flex;
    align-items: center;
    justify-content: flex-start;
    gap: 16px;
    margin-top: 8px;


    .preview-btn {
      padding: 7px 15px;
      border-radius: var(--apple-radius-md);
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
      }
    }

    .custom-audio {
      border-radius: var(--apple-radius-md);
    }
  }

  .dialog-footer {
    padding: 16px 20px;
    text-align: right;
    border-top: 1px solid var(--apple-divider-soft);
    background: var(--apple-surface);

    .el-button {
      min-width: 80px;
      border-radius: var(--apple-radius-md);
      transition: background-color var(--apple-duration-fast) var(--apple-ease-standard),
                  border-color var(--apple-duration-fast) var(--apple-ease-standard),
                  color var(--apple-duration-fast) var(--apple-ease-standard),
                  transform var(--apple-duration-fast) var(--apple-ease-standard);
    }

    .el-button + .el-button {
      margin-left: 8px;
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
}
</style>
