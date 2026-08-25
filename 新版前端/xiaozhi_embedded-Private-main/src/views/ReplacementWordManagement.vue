<template>
  <div class="welcome">
    <HeaderBar />
    <div class="main-wrapper">
      <div class="content-panel">
        <div class="content-area">
          <el-card class="params-card" shadow="never">
            <div class="operation-header">
              <h2 class="page-title">{{ $t('replacementWordManagement.pageTitle') }}</h2>
              <div class="right-operations">
                <el-input
                  :placeholder="$t('replacementWordManagement.searchPlaceholder')"
                  v-model="searchKeyword"
                  class="search-input"
                  @keyup.enter.native="handleSearch"
                  clearable
                />
                <CustomButton icon="el-icon-search" type="confirm" size="small" @click="handleSearch">
                  {{ $t('replacementWordManagement.search') }}
                </CustomButton>
              </div>
            </div>
            <CustomTable
              ref="paramsTable"
              :data="filteredParamsList"
              :columns="tableColumns"
              :loading="loading"
              :show-selection="true"
              :show-operations="true"
              :operations-label="$t('replacementWordManagement.operation')"
              :total="total"
              :current-page="currentPage"
              :page-size="pageSize"
              :page-size-options="pageSizeOptions"
              @size-change="handlePageSizeChange"
              @page-change="goToPage"
            >
              <template slot="content" slot-scope="scope">
                <el-tooltip placement="right" effect="light" popper-class="replace-word-tooltip">
                  <div slot="content" class="replace-word-content">
                    <UiTag
                      v-for="(item, index) in scope.row.content"
                      :key="index"
                      type="primary"
                      size="small"
                      class="custom-tag"
                    >
                      {{ item }}
                    </UiTag>
                  </div>
                  <span class="content-text">{{ formatContent(scope.row.content) }}</span>
                </el-tooltip>
              </template>
              <template slot="operations" slot-scope="scope">
                <UiButton type="text" size="small" class="row-action-btn" @click="handleEdit(scope.row)">
                  {{ $t('replacementWordManagement.edit') }}
                </UiButton>
                <UiButton type="text" size="small" class="row-action-btn" @click="handleDownload(scope.row)">
                  {{ $t('replacementWordManagement.download') }}
                </UiButton>
                <UiButton type="text" size="small" class="row-action-btn delete-btn" @click="handleDelete(scope.row)">
                  {{ $t('replacementWordManagement.delete') }}
                </UiButton>
              </template>
              <template slot="footer-btns">
                <div class="ctrl_btn">
                  <CustomButton
                    :icon="allSelected ? 'el-icon-circle-close' : 'el-icon-circle-check'"
                    size="small"
                    @click="handleSelectAll"
                  >
                    {{ allSelected ? $t('user.deselectAll') : $t('user.selectAll') }}
                  </CustomButton>
                  <CustomButton type="add" icon="el-icon-plus" size="small" @click="handleAdd">
                    {{ $t('replacementWordManagement.addFile') }}
                  </CustomButton>
                  <CustomButton size="small" type="delete" icon="el-icon-delete" @click="handleBatchDelete">
                    {{ $t('replacementWordManagement.batchDelete') }}
                  </CustomButton>
                </div>
              </template>
            </CustomTable>
          </el-card>
        </div>
      </div>
    </div>

    <ReplacementWordDialog
      ref="paramDialog"
      :title="dialogTitle"
      :visible.sync="dialogVisible"
      :form="dialogForm"
      @submit="handleSubmit"
      @cancel="dialogVisible = false"
    />
    <el-footer><VersionFooter/></el-footer>
  </div>
</template>

<script>
import Api from "@/apis/api";
import HeaderBar from "@/components/HeaderBar.vue";
import VersionFooter from "@/components/VersionFooter.vue";
import ReplacementWordDialog from "@/components/ReplacementWordDialog.vue";
import CustomButton from "@/components/CustomButton.vue";
import CustomTable from "@/components/CustomTable.vue";
import UiButton from "@/components/ui/UiButton.vue";
import UiTag from "@/components/ui/UiTag.vue";

export default {
  components: { HeaderBar, VersionFooter, ReplacementWordDialog, CustomButton, CustomTable, UiButton, UiTag },
  data() {
    return {
      searchKeyword: "",
      paramsList: [],
      selectedRows: new Set(),
      currentPage: 1,
      loading: false,
      pageSize: 10,
      pageSizeOptions: [10, 20, 50, 100],
      total: 0,
      dialogVisible: false,
      dialogTitle: '',
      dialogForm: {},
      tableColumns: []
    };
  },
  computed: {
    filteredParamsList() {
      if (!this.searchKeyword) {
        return this.paramsList;
      }
      const keyword = this.searchKeyword.toLowerCase();
      return this.paramsList.filter(row =>
        (row.fileName || '').toLowerCase().includes(keyword)
      );
    },

    hasAnySelected() {
      return this.selectedRows.size > 0;
    },

    allSelected() {
      if (this.filteredParamsList.length === 0) {
        return false;
      }
      return this.filteredParamsList.every(row => this.selectedRows.has(row.id));
    }
  },
  created() {
    this.initTableColumns();
    this.fetchFileList();
  },
  mounted() {
    this.dialogTitle = this.$t('replacementWordManagement.addFile');
  },
  methods: {
    initTableColumns() {
      this.tableColumns = [
        {
          prop: 'fileName',
          label: this.$t('replacementWordManagement.fileName'),
          align: 'center'
        },
        {
          prop: 'wordCount',
          label: this.$t('replacementWordManagement.replacementWordCount'),
          align: 'center'
        },
        {
          prop: 'content',
          label: this.$t('replacementWordManagement.replacementWordContent'),
          align: 'center',
          slot: 'content'
        },
        {
          prop: 'createdAt',
          label: this.$t('replacementWordManagement.createTime'),
          align: 'center'
        },
        {
          prop: 'updatedAt',
          label: this.$t('replacementWordManagement.updateTime'),
          align: 'center'
        }
      ];
    },

    formatContent(content) {
      if (!content) return '';
      if (Array.isArray(content)) {
        return content.join(',');
      }
      return content;
    },

    handleSearch() {
      this.currentPage = 1;
    },

    handlePageSizeChange(val) {
      this.pageSize = val;
      this.currentPage = 1;
      this.fetchFileList();
    },

    goToPage(page) {
      if (page !== this.currentPage) {
        this.currentPage = page;
        this.fetchFileList();
      }
    },

    handleCheckboxChange(row) {
      const newSet = new Set(this.selectedRows);
      if (row.selected) {
        newSet.add(row.id);
      } else {
        newSet.delete(row.id);
      }
      this.selectedRows = newSet;
    },

    handleSelectAll() {
      if (this.allSelected) {
        this.filteredParamsList.forEach(row => {
          this.$set(row, 'selected', false);
        });
        this.selectedRows = new Set();
      } else {
        this.filteredParamsList.forEach(row => {
          this.$set(row, 'selected', true);
        });
        this.selectedRows = new Set(this.filteredParamsList.map(row => row.id));
      }
    },

    handleBatchDelete() {
      const ids = Array.from(this.filteredParamsList)
        .filter(row => row.selected)
        .map(row => row.id);

      if (ids.length === 0) {
        return;
      }

      this.$confirm(
        this.$t('replacementWordManagement.confirmBatchDelete', { count: ids.length }),
        this.$t('replacementWordManagement.batchDeleteHint'),
        {
          confirmButtonText: this.$t('common.confirm'),
          cancelButtonText: this.$t('common.cancel')
        }
      ).then(() => {
        Api.correctWord.batchDeleteFile(ids, ({ data }) => {
          if (data.code === 0) {
            this.$message.success(this.$t('common.deleteSuccess'));

            const newSet = new Set(this.selectedRows);
            ids.forEach(id => {
              newSet.delete(id);
            });
            this.selectedRows = newSet;

            this.fetchFileList();
          } else {
            this.$message.error(data.msg || this.$t('common.deleteFailure'));
          }
        });
      }).catch(() => {});
    },

    fetchFileList() {
      this.loading = true;
      Api.correctWord.getFileList({
        page: this.currentPage,
        pageSize: this.pageSize
      }, ({ data }) => {
        this.loading = false;
        if (data.code === 0) {
          this.paramsList = (data.data.list || []).map(row => ({
            ...row,
            selected: this.selectedRows.has(row.id)
          }));

          this.total = data.data.total || 0;
        } else {
          this.$message.error({
            message: data.msg || this.$t('replacementWordManagement.getListFailed'),
            showClose: true
          });
        }
      });
    },

    handleAdd() {
      this.dialogForm = {
        id: undefined,
        fileName: '',
        content: ''
      };
      this.dialogTitle = this.$t('replacementWordManagement.addFile');
      this.dialogVisible = true;
    },

    handleEdit(row) {
      this.dialogForm = {
        id: row.id,
        fileName: row.fileName,
        content: row.content || ''
      };
      this.dialogTitle = this.$t('replacementWordManagement.edit');
      this.dialogVisible = true;
    },

    handleDownload(row) {
      Api.correctWord.downloadFile(row.id, (res) => {
        const url = window.URL.createObjectURL(new Blob([res.data]));
        const link = document.createElement('a');
        link.href = url;
        link.download = `${row.fileName}.txt`;
        link.click();
        window.URL.revokeObjectURL(url);
      });
    },

    handleDelete(row) {
      this.$confirm(
        this.$t('replacementWordManagement.confirmDelete'),
        this.$t('replacementWordManagement.deleteHint'),
        {
          confirmButtonText: this.$t('common.confirm'),
          cancelButtonText: this.$t('common.cancel')
        }
      ).then(() => {
        Api.correctWord.deleteFile(row.id, ({ data }) => {
          if (data.code === 0) {
            this.$message.success(this.$t('common.deleteSuccess'));
            const newSet = new Set(this.selectedRows);
            newSet.delete(row.id);
            this.selectedRows = newSet;
            this.fetchFileList();
          } else {
            this.$message.error(data.msg || this.$t('common.deleteFailure'));
          }
        });
      }).catch(() => {});
    },

    handleSubmit(formData) {
      if (formData.id) {
        Api.correctWord.updateFile(formData, ({ data }) => {
          if (data.code === 0) {
            this.$message.success(this.$t('replacementWordManagement.saveSuccess'));
            this.dialogVisible = false;
            this.fetchFileList();
          } else {
            this.$message.error(data.msg || this.$t('replacementWordManagement.saveFailed'));
          }
          if (this.$refs.paramDialog) {
            this.$refs.paramDialog.resetSaving();
          }
        });
      } else {
        Api.correctWord.addFile(formData, ({ data }) => {
          if (data.code === 0) {
            this.$message.success(this.$t('common.addSuccess'));
            this.dialogVisible = false;
            this.fetchFileList();
          } else {
            this.$message.error(data.msg || this.$t('replacementWordManagement.addFailed'));
          }
          if (this.$refs.paramDialog) {
            this.$refs.paramDialog.resetSaving();
          }
        });
      }
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

.params-card {
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

::v-deep .row-action-btn.ui-btn--text {
  padding: 4px 10px;
  font-size: var(--apple-font-size-xs);
  line-height: 1;
  border-radius: var(--apple-radius-sm);
  height: auto;
  min-height: 0;
  margin: 0 4px;
  font-weight: var(--apple-font-weight-medium);
}

::v-deep .row-action-btn.ui-btn--text:hover {
  background: var(--apple-brand-tint) !important;
  color: var(--apple-brand-hover) !important;
}

::v-deep .row-action-btn.delete-btn.ui-btn--text:hover {
  background: rgba(255, 59, 48, 0.10) !important;
  color: var(--apple-danger) !important;
}

::v-deep .el-table .cell {
  .content-text {
    display: block;
    max-width: 300px;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }
}

.custom-tag.ui-tag {
  background: var(--apple-brand-tint);
  color: var(--apple-brand);
  border-radius: var(--apple-radius-sm);
  border: none;
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
</style>

<style>
.replace-word-tooltip {
  .popper__arrow {
    border-top-color: transparent !important;
    border-right-color: var(--apple-divider) !important;
  }
}
.replace-word-content {
  max-width: 400px;
  max-height: 60vh;
  overflow-y: auto;
  scrollbar-width: thin;
  display: flex;
  flex-wrap: wrap;
  gap: 4px;
}
</style>
