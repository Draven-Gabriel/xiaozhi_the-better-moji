<template>
  <div class="welcome">
    <HeaderBar />
    <div class="main-wrapper">
      <div class="content-panel">
        <div class="content-area">
          <el-card class="provider-card" shadow="never">
            <div class="operation-header">
              <h2 class="page-title">{{ $t('header.providerManagement') }}</h2>
              <div class="right-operations">
                <el-input :placeholder="$t('providerManagement.searchPlaceholder')" v-model="searchName" class="search-input"
                  @keyup.enter.native="handleSearch" clearable />
                  <el-dropdown trigger="click" @command="handleSelectModelType" @visible-change="handleDropdownVisibleChange">
                    <CustomButton>
                      {{ $t('providerManagement.categoryFilter') }}{{ selectedModelTypeLabel }}<i class="el-icon-arrow-down el-icon--right"
                        :class="{ 'rotate-down': DropdownVisible }"></i>
                    </CustomButton>
                    <el-dropdown-menu slot="dropdown">
                      <el-dropdown-item command="">{{ $t('common.all') }}</el-dropdown-item>
                      <el-dropdown-item v-for="item in translatedModelTypes" :key="item.value" :command="item.value">
                        {{ item.label }}
                      </el-dropdown-item>
                    </el-dropdown-menu>
                  </el-dropdown>
                <CustomButton icon="el-icon-search" type="confirm" @click="handleSearch">{{ $t('common.search') }}</CustomButton>
              </div>
            </div>
            <CustomTable
              ref="providersTable"
              :data="filteredProvidersList"
              :columns="tableColumns"
              :loading="loading"
              :show-selection="true"
              :show-operations="true"
              :operations-label="$t('common.action')"
              :total="total"
              :current-page="currentPage"
              :page-size="pageSize"
              :page-size-options="pageSizeOptions"
              @size-change="handlePageSizeChange"
              @page-change="goToPage"
            >
              <template slot="selection" slot-scope="scope">
                <el-checkbox v-model="scope.row.selected"></el-checkbox>
              </template>
              <template slot="modelType" slot-scope="scope">
                <UiTag :type="getModelTypeTag(scope.row.modelType)">
                  {{ getModelTypeLabel(scope.row.modelType) }}
                </UiTag>
              </template>
              <template slot="fields" slot-scope="scope">
                <el-popover placement="top-start" width="400" trigger="hover">
                  <div v-for="field in scope.row.fields" :key="field.key" class="field-item">
                    <span class="field-label">{{ field.label }}:</span>
                    <span class="field-type">{{ field.type }}</span>
                    <span v-if="isSensitiveField(field.key)" class="sensitive-tag">{{ $t('common.sensitive') }}</span>
                  </div>
                  <UiButton slot="reference" type="text" size="small" class="view-btn">{{ $t('providerManagement.viewFields') }}</UiButton>
                </el-popover>
              </template>
              <template slot="operations" slot-scope="scope">
                <UiButton type="text" size="small" class="edit-btn" @click="editProvider(scope.row)">{{ $t('common.edit') }}</UiButton>
                <UiButton type="text" size="small" class="delete-btn" @click="deleteProvider(scope.row)">{{ $t('common.delete') }}</UiButton>
              </template>
              <template slot="footer-btns">
                <div class="ctrl_btn">
                  <CustomButton :icon="isAllSelected ? 'el-icon-circle-close' : 'el-icon-circle-check'" size="small" @click="handleSelectAll">
                    {{ isAllSelected ? $t('common.deselectAll') : $t('common.selectAll') }}
                  </CustomButton>
                  <CustomButton type="add" icon="el-icon-plus" size="small" @click="showAddDialog">
                    {{ $t('common.add') }}
                  </CustomButton>
                  <CustomButton size="small" type="delete" icon="el-icon-delete" @click="deleteSelectedProviders">
                    {{ $t('common.delete') }}
                  </CustomButton>
                </div>
              </template>
            </CustomTable>
          </el-card>
        </div>
      </div>
    </div>

    <provider-dialog :title="dialogTitle" :visible.sync="dialogVisible" :form="providerForm" :model-types="modelTypes"
      @submit="handleSubmit" @cancel="dialogVisible = false" />

    <el-footer>
      <version-footer />
    </el-footer>
  </div>
</template>

<script>
import Api from "@/apis/api";
import HeaderBar from "@/components/HeaderBar.vue";
import ProviderDialog from "@/components/ProviderDialog.vue";
import VersionFooter from "@/components/VersionFooter.vue";
import CustomButton from "@/components/CustomButton.vue";
import CustomTable from "@/components/CustomTable.vue";
import UiButton from "@/components/ui/UiButton.vue";
import UiTag from "@/components/ui/UiTag.vue";

export default {
  components: { HeaderBar, ProviderDialog, VersionFooter, CustomButton, CustomTable, UiButton, UiTag },
  data() {
    return {
      searchName: "",
      searchModelType: "",
      providersList: [],
      modelTypes: [
        { value: "ASR", labelKey: 'providerManagement.modelType.ASR' },
        { value: "TTS", labelKey: 'providerManagement.modelType.TTS' },
        { value: "LLM", labelKey: 'providerManagement.modelType.LLM' },
        { value: "VLLM", labelKey: 'providerManagement.modelType.VLLM' },
        { value: "Intent", labelKey: 'providerManagement.modelType.Intent' },
        { value: "Memory", labelKey: 'providerManagement.modelType.Memory' },
        { value: "VAD", labelKey: 'providerManagement.modelType.VAD' },
        { value: "Plugin", labelKey: 'providerManagement.modelType.Plugin' },
        { value: "RAG", labelKey: 'providerManagement.modelType.RAG' }
      ],
      currentPage: 1,
      loading: false,
      pageSize: 10,
      pageSizeOptions: [10, 20, 50, 100],
      total: 0,
      dialogVisible: false,
      dialogTitle: "新增供应器",
      isAllSelected: false,
      DropdownVisible: false,
      sensitive_keys: ["api_key", "personal_access_token", "access_token", "token", "secret", "access_key_secret", "secret_key"],
      providerForm: {
        id: null,
        modelType: "",
        providerCode: "",
        name: "",
        fields: [],
        sort: 0
      },
      tableColumns: []
    };
  },
  created() {
    this.initTableColumns();
    this.fetchProviders();
  },
  computed: {
    translatedModelTypes() {
      return this.modelTypes.map(type => ({
        value: type.value,
        label: this.$t(type.labelKey)
      }));
    },
    selectedModelTypeLabel() {
      if (!this.searchModelType) return `（${this.$t('providerManagement.all')}）`;
      const selectedType = this.modelTypes.find(item => item.value === this.searchModelType);
      return selectedType ? `（${this.$t(selectedType.labelKey)}）` : "";
    },
    filteredProvidersList() {
      return this.providersList;
    }
  },
  methods: {
    initTableColumns() {
      this.tableColumns = [
        {
          prop: 'modelType',
          label: this.$t('providerManagement.category'),
          align: 'center',
          width: 200
        },
        {
          prop: 'providerCode',
          label: this.$t('providerManagement.providerCode'),
          align: 'center',
          width: 150
        },
        {
          prop: 'name',
          label: this.$t('common.name'),
          align: 'center'
        },
        {
          prop: 'fields',
          label: this.$t('providerManagement.fieldConfig'),
          align: 'center'
        },
        {
          prop: 'sort',
          label: this.$t('common.sort'),
          align: 'center',
          width: 80
        }
      ];
    },
    fetchProviders() {
      this.loading = true;

      Api.model.getModelProvidersPage(
        {
          page: this.currentPage,
          limit: this.pageSize,
          name: this.searchName,
          modelType: this.searchModelType
        },
        ({ data }) => {
          this.loading = false;
          if (data.code === 0) {
            this.providersList = data.data.list.map(item => {
              return {
                ...item,
                selected: false,
                fields: JSON.parse(item.fields)
              };
            });
            this.total = data.data.total;
          } else {
            this.$message.error({
              message: data.msg || '获取参数列表失败'
            });
          }
        }
      );
    },
    handleSearch() {
      this.currentPage = 1;
      this.fetchProviders();
    },
    handleSelectModelType(value) {
      this.searchModelType = value;
      this.handleSearch();
    },
    handleSelectAll() {
      this.isAllSelected = !this.isAllSelected;
      this.providersList.forEach(row => {
        row.selected = this.isAllSelected;
      });
    },
    showAddDialog() {
      this.dialogTitle = this.$t('common.addProvider');
      this.providerForm = {
        id: null,
        modelType: "",
        providerCode: "",
        name: "",
        fields: [],
        sort: 0
      };
      this.dialogVisible = true;
    },
    editProvider(row) {
      this.dialogTitle = this.$t('common.editProvider');
      this.providerForm = {
        ...row,
        fields: JSON.parse(JSON.stringify(row.fields))
      };
      this.dialogVisible = true;
    },
    handleSubmit({ form, done }) {
      this.loading = true;
      if (form.id) {
        Api.model.updateModelProvider(form, ({ data }) => {
          if (data.code === 0) {
            this.fetchProviders();
            this.$message.success({
              message: this.$t('common.updateSuccess'),
              showClose: true
            });
          }
        });
      } else {
        Api.model.addModelProvider(form, ({ data }) => {
          if (data.code === 0) {
            this.fetchProviders();
            this.$message.success({
              message: this.$t('common.addSuccess'),
              showClose: true
            });
            this.total += 1;
          }
        });
      }
      this.loading = false;
      this.dialogVisible = false;
      done && done();
    },
    deleteSelectedProviders() {
      const selectedRows = this.providersList.filter(row => row.selected);
      if (selectedRows.length === 0) {
        this.$message.warning({
          message: this.$t('providerManagement.selectToDelete'),
          showClose: true
        });
        return;
      }
      this.deleteProvider(selectedRows);
    },
    deleteProvider(row) {
      const providers = Array.isArray(row) ? row : [row];
      const providerCount = providers.length;

      this.$confirm(this.$t('providerManagement.confirmDelete', { count: providerCount }), this.$t('common.warning'), {
        confirmButtonText: this.$t('common.confirm'),
        cancelButtonText: this.$t('common.cancel'),
        type: 'warning',
      }).then(() => {
        const ids = providers.map(provider => provider.id);
        Api.model.deleteModelProviderByIds(ids, ({ data }) => {
          if (data.code === 0) {
            this.isAllSelected = false;
            this.fetchProviders();
            this.$message.success({
              message: this.$t('common.deleteSuccess'),
              showClose: true
            });
          } else {
            this.$message.error({
              message: data.msg || this.$t('common.deleteFailure'),
              showClose: true
            });
          }
        });
      }).catch(() => {
        this.$message({
          type: 'info',
          message: this.$t('common.deleteCancelled'),
          showClose: true,
          duration: 1000
        });
      });
    },
    getModelTypeTag(type) {
      const typeMap = {
        'ASR': 'success',
        'TTS': 'warning',
        'LLM': 'danger',
        'Intent': 'primary',
        'Memory': 'default',
        'VAD': 'primary',
        'RAG': 'warning',
        'VLLM': 'default',
        'Plugin': 'default'
      };
      return typeMap[type] || 'default';
    },
    getModelTypeLabel(type) {
      const typeItem = this.modelTypes.find(item => item.value === type);
      return typeItem ? this.$t(typeItem.labelKey) : type;
    },
    isSensitiveField(fieldKey) {
      if (typeof fieldKey !== 'string') return false;
      return this.sensitive_keys.some(key =>
        fieldKey.toLowerCase().includes(key.toLowerCase())
      );
    },
    handlePageSizeChange(val) {
      this.pageSize = val;
      this.currentPage = 1;
      this.fetchProviders();
    },
    goToPage(page) {
      if (page !== this.currentPage) {
        this.currentPage = page;
        this.fetchProviders();
      }
    },
    handleDropdownVisibleChange(visible) {
      this.DropdownVisible = visible;
    }
  }
};
</script>

<style lang="scss" scoped>
@import "@/styles/tokens.scss";

.welcome {
  min-width: 900px;
  min-height: 506px;
  height: 100vh;
  display: flex;
  position: relative;
  flex-direction: column;
  background: var(--apple-bg);
  font-family: $apple-font-family;
  overflow: hidden;
}

.main-wrapper {
  height: calc(100vh - 63px - 35px);
  padding: 20px 22px 0;
  position: relative;
  display: flex;
  flex-direction: column;
  box-sizing: border-box;
}

.operation-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 0 0 16px 0;
}

.page-title {
  margin: 0;
  font-size: var(--apple-font-size-xl);
  font-weight: var(--apple-font-weight-bold);
  color: var(--apple-text);
  letter-spacing: -0.01em;
}

.right-operations {
  display: flex;
  gap: 10px;
  margin-left: auto;
  align-items: center;
}

.search-input {
  width: 260px;
}

.content-panel {
  display: flex;
  overflow: hidden;
  height: 100%;
  border-radius: var(--apple-radius-lg);
  background: transparent;
}

.content-area {
  flex: 1;
  height: 100%;
  min-width: 600px;
  overflow: auto;
  background-color: var(--apple-surface);
  display: flex;
  flex-direction: column;
  border-radius: var(--apple-radius-lg);
  box-shadow: var(--apple-shadow-card);
}

.provider-card {
  background: var(--apple-surface);
  flex: 1;
  display: flex;
  flex-direction: column;
  border: none;
  box-shadow: none;
  overflow: hidden;

  ::v-deep .el-card__body {
    padding: 18px 20px;
    display: flex;
    flex-direction: column;
    flex: 1;
    overflow: hidden;
  }
}

.ctrl_btn {
  display: flex;
  gap: 10px;
  align-items: center;
}

.field-item {
  padding: 6px 0;
  border-bottom: 1px solid var(--apple-divider-soft);
  display: flex;
  align-items: center;
  font-size: var(--apple-font-size-sm);

  .field-label {
    flex: 1;
    font-weight: var(--apple-font-weight-semibold);
    color: var(--apple-text);
  }

  .field-type {
    width: 80px;
    color: var(--apple-text-2);
    font-size: var(--apple-font-size-xs);
    padding: 2px 8px;
    border-radius: var(--apple-radius-full);
    background: var(--apple-surface-2);
    text-align: center;
  }

  .sensitive-tag {
    margin-left: 10px;
    padding: 2px 10px;
    border-radius: var(--apple-radius-full);
    background: rgba(255, 59, 48, 0.12);
    color: #b3261e;
    font-size: var(--apple-font-size-xs);
    font-weight: var(--apple-font-weight-medium);
  }
}

.rotate-down {
  transform: rotate(180deg);
  transition: transform var(--apple-duration-fast) var(--apple-ease-standard);
}

.el-icon-arrow-down {
  transition: transform var(--apple-duration-fast) var(--apple-ease-standard);
}

::v-deep .search-input .el-input__inner {
  background: var(--apple-surface-2);
  border: 1px solid transparent;
  border-radius: var(--apple-radius-md);
  height: 38px;
  line-height: 38px;
  font-size: var(--apple-font-size-base);
  color: var(--apple-text);
  transition:
    background-color var(--apple-duration-fast) var(--apple-ease-standard),
    border-color var(--apple-duration-fast) var(--apple-ease-standard),
    box-shadow var(--apple-duration-fast) var(--apple-ease-standard);
}

::v-deep .search-input .el-input__inner::placeholder {
  color: var(--apple-text-3);
}

::v-deep .search-input:hover .el-input__inner {
  background: var(--apple-surface);
  border-color: var(--apple-divider);
}

::v-deep .search-input.is-focus .el-input__inner,
::v-deep .search-input .el-input.is-focus .el-input__inner {
  background: var(--apple-surface);
  border-color: var(--apple-brand);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.16);
}

::v-deep .edit-btn.ui-btn--text,
::v-deep .view-btn.ui-btn--text,
::v-deep .delete-btn.ui-btn--text {
  padding: 4px 10px;
  font-size: var(--apple-font-size-xs);
  line-height: 1;
  border-radius: var(--apple-radius-sm);
  height: auto;
  min-height: 0;
  margin: 0 4px;
  font-weight: var(--apple-font-weight-medium);
}

::v-deep .edit-btn.ui-btn--text:hover,
::v-deep .view-btn.ui-btn--text:hover {
  background: var(--apple-brand-tint) !important;
  color: var(--apple-brand-hover) !important;
}

::v-deep .delete-btn.ui-btn--text:hover {
  background: rgba(255, 59, 48, 0.10) !important;
  color: var(--apple-danger) !important;
}
</style>
