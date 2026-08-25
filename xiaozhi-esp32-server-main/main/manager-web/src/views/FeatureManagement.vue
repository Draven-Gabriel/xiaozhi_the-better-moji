<template>
  <div class="welcome">
    <HeaderBar />
    <div class="main-wrapper">
      <div class="content-panel">
        <div class="content-area">
          <el-card class="feature-card" shadow="never">
            <div class="config-header">
              <h2 class="page-title">{{ $t('header.featureManagement') }}</h2>
              <div class="header-actions">
                <CustomButton :icon="isAllSelected ? 'el-icon-circle-close' : 'el-icon-circle-check'" size="medium" @click="!isSaving && toggleSelectAll()" :disabled="isSaving">
                  {{ isAllSelected ? $t('featureManagement.deselectAll') : $t('featureManagement.selectAll') }}
                </CustomButton>
                <CustomButton icon="el-icon-refresh" size="medium" @click="handleReset" :disabled="isSaving">
                  {{ $t('featureManagement.reset') }}
                </CustomButton>
                <CustomButton icon="el-icon-setting" size="medium" type="confirm" @click="handleSave" :disabled="isSaving">
                  {{ isSaving ? $t('featureManagement.saving') : $t('featureManagement.save') }}
                </CustomButton>
              </div>
            </div>
            <div class="divider"></div>

            <!-- 功能分组容器 - 左右布局 -->
            <div class="feature-groups-container">
              <!-- 功能管理分组 -->
              <div v-if="featureManagementFeatures.length > 0" class="feature-group">
                <div class="group-title">
                  <img src="@/assets/setting/menu.png" alt="" width="28" height="28">
                  <div class="group-header">
                    <span class="group-name">{{ $t('featureManagement.groupName.featureManagement') }}</span>
                    <span class="module-count">{{ $t('featureManagement.moduleCount', { count: featureManagementFeatures.length }) }}</span>
                  </div>
                </div>
                <p class="group-description">{{ $t('featureManagement.groupDescription.featureManagement') }}</p>
                <div class="features-grid">
                  <div
                    v-for="feature in featureManagementFeatures"
                    :key="feature.id"
                    class="feature-card-item"
                    :class="{ 'feature-enabled': feature.enabled, 'feature-disabled': isSaving }"
                    @click="!isSaving && toggleFeature(feature)"
                  >
                    <img :src="featureIcons[feature.id]" alt="" width="60" height="60">
                    <div class="feature-content">
                      <div class="feature-header">
                        <h3 class="feature-name">{{ $t(`feature.${feature.id}.name`) }}</h3>
                        <div class="feature-checkbox-container">
                          <el-checkbox
                            v-model="feature.enabled"
                            @change="!isSaving && toggleFeature(feature)"
                            class="feature-checkbox"
                            :disabled="isSaving"
                          />
                        </div>
                      </div>
                      <p class="feature-description">{{ $t(`feature.${feature.id}.description`) }}</p>
                    </div>
                  </div>
                </div>
              </div>

              <!-- 语音管理分组 -->
              <div v-if="voiceManagementFeatures.length > 0" class="feature-group">
                <div class="group-title">
                  <img src="@/assets/setting/agent.png" alt="" width="28" height="28">
                  <div class="group-header">
                    <span class="group-name">{{ $t('featureManagement.groupName.voiceManagement') }}</span>
                    <span class="module-count">{{ $t('featureManagement.moduleCount', { count: voiceManagementFeatures.length }) }}</span>
                  </div>
                </div>
                <p class="group-description">{{ $t('featureManagement.groupDescription.voiceManagement') }}</p>
                <div class="features-grid">
                  <div
                    v-for="feature in voiceManagementFeatures"
                    :key="feature.id"
                    class="feature-card-item"
                    :class="{ 'feature-enabled': feature.enabled, 'feature-disabled': isSaving }"
                    @click="!isSaving && toggleFeature(feature)"
                  >
                    <img :src="voiceManagementIcons[feature.id]" alt="" width="60" height="60">
                    <div class="feature-content">
                      <div class="feature-header">
                        <h3 class="feature-name">{{ $t(`feature.${feature.id}.name`) }}</h3>
                        <div class="feature-checkbox-container">
                          <el-checkbox
                            v-model="feature.enabled"
                            @change="!isSaving && toggleFeature(feature)"
                            class="feature-checkbox"
                            :disabled="isSaving"
                          />
                        </div>
                      </div>
                      <p class="feature-description">{{ $t(`feature.${feature.id}.description`) }}</p>
                    </div>
                  </div>
                </div>
              </div>
            </div>
            
            <div v-if="filteredFeatures.length === 0" class="empty-state">
              <el-empty :description="$t('featureManagement.noFeatures')">
                <p class="empty-tip">{{ $t('featureManagement.contactAdmin') }}</p>
              </el-empty>
            </div>
          </el-card>
        </div>
      </div>
    </div>

    <el-footer>
      <VersionFooter />
    </el-footer>
  </div>
</template>

<script>
import HeaderBar from "@/components/HeaderBar.vue";
import VersionFooter from "@/components/VersionFooter.vue";
import featureManager from "@/utils/featureManager.js";
import CustomButton from "@/components/CustomButton.vue";

import voiceprintIcon from '@/assets/setting/voiceprint.png'
import voiceCloneIcon from '@/assets/setting/voiceclone.png'
import knowledgeBaseIcon from '@/assets/setting/knowledgeBase.png'
import mcpAccessPointIcon from '@/assets/setting/mcpAccessPoint.png'
import addressBookIcon from '@/assets/setting/addressBook.png'
import vadIcon from '@/assets/setting/vad.png'
import asrIcon from '@/assets/setting/asr.png'

export default {
  name: "FeatureManagement",
  components: {
    HeaderBar,
    VersionFooter,
    CustomButton
  },
  data() {
    return {
      pendingChanges: false,
      featureManagementFeatures: [],
      voiceManagementFeatures: [],
      isSaving: false, // 添加保存状态锁定
      // 功能模块图标映射
      featureIcons: {
        'voiceprintRecognition': voiceprintIcon,
        'voiceClone': voiceCloneIcon,
        'knowledgeBase': knowledgeBaseIcon,
        'mcpAccessPoint': mcpAccessPointIcon,
        'addressBook': addressBookIcon,
      },
      voiceManagementIcons: {
        'vad': vadIcon,
        'asr': asrIcon,
      },
    }
  },
  computed: {
    // 所有功能列表
    filteredFeatures() {
      return [...this.featureManagementFeatures, ...this.voiceManagementFeatures]
    },
    
    // 判断是否所有功能都已选中
    isAllSelected() {
      const allFeatures = [...this.featureManagementFeatures, ...this.voiceManagementFeatures]
      return allFeatures.length > 0 && allFeatures.every(feature => feature.enabled)
    }
  },
  async created() {
    // 等待功能配置管理器初始化完成
    try {
      await featureManager.waitForInitialization()
      await this.loadFeatures()
      this.setupConfigChangeListener()
    } catch (error) {
      console.error('功能配置管理器初始化等待失败:', error)
      await this.loadFeatures()
      this.setupConfigChangeListener()
    }
  },
  
  beforeDestroy() {
    this.removeConfigChangeListener()
  },
  
  methods: {
    // 根据ID列表获取功能
    async getFeaturesByIds(featureIds) {
      try {
        const featureConfig = await featureManager.getAllFeatures()
        const result = featureIds.map(id => {
          const feature = featureConfig[id]
          return {
            id: id,
            name: this.$t(`feature.${id}.name`),
            description: this.$t(`feature.${id}.description`),
            enabled: feature?.enabled || false
          }
        })
        
        return result
      } catch (error) {
        console.error('获取功能配置失败:', error)
        // 如果获取失败，返回默认配置
        return featureIds.map(id => ({
          id: id,
          name: this.$t(`feature.${id}.name`),
          description: this.$t(`feature.${id}.description`),
          enabled: false
        }))
      }
    },
    
    // 加载功能配置
    async loadFeatures() {
      // 保存当前用户的选择状态
      const currentFeatureStates = {}
      const allCurrentFeatures = [...this.featureManagementFeatures, ...this.voiceManagementFeatures]
      allCurrentFeatures.forEach(feature => {
        currentFeatureStates[feature.id] = feature.enabled
      })
      
      // 重新加载配置
      this.featureManagementFeatures = await this.getFeaturesByIds(['voiceprintRecognition', 'voiceClone', 'knowledgeBase', 'mcpAccessPoint', 'addressBook'])
      this.voiceManagementFeatures = await this.getFeaturesByIds(['vad', 'asr'])
      
      // 恢复用户的选择状态（如果存在）
      const allFeatures = [...this.featureManagementFeatures, ...this.voiceManagementFeatures]
      allFeatures.forEach(feature => {
        if (currentFeatureStates.hasOwnProperty(feature.id)) {
          feature.enabled = currentFeatureStates[feature.id]
        }
      })
    },
    // 切换功能状态
    async toggleFeature(feature) {
      // 如果正在保存，阻止操作
      if (this.isSaving) {
        return
      }
      
      feature.enabled = !feature.enabled
      this.pendingChanges = true
      
      // 不再立即更新到配置管理器，只在保存时统一更新
    },
    // 保存配置
    async handleSave() {
      if (!this.pendingChanges) {
        this.$message.info({
          message: this.$t('featureManagement.noChanges'),
          showClose: true
        })
        return
      }
      
      // 设置保存状态，锁定界面
      this.isSaving = true
      
      try {
        // 获取当前所有功能的状态并保存
        const featureUpdates = {}
        const allFeatures = [...this.featureManagementFeatures, ...this.voiceManagementFeatures]
        allFeatures.forEach(feature => {
          featureUpdates[feature.id] = feature.enabled
        })
        await featureManager.updateFeatures(featureUpdates)
        
        this.pendingChanges = false
        this.$message.success({
          message: this.$t('featureManagement.saveSuccess'),
          showClose: true
        })

        setTimeout(() => {
          this.loadFeatures()
        }, 1000)
      } catch (error) {
        console.error('保存配置失败:', error)
        this.$message.error({
          message: this.$t('featureManagement.saveError'),
          showClose: true
        })
      } finally {
        // 无论成功与否，都解除保存状态锁定
        this.isSaving = false
      }
    },
    // 设置配置变化监听器
    setupConfigChangeListener() {
      this.configChangeHandler = () => {
        this.loadFeatures()
      }
      window.addEventListener('featureConfigReloaded', this.configChangeHandler)
    },
    
    // 移除配置变化监听器
    removeConfigChangeListener() {
      if (this.configChangeHandler) {
        window.removeEventListener('featureConfigReloaded', this.configChangeHandler)
      }
    },
    
    // 重置配置
    async handleReset() {
      try {
        await this.$confirm(
          this.$t('featureManagement.resetConfirm'),
          this.$t('featureManagement.reset'),
          {
            confirmButtonText: this.$t('featureManagement.confirm'),
            cancelButtonText: this.$t('featureManagement.cancel'),
            type: 'warning'
          }
        )
        
        featureManager.resetToDefault()
        this.loadFeatures()
        this.pendingChanges = false
        
        this.$message.success({
          message: this.$t('featureManagement.resetSuccess'),
          showClose: true
        })
        
        setTimeout(() => {
          this.loadFeatures()
          this.$router.go(0)
        }, 1000)
      } catch (error) {
        // 用户取消操作
      }
    },
    // 搜索功能（预留接口）
    handleSearch() {
      // 搜索功能待实现
    },
    // 全选/取消全选
    toggleSelectAll() {
      // 如果正在保存，阻止操作
      if (this.isSaving) {
        return
      }
      
      const allFeatures = [...this.featureManagementFeatures, ...this.voiceManagementFeatures]
      const newStatus = !this.isAllSelected
      
      allFeatures.forEach(feature => {
        feature.enabled = newStatus
      })
      
      this.pendingChanges = true
    }
  }
}
</script>

<style lang="scss" scoped>
@import '@/styles/global.scss';
@import '@/styles/tokens.scss';

.welcome {
  min-width: 900px;
  min-height: 506px;
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
  padding: 22px 24px;
}

.page-title {
  font-weight: var(--apple-font-weight-bold);
  font-size: var(--apple-font-size-xl);
  color: var(--apple-text);
  letter-spacing: -0.01em;
  line-height: var(--apple-line-height-tight);
  margin: 0;
}

.config-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 0 16px 0;
}

.header-actions {
  display: flex;
  align-items: center;
  gap: 8px;
  margin-left: auto;
}

.divider {
  height: 1px;
  background: var(--apple-divider-soft);
  margin-bottom: 20px;
}

.main-wrapper {
  height: calc(100vh - 63px - 35px);
  margin: 20px 22px 0;
  border-radius: var(--apple-radius-lg);
  position: relative;
  display: flex;
  flex-direction: column;
  overflow: auto;
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
  min-width: 600px;
  overflow: auto;
  background-color: var(--apple-surface);
  display: flex;
  flex-direction: column;
}

.feature-card {
  background: var(--apple-surface);
  flex: 1;
  display: flex;
  flex-direction: column;
  border: none;
  box-shadow: var(--apple-shadow-card);
  overflow: hidden;
}

.feature-card ::v-deep .el-card__body {
  padding: 14px 20px;
  display: flex;
  flex-direction: column;
  flex: 1;
  overflow: auto;
  @include scrollbar-style;
}

.features-grid {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(300px, 1fr));
  gap: 16px;
}

.feature-card-item {
  display: flex;
  padding: 20px;
  border-radius: var(--apple-radius-lg);
  border: 1px solid var(--apple-divider-soft);
  background-color: var(--apple-surface);
  cursor: pointer;
  transition:
    border-color var(--apple-duration-normal) var(--apple-ease-standard),
    background-color var(--apple-duration-normal) var(--apple-ease-standard),
    box-shadow var(--apple-duration-normal) var(--apple-ease-standard),
    transform var(--apple-duration-normal) var(--apple-ease-standard);
  user-select: none;
  position: relative;
  align-items: center;
}

.feature-card-item:hover {
  border-color: var(--apple-brand);
  box-shadow: var(--apple-shadow-raised);
  transform: translateY(-2px);
}

.feature-card-item.feature-enabled {
  border-color: var(--apple-brand);
  background-color: var(--apple-brand-tint);
  box-shadow: 0 6px 18px rgba(0, 113, 227, 0.16);
  transform: translateY(-2px);
}

.feature-card-item.feature-disabled {
  opacity: 0.65;
  cursor: not-allowed;
  transform: none;
}

.feature-content {
  width: 100%;
  margin-left: 16px;
  min-width: 0;
}

.feature-header {
  display: flex;
  align-items: flex-start;
  justify-content: space-between;
  margin-bottom: 6px;
  gap: 12px;
}

.feature-checkbox-container {
  margin-top: 4px;
  display: flex;
  align-items: center;
  flex-shrink: 0;
}

.feature-checkbox ::v-deep .el-checkbox__inner {
  border-color: var(--apple-divider);
}

.feature-checkbox ::v-deep .el-checkbox__input.is-checked .el-checkbox__inner {
  background-color: var(--apple-brand);
  border-color: var(--apple-brand);
}

.feature-checkbox ::v-deep .el-checkbox__input.is-checked + .el-checkbox__label {
  color: var(--apple-brand);
}

.feature-name {
  white-space: normal;
  word-break: break-word;
  overflow-wrap: break-word;
  font-size: var(--apple-font-size-md);
  line-height: 1.4;
  font-weight: var(--apple-font-weight-semibold);
  color: var(--apple-text);
  margin: 0;
  transition: color var(--apple-duration-fast) var(--apple-ease-standard);
}

.feature-description {
  font-size: var(--apple-font-size-sm);
  line-height: var(--apple-line-height-normal);
  color: var(--apple-text-2);
  margin: 0;
  transition: color var(--apple-duration-fast) var(--apple-ease-standard);
  text-align: left;
}

.feature-groups-container {
  display: flex;
  gap: 32px;
  align-items: flex-start;
  position: relative;
}

.feature-groups-container::before {
  content: '';
  position: absolute;
  left: 50%;
  top: 0;
  bottom: 0;
  width: 1px;
  height: 550px;
  background: var(--apple-divider-soft);
  transform: translateX(-50%);
}

.feature-group {
  flex: 1;
  min-width: 0;
  margin-bottom: 32px;
}

.group-title {
  margin-top: 0;
  display: flex;
  align-items: center;
  font-size: var(--apple-font-size-md);
  line-height: var(--apple-line-height-tight);
  font-weight: var(--apple-font-weight-semibold);
  color: var(--apple-text);
  margin-bottom: 0;
  text-align: left;
  letter-spacing: -0.01em;
}

.group-name {
  margin-left: 10px;
}

.group-header {
  display: flex;
  align-items: center;
}

.module-count {
  margin-left: 13px;
  font-size: var(--apple-font-size-xs);
  background: var(--apple-brand-tint);
  color: var(--apple-brand);
  padding: 2px 10px;
  border-radius: var(--apple-radius-full);
  font-weight: var(--apple-font-weight-medium);
}

.group-description {
  margin-top: 6px;
  text-align: left;
  font-size: var(--apple-font-size-sm);
  font-weight: var(--apple-font-weight-regular);
  color: var(--apple-text-2);
}

@media (max-width: 900px) {
  .feature-groups-container {
    flex-direction: column;
    gap: 24px;
  }

  .feature-groups-container::before {
    display: none;
  }
}
</style>