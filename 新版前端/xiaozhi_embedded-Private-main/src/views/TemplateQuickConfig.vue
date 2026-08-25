<template>
  <div class="welcome">
    <HeaderBar />

    <div class="operation-bar">
      <h2 class="page-title">{{ $t('templateQuickConfig.title') }}</h2>
    </div>

    <div class="main-wrapper">
      <div class="content-panel">
        <div class="content-area">
          <el-card class="config-card" shadow="never">
              <div class="config-header">
              <!-- 使用角色配置页面相同的彩色图标效果 -->
              <div class="header-icon">
                <img loading="lazy" src="@/assets/home/setting-user.png" alt="">
              </div>
              <span class="header-title">{{ form.agentName }}</span>
              <div class="header-actions">
                <el-button type="primary" class="save-btn" @click="saveConfig">{{ $t('templateQuickConfig.saveConfig') }}</el-button>
                <el-button class="reset-btn" @click="resetConfig">{{ $t('templateQuickConfig.resetConfig') }}</el-button>
                <button class="custom-close-btn" @click="goToHome">
                  ×
                </button>
              </div>
            </div>
            <div class="divider"></div>

            <el-form ref="form" :model="form" label-width="72px" class="full-height-form">
              <!-- 助手昵称 -->
              <el-form-item :label="$t('templateQuickConfig.agentSettings.agentName')" prop="agentName" class="nickname-item">
                <el-input
                  v-model="form.agentName"
                  :placeholder="$t('templateQuickConfig.agentSettings.agentNamePlaceholder')"
                  :validate-event="false"
                  class="form-input"
                />
              </el-form-item>
              
              <!-- 角色介绍 -->
              <el-form-item :label="$t('templateQuickConfig.agentSettings.systemPrompt')" prop="systemPrompt" class="description-item">
                <el-input
                  v-model="form.systemPrompt"
                  type="textarea"
                  :placeholder="$t('templateQuickConfig.agentSettings.systemPromptPlaceholder')"
                  :validate-event="false"
                  show-word-limit
                  maxlength="2000"
                />
              </el-form-item>
            </el-form>
          </el-card>
        </div>
      </div>
    </div>
    <el-footer>
      <version-footer />
    </el-footer>
  </div>
</template>

<script>
import HeaderBar from "@/components/HeaderBar.vue";
import agentApi from '@/apis/module/agent';
import VersionFooter from "@/components/VersionFooter.vue";

// 默认模型配置常量
const DEFAULT_MODEL_CONFIG = {
  ttsModelId: "TTS_EdgeTTS",
  vadModelId: "VAD_SileroVAD",
  asrModelId: "ASR_FunASR",
  llmModelId: "LLM_ChatGLMLLM",
  vllmModelId: "VLLM_ChatGLMVLLM",
  memModelId: "Memory_nomem",
  intentModelId: "Intent_function_call"
};

export default {
  name: 'TemplateQuickConfig',
  components: { HeaderBar, VersionFooter },
  data() {
    return {
      form: {
        agentCode: "assistant",
        agentName: "",
        systemPrompt: "",
        sort: 0,
        model: { ...DEFAULT_MODEL_CONFIG }
      },
      templateId: '',
      originalForm: null
    };
  },
  methods: {
    // 返回模板管理页面
    goToHome() {
      this.$router.push('/agent-template-management');
    },
    
    // 保存配置
    saveConfig() {
      const configData = this.prepareConfigData();
      
      if (this.templateId) {
        this.updateExistingTemplate(configData);
      } else {
        this.createNewTemplate(configData);
      }
    },
    
    // 准备配置数据
    prepareConfigData() {
      return {
        id: this.templateId || '',
        agentCode: this.form.agentCode,
        agentName: this.form.agentName,
        systemPrompt: this.form.systemPrompt,
        sort: this.form.sort,
        functions: [],
        // 包含必要的模型字段以确保API调用成功
        ...this.form.model
      };
    },
    
    // 更新现有模板
    updateExistingTemplate(configData) {
      agentApi.updateAgentTemplate(configData, (res) => {
        if (res && res.data && res.data.code === 0) {
          this.$message.success({ 
            message: this.$t('templateQuickConfig.saveSuccess'), 
            showClose: true 
          });
          this.originalForm = JSON.parse(JSON.stringify(this.form));
        } else {
          this.$message.error({ 
            message: res?.data?.msg || this.$t('templateQuickConfig.saveFailed'), 
            showClose: true 
          });
        }
      });
    },
    
    // 创建新模板
    createNewTemplate(configData) {
      agentApi.addAgentTemplate(configData, (res) => {
        if (res && res.data && res.data.code === 0) {
          this.$message.success({ 
            message: this.$t('templateQuickConfig.saveSuccess'), 
            showClose: true 
          });
          this.goToHome();
        } else {
          this.$message.error({ 
            message: res?.data?.msg || this.$t('templateQuickConfig.saveFailed'), 
            showClose: true 
          });
        }
      });
    },
    
    // 重置配置
    resetConfig() {
      this.$confirm(
        this.$t('templateQuickConfig.confirmReset'), 
        this.$t('common.tip'), 
        {
          confirmButtonText: this.$t('common.confirm'),
          cancelButtonText: this.$t('common.cancel'),
          type: 'warning'
        }
      ).then(() => {
        if (this.originalForm) {
          this.form = JSON.parse(JSON.stringify(this.originalForm));
        }
        this.$message.success({ 
          message: this.$t('templateQuickConfig.resetSuccess'), 
          showClose: true 
        });
      }).catch(() => {});
    },
    
    // 根据ID获取模板
    fetchTemplateById(templateId) {
      agentApi.getAgentTemplateById(templateId, (res) => {
        if (res && res.data && res.data.code === 0 && res.data.data) {
          const template = res.data.data;
          this.applyTemplateData(template);
          this.templateId = templateId;
          this.originalForm = JSON.parse(JSON.stringify(this.form));
        } else {
          this.$message.error(res?.data?.msg || this.$t('templateQuickConfig.templateNotFound'));
        }
      });
    },
    
    // 应用模板数据
    applyTemplateData(templateData) {
      this.form = {
        ...this.form,
        agentName: templateData.agentName || this.form.agentName,
        agentCode: templateData.agentCode || this.form.agentCode,
        systemPrompt: templateData.systemPrompt || this.form.systemPrompt,
        sort: templateData.sort || this.form.sort,
        model: {
          ttsModelId: templateData.ttsModelId || this.form.model.ttsModelId,
          vadModelId: templateData.vadModelId || this.form.model.vadModelId,
          asrModelId: templateData.asrModelId || this.form.model.asrModelId,
          llmModelId: templateData.llmModelId || this.form.model.llmModelId,
          vllmModelId: templateData.vllmModelId || this.form.model.vllmModelId,
          memModelId: templateData.memModelId || this.form.model.memModelId,
          intentModelId: templateData.intentModelId || this.form.model.intentModelId
        }
      };
    },
    
    // 获取模板列表并设置排序号
    fetchTemplateListForSort() {
      agentApi.getAgentTemplate((res) => {
        if (res && res.data && res.data.code === 0) {
          const templateList = res.data.data || [];
          if (templateList.length > 0) {
            const maxSort = Math.max(...templateList.map(t => t.sort || 0));
            this.form.sort = maxSort + 1;
          } else {
            this.form.sort = 1;
          }
        } else {
          this.form.sort = 1;
        }
        
        this.originalForm = JSON.parse(JSON.stringify(this.form));
      });
    }
  },
  
  // 组件挂载时执行初始化
  mounted() {
    const templateId = this.$route.query.templateId;
    
    if (templateId) {
      // 编辑模式：加载现有模板
      this.fetchTemplateById(templateId);
    } else {
      // 新建模式：设置默认值并获取排序号
      this.form.agentName = this.$t('templateQuickConfig.newTemplate');
      this.fetchTemplateListForSort();
    }
  }
};
</script>

<style lang="scss" scoped>
@import '@/styles/tokens.scss';

.welcome {
  min-width: 900px;
  height: 100vh;
  display: flex;
  position: relative;
  flex-direction: column;
  background: var(--apple-bg);
  background-size: cover;
  background-position: center;
  -webkit-background-size: cover;
  -o-background-size: cover;
  overflow: hidden;
}

.operation-bar {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 16px 24px;
  border-bottom: 1px solid var(--apple-divider-soft);
}

.page-title {
  font-size: var(--apple-font-size-xl);
  font-weight: var(--apple-font-weight-bold);
  letter-spacing: -0.01em;
  color: var(--apple-text);
  margin: 0;
  line-height: var(--apple-line-height-tight);
}

.main-wrapper {
  height: calc(100vh - 63px - 35px - 60px);
  margin: 0 22px;
  border-radius: var(--apple-radius-lg);
  position: relative;
  display: flex;
  flex-direction: column;
  padding: 0 !important;
}

.content-panel {
  flex: 1;
  display: flex;
  overflow: hidden;
  height: 100%;
  border-radius: var(--apple-radius-lg);
  background: transparent;
  border: 1px solid var(--apple-divider-soft);
}

.content-area {
  flex: 1;
  height: 100%;
  overflow: auto;
  background-color: var(--apple-surface);
  display: flex;
  flex-direction: column;
}

.config-card {
  background: var(--apple-surface) !important;
  border-radius: var(--apple-radius-lg) !important;
  overflow: hidden !important;
  height: 100% !important;
  margin: 0 !important;
  border: none !important;
  box-shadow: var(--apple-shadow-card) !important;
}

.full-height-form {
  display: flex;
  flex-direction: column;
  padding: 20px;
  gap: 20px;
  height: calc(100% - 120px);
}

.nickname-item {
  margin-bottom: 0 !important;
}

.description-item {
  margin-bottom: 0 !important;
  display: flex;
  flex-direction: column;
}

::v-deep .description-item .el-textarea {
  height: 300px;
  min-height: 200px;
  max-height: 400px;
  display: flex;
  flex-direction: column;
}

::v-deep .description-item .el-textarea__inner {
  height: 100% !important;
  min-height: 200px !important;
  max-height: 400px !important;
  resize: vertical !important;
  line-height: var(--apple-line-height-normal);
  font-size: var(--apple-font-size-sm);
  padding: 12px;
  border: 1px solid var(--apple-divider);
  border-radius: var(--apple-radius-md);
  background-color: var(--apple-surface);
  color: var(--apple-text);
  font-family: inherit;
  transition:
    border-color var(--apple-duration-fast) var(--apple-ease-standard),
    box-shadow var(--apple-duration-fast) var(--apple-ease-standard);
}

::v-deep .description-item .el-textarea__inner:hover {
  border-color: var(--apple-divider);
  background-color: var(--apple-surface);
}

::v-deep .description-item .el-textarea__inner:focus {
  border-color: var(--apple-brand);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.12);
  outline: none;
}

::v-deep .el-form-item__label {
  font-size: var(--apple-font-size-sm) !important;
  color: var(--apple-text-2) !important;
  font-weight: var(--apple-font-weight-regular);
  line-height: 22px;
  padding-bottom: 2px;
}

::v-deep .el-textarea .el-input__count {
  color: var(--apple-text-3);
  background: rgba(255, 255, 255, 0.85);
  position: absolute;
  font-size: var(--apple-font-size-xs);
  right: 12px;
  bottom: 10px;
  padding: 2px 6px;
  border-radius: var(--apple-radius-sm);
}

.config-header {
  padding: 20px;
  display: flex;
  align-items: center;
  gap: 15px;
  background: var(--apple-surface-2);
  position: relative;
  border-bottom: 1px solid var(--apple-divider-soft);
}

.header-icon {
  width: 37px;
  height: 37px;
  background: var(--apple-brand-tint);
  border-radius: 50%;
  display: flex;
  align-items: center;
  justify-content: center;
  flex-shrink: 0;
}

.header-icon img {
  width: 19px;
  height: 19px;
}

.header-title {
  font-size: var(--apple-font-size-lg);
  font-weight: var(--apple-font-weight-semibold);
  color: var(--apple-text);
  letter-spacing: -0.01em;
}

.custom-close-btn {
  position: absolute;
  top: 25%;
  right: 0;
  transform: translateY(-50%);
  width: 35px;
  height: 35px;
  border-radius: 50%;
  border: 1px solid var(--apple-divider);
  background: var(--apple-surface);
  font-size: 24px;
  font-weight: var(--apple-font-weight-regular);
  color: var(--apple-text-3);
  cursor: pointer;
  display: flex;
  align-items: center;
  justify-content: center;
  z-index: 1;
  padding: 0;
  outline: none;
  transition:
    background-color var(--apple-duration-fast) var(--apple-ease-standard),
    border-color var(--apple-duration-fast) var(--apple-ease-standard),
    color var(--apple-duration-fast) var(--apple-ease-standard),
    transform var(--apple-duration-fast) var(--apple-ease-standard);

  &:hover {
    color: var(--apple-brand);
    border-color: var(--apple-brand);
    background: var(--apple-surface);
  }

  &:active {
    transform: translateY(-50%) scale(0.96);
  }
}

.header-actions {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-left: auto;
}

::v-deep .header-actions .save-btn.el-button {
  background: var(--apple-brand);
  border-color: var(--apple-brand);
  color: #ffffff;
  border-radius: var(--apple-radius-full);
  height: 36px;
  padding: 0 18px;
  font-size: var(--apple-font-size-sm);
  font-weight: var(--apple-font-weight-medium);
  transition:
    background-color var(--apple-duration-fast) var(--apple-ease-standard),
    border-color var(--apple-duration-fast) var(--apple-ease-standard),
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
    border-color: var(--apple-brand-active);
    color: #ffffff;
    transform: translateY(0);
  }
}

::v-deep .header-actions .reset-btn.el-button {
  background: var(--apple-surface);
  border-color: var(--apple-divider);
  color: var(--apple-text);
  border-radius: var(--apple-radius-full);
  height: 36px;
  padding: 0 18px;
  font-size: var(--apple-font-size-sm);
  font-weight: var(--apple-font-weight-medium);
  transition:
    background-color var(--apple-duration-fast) var(--apple-ease-standard),
    border-color var(--apple-duration-fast) var(--apple-ease-standard),
    transform var(--apple-duration-fast) var(--apple-ease-standard),
    box-shadow var(--apple-duration-fast) var(--apple-ease-standard);

  &:hover,
  &:focus {
    background: var(--apple-surface-2);
    border-color: var(--apple-divider);
    color: var(--apple-text);
    transform: translateY(-1px);
    box-shadow: var(--apple-shadow-card);
  }

  &:active {
    background: var(--apple-surface-2);
    transform: translateY(0);
  }
}

.header-actions .custom-close-btn {
  position: static;
  transform: none;
  width: 32px;
  height: 32px;
  margin-left: 8px;
}
</style>