<template>
  <div class="welcome">
    <HeaderBar />
    <div class="main-wrapper">
      <div class="content-panel">
        <div class="content-area">
          <el-card class="params-card" shadow="never">
            <div class="operation-header">
              <h2 class="page-title">{{ $t('paramManagement.pageTitle') }}</h2>
              <div class="right-operations">
                <el-input :placeholder="$t('paramManagement.searchPlaceholder')" v-model="searchCode" class="search-input"
                  @keyup.enter.native="handleSearch" clearable />
                <CustomButton icon="el-icon-search" type="confirm" @click="handleSearch">{{ $t('paramManagement.search') }}</CustomButton>
              </div>
            </div>
            <CustomTable
              ref="paramsTable"
              :data="paramsList"
              :columns="tableColumns"
              :loading="loading"
              :show-selection="true"
              :show-operations="true"
              :operations-label="$t('paramManagement.operation')"
              :total="total"
              :current-page="currentPage"
              :page-size="pageSize"
              :page-size-options="pageSizeOptions"
              :hide-text="$t('paramManagement.hide')"
              :view-text="$t('paramManagement.view')"
              @size-change="handlePageSizeChange"
              @page-change="goToPage"
            >
              <template slot="selection" slot-scope="scope">
                <el-checkbox v-model="scope.row.selected"></el-checkbox>
              </template>
              <template slot="paramValue" slot-scope="scope">
                <div v-if="isSensitiveParam(scope.row.paramCode)" class="param-value-row">
                  <span class="param-value-mask" v-if="!scope.row.showValue">
                    {{ maskSensitiveValue(scope.row.paramValue) }}
                  </span>
                  <span class="param-value-text" v-else>{{ scope.row.paramValue }}</span>
                  <UiButton type="text" size="small" class="view-btn" @click="toggleSensitiveValue(scope.row)">
                    {{ scope.row.showValue ? $t('paramManagement.hide') : $t('paramManagement.view') }}
                  </UiButton>
                </div>
                <span v-else class="param-value-text">{{ scope.row.paramValue }}</span>
              </template>
              <template slot="operations" slot-scope="scope">
                <UiButton type="text" size="small" class="edit-btn" @click="editParam(scope.row)">
                  {{ $t('paramManagement.edit') }}
                </UiButton>
                <UiButton type="text" size="small" class="delete-btn" @click="deleteParam(scope.row)">
                  {{ $t('paramManagement.delete') }}
                </UiButton>
              </template>
              <template slot="footer-btns">
                <div class="ctrl_btn">
                  <CustomButton :icon="isAllSelected ? 'el-icon-circle-close' : 'el-icon-circle-check'" size="small" @click="handleSelectAll">
                    {{ isAllSelected ? $t('paramManagement.deselectAll') : $t('paramManagement.selectAll') }}
                  </CustomButton>
                  <CustomButton icon="el-icon-plus" type="add" size="small" @click="showAddDialog">
                    {{ $t('paramManagement.add') }}
                  </CustomButton>
                  <CustomButton size="small" type="delete" icon="el-icon-delete" @click="deleteSelectedParams">
                    {{ $t('paramManagement.delete') }}
                  </CustomButton>
                </div>
              </template>
            </CustomTable>
          </el-card>
        </div>
      </div>
    </div>

    <param-dialog
      ref="paramDialog"
      :title="dialogTitle"
      :visible.sync="dialogVisible"
      :form="paramForm"
      @submit="handleSubmit"
      @cancel="dialogVisible = false"
    />
    <el-footer>
      <version-footer />
    </el-footer>
  </div>
</template>

<script>
import Api from "@/apis/api";
import HeaderBar from "@/components/HeaderBar.vue";
import ParamDialog from "@/components/ParamDialog.vue";
import VersionFooter from "@/components/VersionFooter.vue";
import CustomButton from "@/components/CustomButton.vue";
import CustomTable from "@/components/CustomTable.vue";
import CustomDialog from "@/components/CustomDialog.vue";
import UiButton from "@/components/ui/UiButton.vue";

export default {
  components: { HeaderBar, ParamDialog, VersionFooter, CustomButton, CustomTable, CustomDialog, UiButton },
  data() {
    return {
      searchCode: "",
      paramsList: [],
      currentPage: 1,
      loading: false,
      pageSize: 10,
      pageSizeOptions: [10, 20, 50, 100],
      total: 0,
      dialogVisible: false,
      dialogTitle: "新增参数",
      isAllSelected: false,
      sensitive_keys: ["api_key", "personal_access_token", "access_token", "token", "secret", "access_key_secret", "secret_key", "password", "mqtt_signature_key", "private_key"],
      paramForm: {
        id: null,
        paramCode: "",
        paramValue: "",
        valueType: "string",
        remark: ""
      },
      tableColumns: []
    };
  },
  created() {
    this.initTableColumns();
    this.fetchParams();

  },
  methods: {
    initTableColumns() {
      this.tableColumns = [
        {
          prop: 'paramCode',
          label: this.$t('paramManagement.paramCode'),
          align: 'center'
        },
        {
          prop: 'paramValue',
          label: this.$t('paramManagement.paramValue'),
          align: 'center',
          sensitive: true,
          toggleable: true
        },
        {
          prop: 'remark',
          label: this.$t('paramManagement.remark'),
          align: 'center'
        }
      ];
    },
    handlePageSizeChange(val) {
      this.pageSize = val;
      this.currentPage = 1;
      this.fetchParams();
    },
    fetchParams() {
      this.loading = true;
      Api.admin.getParamsList(
        {
          page: this.currentPage,
          limit: this.pageSize,
          paramCode: this.searchCode,
        },
        ({ data }) => {
          this.loading = false;
          if (data.code === 0) {
            this.paramsList = data.data.list.map(item => ({
              ...item,
              valueType: item.valueType || "string",
              selected: false,
              showValue: false
            }));
            this.total = data.data.total;
          } else {
            this.$message.error({
              message: data.msg || this.$t('paramManagement.getParamsListFailed'),
              showClose: true
            });
          }
        }
      );
    },
    handleSearch() {
      this.currentPage = 1;
      this.fetchParams();
    },
    handleSelectAll() {
      this.isAllSelected = !this.isAllSelected;
      this.paramsList.forEach(row => {
        row.selected = this.isAllSelected;
      });
    },
    showAddDialog() {
      this.dialogTitle = this.$t('paramManagement.addParam');
      this.paramForm = {
        id: null,
        paramCode: "",
        paramValue: "",
        valueType: "string", // 默认值
        remark: ""
      };
      this.dialogVisible = true;
    },
    editParam(row) {
      this.dialogTitle = this.$t('paramManagement.editParam');
      this.paramForm = {
        id: row.id,
        paramCode: row.paramCode,
        paramValue: row.paramValue,
        valueType: row.valueType || "string", // 确保有值
        remark: row.remark
      };
      this.dialogVisible = true;
    },
    handleSubmit(form) {
      if (form.id) {
        // 更新参数
        Api.admin.updateParam(form, ({ data }) => {
          this.dialogVisible = false;
          this.fetchParams();
          this.$message.success({
            message: this.$t('paramManagement.updateSuccess'),
            showClose: true
          });
        }, ({ data }) => {
          this.$message.error({
            message: data.msg || this.$t('paramManagement.updateFailed'),
            showClose: true
          });
          // 调用ParamDialog的resetSaving方法重置保存状态
          if (this.$refs.paramDialog && typeof this.$refs.paramDialog.resetSaving === 'function') {
            this.$refs.paramDialog.resetSaving();
          }
        });
      } else {
        // 新增参数
        Api.admin.addParam(form, ({ data }) => {
          if (data.code === 0) {
            this.dialogVisible = false;
            this.fetchParams();
            this.$message.success({
              message: this.$t('paramManagement.addSuccess'),
              showClose: true
            });
          } else {
            this.$message.error({
              message: data.msg || this.$t('paramManagement.addFailed'),
              showClose: true
            });
            // 调用ParamDialog的resetSaving方法重置保存状态
            if (this.$refs.paramDialog && typeof this.$refs.paramDialog.resetSaving === 'function') {
              this.$refs.paramDialog.resetSaving();
            }
          }
        }, ({ data }) => {
          this.$message.error({
            message: data.msg || this.$t('paramManagement.updateFailed'),
            showClose: true
          });
          // 调用ParamDialog的resetSaving方法重置保存状态
          if (this.$refs.paramDialog && typeof this.$refs.paramDialog.resetSaving === 'function') {
            this.$refs.paramDialog.resetSaving();
          }
        });
      }
    },
    deleteSelectedParams() {
      const selectedParams = this.paramsList.filter(row => row.selected);
      if (selectedParams.length === 0) {
        this.$message.warning({
          message: this.$t('paramManagement.selectParamsFirst'),
          showClose: true
        });
        return;
      }
      this.deleteParams(selectedParams);
    },
    deleteParam(row) {
      if (!row.id) {
        this.$message.warning({
          message: this.$t('paramManagement.selectParamsFirst'),
          showClose: true
        });
        return;
      }
      this.deleteParams([row]);
    },
    deleteParams(params) {
      const paramCount = params.length;
      const paramIds = params.map(param => param.id).filter(id => id);
      if (paramIds.length === 0) {
        this.$message.error({
          message: this.$t('paramManagement.invalidParamId'),
          showClose: true
        });
        return;
      }
      this.$confirm(this.$t('paramManagement.confirmBatchDelete', { paramCount }), this.$t('message.warning'), {
        confirmButtonText: this.$t('button.ok'),
        cancelButtonText: this.$t('button.cancel'),
        type: 'warning'
      }).then(() => {
        Api.admin.deleteParam(paramIds, ({ data }) => {
          if (data.code === 0) {
            // 删除后检查是否需要调整页码
            const newTotal = this.total - paramCount;
            const maxPage = Math.max(1, Math.ceil(newTotal / this.pageSize));
            if (this.currentPage > maxPage) {
              this.currentPage = maxPage;
            }
            this.fetchParams();
            this.$message.success({
              message: this.$t('paramManagement.batchDeleteSuccess', { paramCount }),
              showClose: true
            });
          } else {
            this.$message.error({
              message: data.msg || this.$t('paramManagement.deleteFailed'),
              showClose: true
            });
          }
        });
      }).catch(() => {
        this.$message({
          type: 'info',
          message: this.$t('paramManagement.operationCancelled'),
          duration: 1000
        });
      });
    },
    goToPage(page) {
      if (page !== this.currentPage) {
        this.currentPage = page;
        this.fetchParams();
      }
    },
    isSensitiveParam(paramCode) {
      return this.sensitive_keys.some(key => paramCode.toLowerCase().includes(key));
    },
    maskSensitiveValue(value) {
      if (value.length <= 4) {
        return '****';
      }
      return value.substring(0, 2) + '****' + value.substring(value.length - 2);
    },
    toggleSensitiveValue(row) {
      row.showValue = !row.showValue;
    },
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

.param-value-row {
  display: inline-flex;
  align-items: center;
  gap: 4px;
}

.param-value-mask {
  display: inline-block;
  padding: 2px 10px;
  border-radius: var(--apple-radius-full);
  background: var(--apple-surface-2);
  color: var(--apple-text-2);
  font-family: ui-monospace, "SF Mono", "Menlo", monospace;
  font-size: var(--apple-font-size-xs);
  font-weight: var(--apple-font-weight-medium);
  letter-spacing: 0.04em;
}

.param-value-text {
  color: var(--apple-text);
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
