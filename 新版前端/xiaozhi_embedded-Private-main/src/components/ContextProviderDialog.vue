<template>
  <el-dialog
    :visible.sync="dialogVisible"
    width="900px"
    :title="$t('contextProviderDialog.title')"
    :close-on-click-modal="false"
    custom-class="context-provider-dialog"
    append-to-body
  >
    <div class="dialog-content">
      <el-empty v-if="localProviders.length === 0" :description="$t('contextProviderDialog.noContextApi')">
        <el-button type="primary" icon="el-icon-plus" @click="addProvider(0)">{{ $t('contextProviderDialog.add') }}</el-button>
      </el-empty>

      <div
        v-for="(provider, pIndex) in localProviders"
        :key="pIndex"
        class="provider-item"
      >
        <el-card class="provider-card" shadow="hover" :body-style="{ padding: '15px 20px' }">
          <!-- URL Row -->
          <div class="input-row">
            <span class="label-text">{{ $t('contextProviderDialog.apiUrl') }}</span>
            <el-input
              v-model="provider.url"
              :placeholder="$t('contextProviderDialog.apiUrlPlaceholder')"
              size="small"
              class="flex-1"
            ></el-input>
          </div>

          <!-- Headers Section -->
          <div class="headers-section">
            <div class="label-text" style="margin-top: 6px;">{{ $t('contextProviderDialog.requestHeaders') }}</div>
            <div class="headers-list">
              <div
                v-for="(header, hIndex) in provider.headers"
                :key="hIndex"
                class="header-row"
              >
                <el-input
                  v-model="header.key"
                  :placeholder="$t('contextProviderDialog.headerKeyPlaceholder')"
                  size="small"
                  style="width: 180px;"
                ></el-input>
                <span class="separator">:</span>
                <el-input
                  v-model="header.value"
                  :placeholder="$t('contextProviderDialog.headerValuePlaceholder')"
                  size="small"
                  class="flex-1"
                ></el-input>
                
                <div class="row-controls">
                  <el-button
                    type="primary"
                    icon="el-icon-plus"
                    circle
                    size="mini"
                    plain
                    @click="addHeader(pIndex, hIndex + 1)"
                  ></el-button>
                  <el-button
                    type="danger"
                    icon="el-icon-minus"
                    circle
                    size="mini"
                    plain
                    @click="removeHeader(pIndex, hIndex)"
                  ></el-button>
                </div>
              </div>
              <!-- Empty Headers State -->
              <div v-if="provider.headers.length === 0" class="header-row empty-header">
                 <span class="no-header-text">{{ $t('contextProviderDialog.noHeaders') }}</span>
                 <el-button
                    type="text"
                    icon="el-icon-plus"
                    size="mini"
                    @click="addHeader(pIndex, 0)"
                  >{{ $t('contextProviderDialog.addHeader') }}</el-button>
              </div>
            </div>
          </div>
        </el-card>

        <!-- Provider Block Controls (Right Side) -->
        <div class="block-controls">
          <el-button
            type="primary"
            icon="el-icon-plus"
            circle
            size="medium"
            @click="addProvider(pIndex + 1)"
          ></el-button>
          <el-button
            type="danger"
            icon="el-icon-minus"
            circle
            size="medium"
            @click="removeProvider(pIndex)"
          ></el-button>
        </div>
      </div>
    </div>

    <span slot="footer" class="dialog-footer">
      <el-button @click="dialogVisible = false">{{ $t('contextProviderDialog.cancel') }}</el-button>
      <el-button type="primary" @click="handleConfirm">{{ $t('contextProviderDialog.confirm') }}</el-button>
    </span>
  </el-dialog>
</template>

<script>
export default {
  name: 'ContextProviderDialog',
  props: {
    visible: {
      type: Boolean,
      default: false
    },
    providers: {
      type: Array,
      default: () => []
    }
  },
  data() {
    return {
      localProviders: []
    };
  },
  computed: {
    dialogVisible: {
      get() {
        return this.visible;
      },
      set(val) {
        this.$emit('update:visible', val);
      }
    }
  },
  watch: {
    visible(val) {
      if (val) {
        this.initLocalData();
      }
    }
  },
  methods: {
    initLocalData() {
      // 深拷贝并将 headers 对象转换为数组
      this.localProviders = this.providers.map(p => {
        const headers = p.headers || {};
        return {
          url: p.url || '',
          headers: Object.entries(headers).map(([key, value]) => ({ key, value }))
        };
      });
      
      // 如果为空，添加一个默认块
      if (this.localProviders.length === 0) {
         this.localProviders.push({ url: '', headers: [{ key: '', value: '' }] });
      }
    },
    addProvider(index) {
      this.localProviders.splice(index, 0, {
        url: '',
        headers: [{ key: '', value: '' }]
      });
    },
    removeProvider(index) {
      this.localProviders.splice(index, 1);
    },
    addHeader(pIndex, hIndex) {
      this.localProviders[pIndex].headers.splice(hIndex, 0, { key: '', value: '' });
    },
    removeHeader(pIndex, hIndex) {
      this.localProviders[pIndex].headers.splice(hIndex, 1);
    },
    handleConfirm() {
      const result = this.localProviders
        .filter(p => p.url.trim() !== '')
        .map(p => {
          const headersObj = {};
          p.headers.forEach(h => {
            if (h.key.trim()) {
              headersObj[h.key.trim()] = h.value;
            }
          });
          return {
            url: p.url.trim(),
            headers: headersObj
          };
        });
      
      this.$emit('confirm', result);
      this.dialogVisible = false;
    }
  }
};
</script>

<style scoped>
.dialog-content {
  max-height: 60vh;
  overflow-y: auto;
  padding: 20px 25px;
}

.dialog-content::-webkit-scrollbar {
  width: 6px;
}
.dialog-content::-webkit-scrollbar-thumb {
  background: var(--apple-divider);
  border-radius: 3px;
}
.dialog-content::-webkit-scrollbar-track {
  background: var(--apple-surface-2);
}

.provider-item {
  display: flex;
  gap: 15px;
  margin-bottom: 20px;
  align-items: center;
}

.provider-card {
  flex: 1;
  border-radius: var(--apple-radius-md);
  border: 1px solid var(--apple-divider-soft);
  border-left: 4px solid var(--apple-brand);
  background-color: var(--apple-surface);
  transition: all var(--apple-duration-normal) var(--apple-ease-standard);
  box-shadow: var(--apple-shadow-card);
}

.provider-card:hover {
  box-shadow: var(--apple-shadow-raised);
  transform: translateY(-2px);
}

.block-controls {
  display: flex;
  flex-direction: row;
  gap: 8px;
}

.input-row {
  display: flex;
  align-items: center;
  gap: 12px;
  margin-bottom: 18px;
}

.label-text {
  width: 60px;
  font-weight: var(--apple-font-weight-semibold);
  color: var(--apple-text);
  text-align: right;
  font-size: 13px;
  white-space: nowrap;
  line-height: 32px;
}

.flex-1 {
  flex: 1;
}

.headers-section {
  display: flex;
  gap: 12px;
  align-items: flex-start;
}

.headers-list {
  flex: 1;
  display: flex;
  flex-direction: column;
  gap: 10px;
  background: var(--apple-surface-2);
  padding: 15px;
  border-radius: var(--apple-radius-md);
  border: 1px dashed var(--apple-divider);
  transition: all var(--apple-duration-fast) var(--apple-ease-standard);
}

.headers-list:hover {
  border-color: var(--apple-brand);
  background: var(--apple-surface);
}

.header-row {
  display: flex;
  align-items: center;
  gap: 10px;
}

.separator {
  color: var(--apple-text-2);
  font-weight: var(--apple-font-weight-semibold);
  margin: 0 2px;
}

.row-controls {
  display: flex;
  gap: 6px;
  margin-left: 8px;
  flex-shrink: 0;
  opacity: 0.6;
  transition: opacity var(--apple-duration-fast) var(--apple-ease-standard);
}

.header-row:hover .row-controls {
  opacity: 1;
}

.empty-header {
  justify-content: center;
  padding: 10px;
  color: var(--apple-text-3);
  font-size: 13px;
}

.no-header-text {
  margin-right: 8px;
}

::v-deep .el-input__inner {
  background-color: var(--apple-surface-2);
  border-radius: var(--apple-radius-sm);
  border: 1px solid transparent;
  color: var(--apple-text);
  transition: all var(--apple-duration-fast) var(--apple-ease-standard);
}
::v-deep .el-input__inner:focus {
  border-color: var(--apple-brand);
  background-color: var(--apple-surface);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.24);
}
::v-deep .el-input__inner::placeholder {
  color: var(--apple-text-3);
}

::v-deep .el-button.is-circle {
  background-color: var(--apple-surface);
  border: 1px solid var(--apple-divider);
  color: var(--apple-text-2);
  transition: all var(--apple-duration-fast) var(--apple-ease-standard);
}
::v-deep .el-button.is-circle:hover {
  background-color: var(--apple-surface-2);
  transform: translateY(-1px);
}
::v-deep .el-button.is-circle.is-plain.el-button--primary {
  background-color: var(--apple-brand-tint);
  border-color: var(--apple-brand);
  color: var(--apple-brand);
}
::v-deep .el-button.is-circle.is-plain.el-button--primary:hover {
  background-color: var(--apple-brand);
  color: var(--apple-surface);
}
::v-deep .el-button.is-circle.el-button--danger {
  background-color: rgba(255, 59, 48, 0.10);
  border-color: rgba(255, 59, 48, 0.30);
  color: var(--apple-danger);
}
::v-deep .el-button.is-circle.el-button--danger:hover {
  background-color: var(--apple-danger);
  color: var(--apple-surface);
  border-color: var(--apple-danger);
}

::v-deep .el-button--text {
  color: var(--apple-brand);
  transition: color var(--apple-duration-fast) var(--apple-ease-standard);
}
::v-deep .el-button--text:hover {
  color: var(--apple-brand-hover);
}
</style>
