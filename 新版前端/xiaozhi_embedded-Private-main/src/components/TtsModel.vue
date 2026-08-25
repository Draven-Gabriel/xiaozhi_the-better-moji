<template>
  <CustomDialog :visible.sync="localVisible" :title="$t('modelConfig.voiceManagement')" width="90%"
    :close-on-click-modal="true" :destroy-on-close="false" :footer="false" :append-to-body="true"
    @close="handleClose">
    <div class="scroll-wrapper">
      <div class="table-container" ref="tableContainer" @scroll="handleScroll">
        <el-table v-loading="loading" :data="filteredTtsModels" style="width: 100%;" class="data-table"
          header-row-class-name="table-header" :fit="true" :element-loading-text="$t('voicePrint.loading')"
          element-loading-spinner="el-icon-loading" element-loading-background="rgba(0, 0, 0, 0.8)">
          <el-table-column :label="$t('ttsModel.select')" width="50" align="center">
            <template slot-scope="scope">
              <el-checkbox v-model="scope.row.selected"></el-checkbox>
            </template>
          </el-table-column>
          <el-table-column :label="$t('ttsModel.voiceCode')" align="center">
            <template slot-scope="scope">
              <el-input v-if="scope.row.editing" v-model="scope.row.voiceCode"></el-input>
              <span v-else>{{ scope.row.voiceCode }}</span>
            </template>
          </el-table-column>
          <el-table-column :label="$t('ttsModel.voiceName')" align="center">
            <template slot-scope="scope">
              <el-input v-if="scope.row.editing" v-model="scope.row.voiceName"></el-input>
              <span v-else>{{ scope.row.voiceName }}</span>
            </template>
          </el-table-column>
          <el-table-column :label="$t('ttsModel.languageType')" align="center">
            <template slot-scope="scope">
              <el-input v-if="scope.row.editing" v-model="scope.row.languageType"></el-input>
              <span v-else>{{ scope.row.languageType }}</span>
            </template>
          </el-table-column>
          <el-table-column v-if="!showReferenceColumns" :label="$t('ttsModel.preview')" align="center" class-name="audio-column">
            <template slot-scope="scope">
              <div class="custom-audio-container">
                <el-input v-if="scope.row.editing" v-model="scope.row.voiceDemo" :placeholder="$t('ttsModel.enterMp3Url')"
                  class="audio-input">
                </el-input>
                <AudioPlayer v-else-if="isValidAudioUrl(scope.row.voiceDemo)" :audioUrl="scope.row.voiceDemo" />
              </div>
            </template>
          </el-table-column>
          <el-table-column v-if="!showReferenceColumns" :label="$t('ttsModel.remark')" align="center">
            <template slot-scope="scope">
              <el-input v-if="scope.row.editing" type="textarea" :rows="1" autosize v-model="scope.row.remark"
                  :placeholder="$t('ttsModel.enterRemark')" class="remark-input"></el-input>
              <span v-else>{{ scope.row.remark }}</span>
            </template>
          </el-table-column>
          <el-table-column v-if="showReferenceColumns" :label="$t('ttsModel.referenceAudioPath')" align="center">
            <template slot-scope="scope">
              <el-input v-if="scope.row.editing" v-model="scope.row.referenceAudio" :placeholder="$t('ttsModel.enterReferenceAudio')"></el-input>
              <span v-else>{{ scope.row.referenceAudio }}</span>
            </template>
          </el-table-column>
          <el-table-column v-if="showReferenceColumns" :label="$t('ttsModel.referenceText')" align="center">
            <template slot-scope="scope">
              <el-input v-if="scope.row.editing" v-model="scope.row.referenceText" :placeholder="$t('ttsModel.enterReferenceText')"></el-input>
              <span v-else>{{ scope.row.referenceText }}</span>
            </template>
          </el-table-column>
          <el-table-column :label="$t('ttsModel.operation')" align="center" width="150">
            <template slot-scope="scope">
              <template v-if="!scope.row.editing">
                <UiButton type="text" size="small" @click="startEdit(scope.row)" class="edit-btn">
                    {{ $t('ttsModel.edit') }}
                  </UiButton>
                  <UiButton type="text" size="small" @click="deleteRow(scope.row)" class="delete-btn">
                    {{ $t('ttsModel.delete') }}
                  </UiButton>
              </template>
              <template v-else>
                <UiButton type="default" size="small" @click="cancelEdit(scope.row)">
                  {{ $t('button.cancel') }}
                </UiButton>
                <UiButton type="primary" size="small" @click="saveEdit(scope.row)">
                  {{ $t('ttsModel.save') }}
                </UiButton>
              </template>
            </template>
          </el-table-column>
        </el-table>
      </div>

      <!-- 自定义滚动条 -->
      <div class="custom-scrollbar" ref="scrollbar">
        <div class="custom-scrollbar-track" ref="scrollbarTrack" @click="handleTrackClick">
          <div class="custom-scrollbar-thumb" ref="scrollbarThumb" @mousedown="startDrag"></div>
        </div>
      </div>
    </div>
    <div class="action-buttons">
      <UiButton :icon="selectAll ? 'el-icon-circle-close' : 'el-icon-circle-check'" type="default" size="small" round @click="toggleSelectAll">
        {{ selectAll ? $t('ttsModel.deselectAll') : $t('ttsModel.selectAll') }}
      </UiButton>
      <UiButton icon="el-icon-plus" type="primary" size="small" round @click="addNew">
        {{ $t('ttsModel.add') }}
      </UiButton>
      <UiButton icon="el-icon-delete" type="default" size="small" round class="batch-delete-btn" @click="deleteRow(filteredTtsModels.filter(row => row.selected))">
        {{ $t('ttsModel.delete') }}
      </UiButton>
    </div>
  </CustomDialog>
</template>

<script>
import Api from "@/apis/api";
import AudioPlayer from './AudioPlayer.vue';
import CustomDialog from './CustomDialog.vue';
import UiButton from '@/components/ui/UiButton.vue';

export default {
  components: { AudioPlayer, CustomDialog, UiButton },
  props: {
    visible: {
      type: Boolean,
      default: false
    },
    ttsModelId: {
      type: String,
      required: true
    },
    modelConfig: {
      type: Object,
      default: null
    }
  },
  data() {
    return {
      localVisible: this.visible,
      searchQuery: '',
      editDialogVisible: false,
      editVoiceData: {},
      ttsModels: [],
      currentPage: 1,
      pageSize: 10000,
      total: 0,
      isDragging: false,
      startY: 0,
      scrollTop: 0,
      selectAll: false,
      selectedRows: [],
      loading: false,
      showReferenceColumns: false, // 控制是否显示参考列
    };
  },
  watch: {
    visible(newVal) {
      this.localVisible = newVal;
      if (newVal) {
        this.currentPage = 1;
        this.updateShowReferenceColumns(); // 更新显示状态
        this.loadData(); // 对话框显示时加载数据
        this.$nextTick(() => {
          this.updateScrollbar();
        });
      }
    },
    modelConfig: {
      handler(newVal) {
        this.updateShowReferenceColumns();
      },
      immediate: true
    },
    filteredTtsModels() {
      this.$nextTick(() => {
        this.updateScrollbar();
      });
    }
  },
  computed: {
    filteredTtsModels() {
      return this.ttsModels.filter(model =>
        model.voiceName.toLowerCase().includes(this.searchQuery.toLowerCase())
      );
    }
  },
  mounted() {
    this.updateScrollbar();
    window.addEventListener('resize', this.updateScrollbar);
    window.addEventListener('mouseup', this.stopDrag);
    window.addEventListener('mousemove', this.handleDrag);
  },
  beforeDestroy() {
    window.removeEventListener('resize', this.updateScrollbar);
    window.removeEventListener('mouseup', this.stopDrag);
    window.removeEventListener('mousemove', this.handleDrag);
  },
  methods: {
    // 更新是否显示参考列
    updateShowReferenceColumns() {
      if (this.modelConfig && this.modelConfig.configJson) {
        const providerType = this.modelConfig.configJson.type;
        this.showReferenceColumns = ['fishspeech', 'gpt_sovits_v2', 'gpt_sovits_v3'].includes(providerType);
      } else {
        this.showReferenceColumns = false;
      }
    },

    loadData() {
      this.loading = true;
      const params = {
        ttsModelId: this.ttsModelId,
        page: this.currentPage,
        limit: this.pageSize,
        name: this.searchQuery
      };
      Api.timbre.getVoiceList(params, (data) => {
        if (data.code === 0) {
          this.ttsModels = data.data.list
            .map(item => ({
              id: item.id || '',
              voiceCode: item.ttsVoice || '',
              voiceName: item.name || this.$t('ttsModel.unnamedVoice'),
              languageType: item.languages || '',
              remark: item.remark || '',
              referenceAudio: item.referenceAudio || '',
              referenceText: item.referenceText || '',
              voiceDemo: item.voiceDemo || '',
              selected: false,
              editing: false,
              sort: Number(item.sort)
            }))
            .sort((a, b) => a.sort - b.sort);
          this.total = data.total;
        } else {
          this.$message.error({
            message: data.msg || this.$t('ttsModel.getVoiceListFailed'),
            showClose: true
          });
        }
        this.loading = false;
      }, (err) => {
        console.error('加载失败:', err);
        this.$message.error({
          message: this.$t('ttsModel.loadVoiceDataFailed'),
          showClose: true
        });
        this.loading = false;
      });
    },

    handleClose() {
      // 重置状态
      this.ttsModels = [];
      this.currentPage = 1;
      this.total = 0;
      this.selectAll = false;
      this.searchQuery = '';
      this.showReferenceColumns = false;

      this.localVisible = false;
      this.$emit('update:visible', false);
    },

    updateScrollbar() {
      const container = this.$refs.tableContainer;
      const scrollbarThumb = this.$refs.scrollbarThumb;
      const scrollbarTrack = this.$refs.scrollbarTrack;

      if (!container || !scrollbarThumb || !scrollbarTrack) return;

      const { scrollHeight, clientHeight } = container;
      const trackHeight = scrollbarTrack.clientHeight;
      const thumbHeight = Math.max((clientHeight / scrollHeight) * trackHeight, 20);

      scrollbarThumb.style.height = `${thumbHeight}px`;
      this.updateThumbPosition();
    },

    updateThumbPosition() {
      const container = this.$refs.tableContainer;
      const scrollbarThumb = this.$refs.scrollbarThumb;
      const scrollbarTrack = this.$refs.scrollbarTrack;

      if (!container || !scrollbarThumb || !scrollbarTrack) return;

      const { scrollHeight, clientHeight, scrollTop } = container;
      const trackHeight = scrollbarTrack.clientHeight;
      const thumbHeight = scrollbarThumb.clientHeight;
      const maxTop = trackHeight - thumbHeight;
      const thumbTop = (scrollTop / (scrollHeight - clientHeight)) * (trackHeight - thumbHeight);

      scrollbarThumb.style.top = `${Math.min(thumbTop, maxTop)}px`;
    },

    handleScroll() {
      const container = this.$refs.tableContainer;
      if (container.scrollTop + container.clientHeight >= container.scrollHeight - 50) {
        if (this.currentPage * this.pageSize < this.total) {
          this.currentPage++;
          this.loadData();
        }
      }
      this.updateThumbPosition();
    },

    startDrag(e) {
      this.isDragging = true;
      this.startY = e.clientY;
      this.scrollTop = this.$refs.tableContainer.scrollTop;
      e.preventDefault();
    },

    stopDrag() {
      this.isDragging = false;
    },

    handleDrag(e) {
      if (!this.isDragging) return;

      const container = this.$refs.tableContainer;
      const scrollbarTrack = this.$refs.scrollbarTrack;
      const scrollbarThumb = this.$refs.scrollbarThumb;
      const deltaY = e.clientY - this.startY;
      const trackHeight = scrollbarTrack.clientHeight;
      const thumbHeight = scrollbarThumb.clientHeight;
      const maxScrollTop = container.scrollHeight - container.clientHeight;

      const scrollRatio = (trackHeight - thumbHeight) / maxScrollTop;
      container.scrollTop = this.scrollTop + deltaY / scrollRatio;
    },

    handleTrackClick(e) {
      const container = this.$refs.tableContainer;
      const scrollbarTrack = this.$refs.scrollbarTrack;
      const scrollbarThumb = this.$refs.scrollbarThumb;

      if (!container || !scrollbarTrack || !scrollbarThumb) return;

      const trackRect = scrollbarTrack.getBoundingClientRect();
      const thumbHeight = scrollbarThumb.clientHeight;
      const clickPosition = e.clientY - trackRect.top;
      const thumbCenter = clickPosition - thumbHeight / 2;

      const trackHeight = scrollbarTrack.clientHeight;
      const maxTop = trackHeight - thumbHeight;
      const newTop = Math.max(0, Math.min(thumbCenter, maxTop));

      scrollbarThumb.style.top = `${newTop}px`;
      container.scrollTop = (newTop / (trackHeight - thumbHeight)) * (container.scrollHeight - container.clientHeight);
    },

    startEdit(row) {
      row.editing = true;
      this.$set(row, 'originalData', { ...row });
    },

    cancelEdit(row) {
      // 通过新增创建的数据，取消编辑时，需要从数组中移除
      if (!row.id) {
        this.ttsModels.shift(row);
      } else {
        Object.assign(row, row.originalData);
        delete row.originalData;
      }
      row.editing = false;
    },

    saveEdit(row) {
      if (!row.voiceCode || !row.voiceName || !row.languageType) {
        this.$message.error({
          message: this.$t('ttsModel.voiceCodeNameLanguageRequired'),
          showClose: true
        });
        return;
      }

      try {
        const params = {
          id: row.id,
          voiceCode: row.voiceCode,
          voiceName: row.voiceName,
          languageType: row.languageType,
          remark: row.remark,
          ttsModelId: this.ttsModelId,
          voiceDemo: row.voiceDemo || '',
          sort: row.sort
        };

        // 只有在显示参考列的情况下才添加参考字段
        if (this.showReferenceColumns) {
          params.referenceAudio = row.referenceAudio;
          params.referenceText = row.referenceText;
        }

        let res;
        if (row.id) {
          // 已有ID，执行更新操作
          Api.timbre.updateVoice(params, (response) => {
            res = response;
            this.handleResponse(res, row);
          });
        } else {
          // 没有ID，执行新增操作
          Api.timbre.saveVoice(params, (response) => {
            res = response;
            this.handleResponse(res, row);
          });
        }
      } catch (error) {
        console.error('操作失败:', error);
        // 异常情况下也恢复原始数据
        if (row.originalData) {
          Object.assign(row, row.originalData);
          row.editing = false;
          delete row.originalData;
        }
        this.$message.error({
          message: this.$t('ttsModel.operationFailed'),
          showClose: true
        });
      }
    },

    handleResponse(res, row) {
      if (res.code === 0) {
        this.$message.success({
          message: row.id ? this.$t('ttsModel.updateSuccess') : this.$t('ttsModel.saveSuccess'),
          showClose: true
        });
        row.editing = false;
        delete row.originalData;
        this.loadData(); // 刷新数据
      } else {
        // 保存失败时恢复原始数据
        if (row.originalData) {
          Object.assign(row, row.originalData);
          row.editing = false;
          delete row.originalData;
        }
        this.$message.error({
            message: res.msg || (row.id ? this.$t('ttsModel.updateFailed') : this.$t('ttsModel.saveFailed')),
            showClose: true
          });
      }
    },

    toggleSelectAll() {
      this.selectAll = !this.selectAll;
      this.filteredTtsModels.forEach(row => {
        row.selected = this.selectAll;
      });
    },

    addNew() {
      const hasEditing = this.ttsModels.some(row => row.editing);
      if (hasEditing) {
        this.$message.warning(this.$t('ttsModel.finishEditingFirst'));
        return;
      }

      const maxSort = this.ttsModels.length > 0
        ? Math.max(...this.ttsModels.map(item => Number(item.sort) || 0))
        : 0;

      const newRow = {
        voiceCode: '',
        voiceName: '',
        languageType: this.$t('editVoiceDialog.defaultLanguageType'),
        voiceDemo: '',
        remark: '',
        referenceAudio: '',
        referenceText: '',
        selected: false,
        editing: true,
        sort: 0 // 新增数据默认排序在顶部
      };

      this.ttsModels.unshift(newRow);
    },

    deleteRow(row) {
      // 处理单个音色或音色数组
      const voices = Array.isArray(row) ? row : [row];

      if (Array.isArray(row) && row.length === 0) {
        this.$message.warning(this.$t('ttsModel.selectVoiceToDelete'));
        return;
      }


      const voiceCount = voices.length;
      this.$confirm(this.$t('ttsModel.confirmDeleteVoice', {count: voiceCount}), this.$t('ttsModel.warning'), {
        confirmButtonText: this.$t('common.confirm'),
        cancelButtonText: this.$t('common.cancel'),
        type: "warning",
        distinguishCancelAndClose: true
      }).then(() => {
        const ids = voices.map(voice => voice.id);
        if (ids.some(id => !id)) {
          this.$message.error(this.$t('ttsModel.invalidVoiceId'));
          return;
        }

        Api.timbre.deleteVoice(ids, ({ data }) => {
          if (data.code === 0) {
            this.$message.success({
              message: this.$t('ttsModel.deleteVoiceSuccess', {count: voiceCount}),
              showClose: true
            });
            this.loadData(); // 刷新参数列表
          } else {
            this.$message.error({
              message: data.msg || this.$t('ttsModel.deleteFailed'),
              showClose: true
            });
          }
        });
      }).catch(action => {
        if (action === 'cancel') {
          this.$message({
            type: 'info',
            message: this.$t('ttsModel.deleteCancelled'),
            duration: 1000
          });
        } else {
          this.$message({
            type: 'info',
            message: this.$t('ttsModel.operationClosed'),
            duration: 1000
          });
        }
      });
    },

    isValidAudioUrl(url) {
      return url && (url.endsWith('.mp3') || url.endsWith('.ogg') || url.endsWith('.wav'));
    }
  }
};
</script>

<style lang="scss" scoped>
/* 表格样式 */
::v-deep .data-table .el-table__header th {
  color: var(--apple-text);
  background-color: var(--apple-surface-2);
  padding: 6px 0 !important;
}

::v-deep .data-table .el-table__row td {
  padding: 8px 0 12px !important;
}

::v-deep .data-table {
  border: none !important;
}

::v-deep .data-table.el-table::before {
  display: none !important;
}

::v-deep .data-table .el-table__header-wrapper {
  border-bottom: 1px solid var(--apple-divider-soft) !important;
}

::v-deep .data-table .el-table__body-wrapper .el-table__body td {
  border: none !important;
}

::v-deep .el-table tr {
  background-color: var(--apple-surface);
  transition: background-color var(--apple-duration-fast) var(--apple-ease-standard);
}

::v-deep .el-table__row:hover > td {
  background-color: var(--apple-surface-2) !important;
}

/* 备注文本 */
::v-deep .remark-input .el-textarea__inner {
  border-radius: var(--apple-radius-sm);
  border: 1px solid transparent;
  background-color: var(--apple-surface-2);
  padding: 8px 12px;
  resize: none;
  max-height: 40px !important;
  line-height: 1.5;
  color: var(--apple-text);
  transition: all var(--apple-duration-fast) var(--apple-ease-standard);
}

::v-deep .remark-input .el-textarea__inner:focus {
  border-color: var(--apple-brand) !important;
  background-color: var(--apple-surface);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.24);
  outline: none;
}

::v-deep .remark-input .el-textarea__inner::placeholder {
  color: var(--apple-text-3) !important;
  opacity: 1;
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


/* 滚动容器 */
.scroll-wrapper {
  display: flex;
  max-height: 55vh;
  position: relative;
}

.table-container {
  flex: 1;
  overflow: auto;
  scrollbar-width: none;
  padding-right: 15px;
  width: calc(100% - 16px);
}

.table-container::-webkit-scrollbar {
  display: none;
}

/* 自定义滚动条 */
.custom-scrollbar {
  width: 8px;
  background: var(--apple-surface-2);
  border-radius: 4px;
  position: relative;
  margin-left: 8px;
  height: 100%;
  top: 55px;
}

.custom-scrollbar-track {
  position: relative;
  height: 380px;
  cursor: pointer;
}

.custom-scrollbar-thumb {
  position: absolute;
  width: 100%;
  background: var(--apple-divider);
  border-radius: 4px;
  cursor: grab;
  transition: background var(--apple-duration-fast) var(--apple-ease-standard);
}

.custom-scrollbar-thumb:hover {
  background: var(--apple-brand);
}

.custom-scrollbar-thumb:active {
  cursor: grabbing;
  background: var(--apple-brand-hover);
}

.custom-audio-container audio {
  display: none;
}

/* 音频播放器容器样式 */
.custom-audio-container {
  width: 90%;
  margin: 0 auto;
}

::v-deep .edit-btn.ui-btn--text,
::v-deep .delete-btn.ui-btn--text {
  padding: 4px 8px;
  font-size: 13px;
  line-height: 1;
  border-radius: var(--apple-radius-sm);
  height: auto;
  min-height: 0;
  margin: 0 4px;
}

::v-deep .edit-btn.ui-btn--text:hover {
  background: var(--apple-brand-tint);
  color: var(--apple-brand-hover);
}

::v-deep .delete-btn.ui-btn--text:hover {
  background: rgba(255, 59, 48, 0.10) !important;
  color: var(--apple-danger) !important;
}

::v-deep .batch-delete-btn.ui-btn--default:hover {
  background: rgba(255, 59, 48, 0.10) !important;
  color: var(--apple-danger) !important;
  border-color: rgba(255, 59, 48, 0.20) !important;
  transform: translateY(-1px);
  box-shadow: 0 2px 8px rgba(255, 59, 48, 0.20);
}

/* 表格单元格自适应 */
::v-deep .el-table__body-wrapper {
  overflow-x: hidden !important;
}

::v-deep .el-table td {
  white-space: pre-wrap !important;
  word-break: break-all !important;
}

/* 按钮组定位调整 */
.action-buttons {
  display: flex;
  align-items: center;
  gap: 8px;
  padding-top: 10px;
  text-align: left;
}

/* 输入框自适应 */
::v-deep .el-input__inner,
::v-deep .el-textarea__inner {
  width: 100% !important;
  min-width: 120px;
}

/* 音频输入框特殊处理 */
.audio-input ::v-deep .el-input__inner {
  min-width: 200px;
}

/* 操作按钮弹性布局 */
::v-deep .el-table__row .el-button {
  flex-shrink: 0;
  margin: 2px !important;
}
</style>