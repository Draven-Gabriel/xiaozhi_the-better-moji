<template>
  <div class="welcome">
    <HeaderBar />
    <div class="main-wrapper">
      <el-card class="main-card" shadow="never">
        <div class="operation-header">
          <h2 class="page-title">{{ $t('dictManagement.pageTitle') }}</h2>
          <div class="right-operations">
            <el-input :placeholder="$t('dictManagement.searchPlaceholder')" v-model="search" class="search-input"
              @keyup.enter.native="handleSearch" clearable />
            <CustomButton icon="el-icon-search" type="confirm" @click="handleSearch">
              {{ $t('dictManagement.search') }}
            </CustomButton>
          </div>
        </div>
        <div class="content-panel">
          <div class="dict-type-panel">
            <div class="dict-type-header">
              <h4 class="dict-type-title">{{ $t('dictManagement.dictTypeCategory') }}</h4>
              <div class="dict-type-header-actions">
                <UiButton type="default" size="small" circle icon="el-icon-plus" @click="showAddDictTypeDialog" />
                <UiButton type="default" size="small" circle icon="el-icon-delete" @click="batchDeleteDictType" />
              </div>
            </div>
            <div class="dict-type-checkbox" v-loading="dictTypeLoading">
              <el-checkbox class="dict-type-all" :indeterminate="isIndeterminate" v-model="checkAll" @change="handleCheckAllChange">{{ $t('common.selectAll') }}</el-checkbox>
              <el-checkbox-group v-model="checkedDictTypesIds" @change="handleDictTypeSelectionChange">
                <div
                  class="dict-type-item"
                  :class="{ 'dict-type-active': selectedDictType?.id === item.id }"
                  v-for="item in dictTypeList"
                  :key="item.id"
                >
                  <el-checkbox :label="item.dictName" :key="item.id">
                  </el-checkbox>
                  <div class="dict-type-item-content" @click="handleDictTypeRowClick(item)">
                    <span class="dict-type-name">{{item.dictName}}</span>
                    <UiButton type="text" size="small" class="dict-type-edit-btn" icon="el-icon-edit" @click.stop="editDictType(item)" />
                  </div>
                </div>
              </el-checkbox-group>
            </div>
          </div>
          <div class="content-area">
            <CustomTable :data="dictDataList" :columns="tableColumns" :loading="dictDataLoading" :show-selection="true"
              :show-operations="true" :operations-label="$t('dictManagement.operation')" :total="total"
              :current-page="currentPage" :page-size="pageSize" :page-size-options="pageSizeOptions"
              @size-change="handlePageSizeChange" @page-change="goToPage">
              <template slot="selection" slot-scope="scope">
                <el-checkbox v-model="scope.row.selected"></el-checkbox>
              </template>
              <template slot="operations" slot-scope="scope">
                <UiButton type="text" size="small" class="edit-btn" @click="editDictData(scope.row)">
                  {{ $t('dictManagement.edit') }}
                </UiButton>
                <UiButton type="text" size="small" class="delete-btn" @click="deleteDictData(scope.row)">
                  {{ $t('dictManagement.delete') }}
                </UiButton>
              </template>
              <template slot="footer-btns">
                <div class="ctrl_btn">
                  <CustomButton :icon="isAllDictDataSelected ? 'el-icon-circle-close' : 'el-icon-circle-check'"
                    size="small" @click="selectAllDictData">
                    {{ isAllDictDataSelected ? $t('dictManagement.deselectAll') : $t('dictManagement.selectAll') }}
                  </CustomButton>
                  <CustomButton type="add" icon="el-icon-plus" size="small" @click="showAddDictDataDialog">
                    {{ $t('dictManagement.addDictData') }}
                  </CustomButton>
                  <CustomButton size="small" type="delete" icon="el-icon-delete" @click="batchDeleteDictData">
                    {{ $t('dictManagement.batchDeleteDictData') }}
                  </CustomButton>
                </div>
              </template>
            </CustomTable>
          </div>
        </div>
      </el-card>
    </div>

    <DictTypeDialog :visible.sync="dictTypeDialogVisible" :title="dictTypeDialogTitle" :dictTypeData="dictTypeForm"
      @save="saveDictType" />

    <DictDataDialog :visible.sync="dictDataDialogVisible" :title="dictDataDialogTitle" :dictData="dictDataForm"
      :dictTypeId="selectedDictType?.id" @save="saveDictData" />
    <el-footer style="flex-shrink:unset;">
      <version-footer />
    </el-footer>
  </div>
</template>

<script>
import dictApi from '@/apis/module/dict'
import DictDataDialog from '@/components/DictDataDialog.vue'
import DictTypeDialog from '@/components/DictTypeDialog.vue'
import HeaderBar from '@/components/HeaderBar.vue'
import VersionFooter from '@/components/VersionFooter.vue'
import CustomButton from '@/components/CustomButton.vue'
import CustomTable from '@/components/CustomTable.vue'
import UiButton from '@/components/ui/UiButton.vue'
export default {
  name: 'DictManagement',
  components: {
    HeaderBar,
    DictDataDialog,
    DictTypeDialog,
    VersionFooter,
    CustomButton,
    CustomTable,
    UiButton
  },
  data() {
    return {
      dictTypeList: [],
      dictTypeLoading: false,
      selectedDictType: null,
      selectedDictTypes: [],
      dictTypeDialogVisible: false,
      dictTypeDialogTitle: '新增字典类型',
      dictTypeForm: {
        id: null,
        dictName: '',
        dictType: ''
      },

      dictDataList: [],
      dictDataLoading: false,
      isAllDictDataSelected: false,
      dictDataDialogVisible: false,
      dictDataDialogTitle: '新增字典数据',
      dictDataForm: {
        id: null,
        dictTypeId: null,
        dictLabel: '',
        dictValue: '',
        sort: 0
      },
      search: '',
      pageSizeOptions: [10, 20, 50, 100],
      currentPage: 1,
      pageSize: 10,
      total: 0,
      tableColumns: [],
      isIndeterminate: false,
      checkAll: false,
      checkedDictTypesIds: []
    }
  },
  created() {
    this.initTableColumns()
    this.loadDictTypeList()
  },
  methods: {
    handleCheckAllChange(val) {
      const isAllSelected = this.checkedDictTypesIds.length === this.dictTypeList.length;
      if (isAllSelected) {
        this.checkedDictTypesIds = [];
        this.checkAll = false;
        this.selectedDictTypes = [];
      } else {
        this.checkedDictTypesIds = this.dictTypeList.map(item => item.dictName);
        this.checkAll = true;
        this.selectedDictTypes = [...this.dictTypeList];
      }
      this.isIndeterminate = false;
    },
    handleDictTypeSelectionChange(checkedIds) {
      this.selectedDictTypes = this.dictTypeList.filter(item => checkedIds.includes(item.dictName));
      this.checkAll = checkedIds.length > 0;
      this.isIndeterminate = checkedIds.length > 0 && checkedIds.length < this.dictTypeList.length;
    },
    initTableColumns() {
      this.tableColumns = [
        {
          prop: 'dictLabel',
          label: this.$t('dictManagement.dictLabel'),
          align: 'center'
        },
        {
          prop: 'dictValue',
          label: this.$t('dictManagement.dictValue'),
          align: 'center'
        },
        {
          prop: 'sort',
          label: this.$t('dictManagement.sort'),
          align: 'center'
        }
      ]
    },
    loadDictTypeList() {
      this.dictTypeLoading = true
      dictApi.getDictTypeList({
        page: 1,
        limit: 100,
        dictName: this.search
      }, ({ data }) => {
        if (data.code === 0) {
          this.dictTypeList = data.data.list
          if (this.dictTypeList.length > 0) {
            this.selectedDictType = this.dictTypeList[0]
            this.loadDictDataList(this.dictTypeList[0].id)
          }
        }
        this.dictTypeLoading = false
      })
    },
    handleDictTypeRowClick(row) {
      this.selectedDictType = row
      this.loadDictDataList(row.id)
    },
    showAddDictTypeDialog() {
      this.dictTypeDialogTitle = this.$t('dictManagement.addDictType')
      this.dictTypeForm = {
        id: null,
        dictName: '',
        dictType: ''
      }
      this.dictTypeDialogVisible = true
    },
    editDictType(row) {
      this.dictTypeDialogTitle = this.$t('dictManagement.editDictType')
      this.dictTypeForm = { ...row }
      this.dictTypeDialogVisible = true
    },
    saveDictType(formData) {
      const api = formData.id ? dictApi.updateDictType : dictApi.addDictType
      api(formData, ({ data }) => {
        if (data.code === 0) {
          this.$message.success(this.$t('dictManagement.saveSuccess'))
          this.dictTypeDialogVisible = false
          this.loadDictTypeList()
        }
      })
    },
    batchDeleteDictType() {
      if (this.selectedDictTypes.length === 0) {
        this.$message.warning(this.$t('dictManagement.selectDictTypeToDelete'))
        return
      }

      this.$confirm(this.$t('dictManagement.confirmDeleteDictType'), this.$t('dictManagement.confirm'), {
        confirmButtonText: this.$t('dictManagement.confirm'),
        cancelButtonText: this.$t('dictManagement.cancel'),
        type: 'warning'
      }).then(() => {
        const ids = this.selectedDictTypes.map(item => item.id)
        dictApi.deleteDictType(ids, ({ data }) => {
          if (data.code === 0) {
            this.$message.success(this.$t('dictManagement.deleteSuccess'))
            this.loadDictTypeList()
          }
        })
      })
    },

    loadDictDataList(dictTypeId) {
      if (!dictTypeId) return
      this.dictDataLoading = true
      dictApi.getDictDataList({
        dictTypeId,
        page: this.currentPage,
        limit: this.pageSize,
        dictLabel: this.search,
        dictValue: ''
      }, ({ data }) => {
        if (data.code === 0) {
          this.dictDataList = data.data.list.map(item => ({
            ...item,
            selected: false
          }))
          this.total = data.data.total
        } else {
          this.$message.error(data.msg || this.$t('dictManagement.getDictDataFailed'))
        }
        this.dictDataLoading = false
      })
    },
    selectAllDictData() {
      this.isAllDictDataSelected = !this.isAllDictDataSelected
      this.dictDataList.forEach(row => {
        row.selected = this.isAllDictDataSelected
      })
    },
    showAddDictDataDialog() {
      if (!this.selectedDictType) {
        this.$message.warning(this.$t('dictManagement.selectDictTypeFirst'))
        return
      }
      this.dictDataDialogTitle = this.$t('dictManagement.addDictData')
      this.dictDataForm = {
        id: null,
        dictTypeId: this.selectedDictType.id,
        dictLabel: '',
        dictValue: '',
        sort: 0
      }
      this.dictDataDialogVisible = true
    },
    editDictData(row) {
      this.dictDataDialogTitle = this.$t('dictManagement.editDictData')
      this.dictDataForm = { ...row }
      this.dictDataDialogVisible = true
    },
    saveDictData(formData) {
      const api = formData.id ? dictApi.updateDictData : dictApi.addDictData
      api(formData, ({ data }) => {
        if (data.code === 0) {
          this.$message.success(this.$t('dictManagement.saveSuccess'))
          this.dictDataDialogVisible = false
          this.loadDictDataList(formData.dictTypeId)
        }
      })
    },
    deleteDictData(row) {
      this.$confirm(this.$t('dictManagement.confirmDeleteDictData'), this.$t('dictManagement.confirm'), {
        confirmButtonText: this.$t('dictManagement.confirm'),
        cancelButtonText: this.$t('dictManagement.cancel'),
        type: 'warning'
      }).then(() => {
        dictApi.deleteDictData([row.id], ({ data }) => {
          if (data.code === 0) {
            this.$message.success(this.$t('dictManagement.deleteSuccess'))
            this.loadDictDataList(row.dictTypeId)
          }
        })
      })
    },
    batchDeleteDictData() {
      const selectedRows = this.dictDataList.filter(row => row.selected)
      if (selectedRows.length === 0) {
        this.$message.warning(this.$t('dictManagement.selectDictDataToDelete'))
        return
      }

      this.$confirm(this.$t('dictManagement.confirmBatchDeleteDictData', { count: selectedRows.length }), this.$t('dictManagement.confirm'), {
        confirmButtonText: this.$t('dictManagement.confirm'),
        cancelButtonText: this.$t('dictManagement.cancel'),
        type: 'warning'
      }).then(() => {
        const ids = selectedRows.map(item => item.id)
        dictApi.deleteDictData(ids, ({ data }) => {
          if (data.code === 0) {
            this.$message.success(this.$t('dictManagement.deleteSuccess'))
            this.loadDictDataList(this.selectedDictType.id)
          }
        })
      })
    },
    handleSearch() {
      if (!this.selectedDictType) {
        this.$message.warning(this.$t('dictManagement.selectDictTypeFirst'))
        return
      }
      this.currentPage = 1
      this.loadDictDataList(this.selectedDictType.id)
    },
    handlePageSizeChange(val) {
      this.pageSize = val
      this.currentPage = 1
      this.loadDictDataList(this.selectedDictType?.id)
    },
    goToPage(page) {
      if (page !== this.currentPage) {
        this.currentPage = page
        this.loadDictDataList(this.selectedDictType?.id)
      }
    }
  }
}
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

.content-panel {
  flex: 1;
  gap: 16px;
  padding: 0 20px 20px;
  display: flex;
  min-height: 0;
}

.dict-type-panel {
  width: 320px;
  display: flex;
  flex-direction: column;
  background: var(--apple-surface);
  border-radius: var(--apple-radius-lg);
  box-shadow: var(--apple-shadow-card);
  border: 1px solid var(--apple-divider-soft);

  .dict-type-checkbox {
    flex: 1;
    padding: 14px 10px 18px;
    display: flex;
    flex-direction: column;
    align-items: flex-start;
    gap: 8px;
    overflow: auto;

    .dict-type-all {
      margin-left: 10px;
      margin-bottom: 4px;
      font-size: var(--apple-font-size-xs);
      color: var(--apple-text-2);
    }

    ::v-deep .el-checkbox-group {
      width: 100%;
      gap: 6px;
      display: flex;
      flex-direction: column;
      align-items: flex-start;

      .el-checkbox__label {
        display: none;
      }

      .el-checkbox {
        margin-right: 0;
      }
    }

    .dict-type-item {
      padding: 0 10px;
      padding-right: 4px;
      width: 100%;
      display: flex;
      align-items: center;
      cursor: pointer;
      box-sizing: border-box;
      border-radius: var(--apple-radius-md);
      transition:
        background-color var(--apple-duration-fast) var(--apple-ease-standard),
        color var(--apple-duration-fast) var(--apple-ease-standard);

      ::v-deep .el-checkbox__inner {
        background-color: var(--apple-surface) !important;
        border-color: var(--apple-divider) !important;
      }

      ::v-deep .el-checkbox__input.is-checked .el-checkbox__inner {
        background-color: var(--apple-brand) !important;
        border-color: var(--apple-brand) !important;
      }

      &:hover {
        background: var(--apple-surface-2);

        .dict-type-name,
        ::v-deep .dict-type-edit-btn.ui-btn--text {
          color: var(--apple-brand) !important;
        }
      }

      .dict-type-item-content {
        flex: 1;
        padding: 8px 0;
        display: flex;
        align-items: center;
        justify-content: space-between;
      }

      .dict-type-name {
        margin-left: 10px;
        font-size: var(--apple-font-size-sm);
        color: var(--apple-text);
        font-weight: var(--apple-font-weight-regular);
        transition: color var(--apple-duration-fast) var(--apple-ease-standard);
      }
    }

    .dict-type-active {
      background: var(--apple-brand-tint);

      .dict-type-name {
        color: var(--apple-brand);
        font-weight: var(--apple-font-weight-semibold);
      }

      ::v-deep .dict-type-edit-btn.ui-btn--text {
        color: var(--apple-brand) !important;
      }
    }
  }
}

.dict-type-header {
  display: flex;
  padding: 14px 16px;
  border-bottom: 1px solid var(--apple-divider-soft);
  justify-content: space-between;
  align-items: center;

  .dict-type-title {
    margin: 0;
    font-size: var(--apple-font-size-sm);
    font-weight: var(--apple-font-weight-semibold);
    color: var(--apple-text);
    letter-spacing: -0.01em;
  }
}

.dict-type-header-actions {
  display: flex;
  gap: 6px;
}

::v-deep .dict-type-edit-btn.ui-btn--text {
  padding: 4px 6px;
  font-size: 13px;
  color: var(--apple-text-3);
  height: auto;
  min-height: 0;
  margin: 0;

  &:hover {
    background: transparent !important;
    color: var(--apple-brand) !important;
  }
}

.content-area {
  flex: 1;
  height: 100%;
  min-width: 600px;
  display: flex;
  flex-direction: column;
  padding: 0;
  box-sizing: border-box;
}

.main-card {
  background: var(--apple-surface);
  flex: 1;
  display: flex;
  flex-direction: column;
  border: none;
  border-radius: var(--apple-radius-lg);
  overflow: hidden;
  box-shadow: var(--apple-shadow-card);

  ::v-deep .el-card__body {
    padding: 0;
    display: flex;
    flex-direction: column;
    flex: 1;
    overflow: hidden;
  }
}

.operation-header {
  display: flex;
  align-items: center;
  justify-content: space-between;
  padding: 18px 20px 16px;
  flex-shrink: 0;
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

.ctrl_btn {
  display: flex;
  gap: 10px;
  align-items: center;
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

::v-deep .edit-btn.ui-btn--text:hover {
  background: var(--apple-brand-tint) !important;
  color: var(--apple-brand-hover) !important;
}

::v-deep .delete-btn.ui-btn--text:hover {
  background: rgba(255, 59, 48, 0.10) !important;
  color: var(--apple-danger) !important;
}

::v-deep .el-checkbox__inner {
  background-color: var(--apple-surface) !important;
  border-color: var(--apple-divider) !important;
}

::v-deep .el-checkbox__inner:hover {
  border-color: var(--apple-divider) !important;
}

::v-deep .el-checkbox__input.is-checked .el-checkbox__inner {
  background-color: var(--apple-brand) !important;
  border-color: var(--apple-brand) !important;
}
</style>
