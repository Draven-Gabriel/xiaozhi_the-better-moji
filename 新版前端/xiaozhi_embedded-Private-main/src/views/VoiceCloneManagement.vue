<template>
  <div class="welcome">
    <HeaderBar />
    <div class="main-wrapper">
      <div class="content-panel">
        <div class="content-area">
          <el-card class="params-card" shadow="never">
            <div class="operation-header">
              <h2 class="page-title">{{ $t('voiceClone.title') }}</h2>
              <div class="right-operations">
                <el-input :placeholder="$t('voiceClone.searchPlaceholder')" v-model="searchName" class="search-input"
                  @keyup.enter.native="handleSearch" clearable />
                <CustomButton type="confirm" icon="el-icon-search" @click="handleSearch">{{ $t('voiceClone.search') }}</CustomButton>
              </div>
            </div>
            <div v-loading="loading" class="voice-clone-grid">
              <div v-for="item in voiceCloneList" :key="item.id" class="voice-clone-card">
                <div class="card-top">
                  <img src="@/assets/setting/voiceclone.png" alt="" width="80px" height="80px">
                  <div class="card-info">
                    <div class="info-left">
                      <div class="info-title">
                        <el-input v-show="item.isEdit" v-model="item.name" size="mini" maxlength="64"
                          show-word-limit @keyup.enter.native="onNameEnter(item)"
                          ref="nameInput" />
                        <span v-show="!item.isEdit" class="name-text" :title="item.name">{{ item.name || '-' }}</span>
                      </div>
                      <div class="info-grid">
                        <div class="info-row">
                          <span class="info-label">{{ $t('voiceClone.voiceId') }}</span>
                          <span class="info-value" :title="item.voiceId">{{ item.voiceId || '-' }}</span>
                        </div>
                        <div class="info-row">
                          <span class="info-label">{{ $t('voiceClone.languages') }}</span>
                          <span class="info-value">{{ item.languages || '-' }}</span>
                        </div>
                      </div>
                    </div>
                    <div class="info-right" :class="{ 'info-right--centered': !item.hasVoice }">
                       <el-tooltip :content="getTooltipContent(item)" placement="top" effect="light">
                          <span class="status-pill" :class="getStatusButtonClass(item)">
                            {{ getTrainStatusText(item) }}
                          </span>
                       </el-tooltip>
                      <div v-if="item.hasVoice" class="voice-wave" :class="{ 'is-playing': playingRowId === item.id }">
                        <span class="wave-bar"></span>
                        <span class="wave-bar"></span>
                        <span class="wave-bar"></span>
                        <span class="wave-bar"></span>
                        <span class="wave-bar"></span>
                        <span class="wave-bar"></span>
                        <span class="wave-bar"></span>
                        <span class="wave-bar"></span>
                        <span class="wave-bar"></span>
                        <span class="wave-bar"></span>
                        <span class="wave-bar"></span>
                      </div>
                    </div>
                  </div>
                </div>

                <div class="card-actions">
                  <UiButton v-if="item.hasVoice" type="text" size="small" icon="el-icon-video-play" class="card-action-btn"
                    @click="handlePlay(item)">
                    {{ playingRowId === item.id ? $t('voiceClone.stop') : $t('voiceClone.play') }}
                  </UiButton>
                  <UiButton type="text" size="small" icon="el-icon-upload2" class="card-action-btn"
                    @click="handleUpload(item)">
                    {{ $t('voiceClone.upload') }}
                  </UiButton>
                  <UiButton v-if="item.hasVoice" type="text" size="small" icon="el-icon-copy-document" class="card-action-btn"
                    @click="handleClone(item)" :loading="item._cloning">
                    {{ $t('voiceClone.clone') }}
                  </UiButton>
                  <UiButton type="text" size="small"
                    :icon="item.isEdit ? 'el-icon-check' : 'el-icon-edit'"
                    class="card-action-btn"
                    @click="handleEditButtonClick(item)">
                    {{ item.isEdit ? $t('button.save') : $t('common.edit') }}
                  </UiButton>
                </div>
              </div>

              <div v-if="!loading && voiceCloneList.length === 0" class="empty-state">
                {{ $t('voiceClone.noVoiceCloneAssigned') }}
              </div>
            </div>

            <CustomPagination
              :total="total"
              :current-page="currentPage"
              :page-size="pageSize"
              :page-size-options="pageSizeOptions"
              @size-change="handlePageSizeChange"
              @page-change="goToPage"
            />
          </el-card>
        </div>
      </div>
    </div>

    <el-footer>
      <version-footer />
    </el-footer>

    <VoiceCloneDialog :visible.sync="cloneDialogVisible" :voiceCloneData="currentVoiceClone"
      @success="handleCloneSuccess" />
  </div>
</template>

<script>
import Api from "@/apis/api";
import HeaderBar from "@/components/HeaderBar.vue";
import VersionFooter from "@/components/VersionFooter.vue";
import VoiceCloneDialog from "@/components/VoiceCloneDialog.vue";
import CustomButton from "@/components/CustomButton.vue";
import CustomPagination from "@/components/CustomPagination.vue";
import UiButton from "@/components/ui/UiButton.vue";
import { formatDate } from "@/utils/format";

export default {
  components: { HeaderBar, VersionFooter, VoiceCloneDialog, CustomButton, CustomPagination, UiButton },
  data() {
    return {
      searchName: "",
      loading: false,
      voiceCloneList: [],
      currentPage: 1,
      pageSize: 10,
      pageSizeOptions: [10, 20, 50, 100],
      total: 0,
      cloneDialogVisible: false,
      currentVoiceClone: {},
      voiceCloneForm: {
        modelId: "",
        voiceIds: [],
        userId: null
      },
      currentAudio: null,
      playingRowId: null
    };
  },
  created() {
    this.fetchVoiceCloneList();
  },
  methods: {
    getTooltipContent(row) {
      if (!row.hasVoice) {
        return this.$t('voiceClone.waitingUpload');
      }
      switch (row.trainStatus) {
        case 0:
          return this.$t('voiceClone.waitingTraining');
        case 2:
          return this.$t('voiceClone.trainSuccess');
        case 3:
          if (row.trainError) {
            return this.$t('voiceClone.trainFailedWithError', { error: row.trainError });
          }
          return this.$t('voiceClone.trainFailed');
        default:
          return '';
      }
    },
    handlePageSizeChange(val) {
      this.pageSize = val;
      this.currentPage = 1;
      this.fetchVoiceCloneList();
    },
    goToPage(page) {
      if (page !== this.currentPage) {
        this.currentPage = page;
        this.fetchVoiceCloneList();
      }
    },
    fetchVoiceCloneList() {
      this.loading = true;
      const params = {
        page: this.currentPage,
        limit: this.pageSize,
        name: this.searchName || "",
        orderField: "create_date",
        order: "desc"
      };
      Api.voiceClone.getVoiceCloneList(params, (res) => {
        this.loading = false;
        res = res.data;
        if (res.code === 0) {
          this.voiceCloneList = res.data.list;
          this.total = res.data.total || 0;
        } else {
          this.voiceCloneList = [];
          this.total = 0;
          this.$message.error({
            message: res?.data?.msg || this.$t('voiceClone.deleteFailed'),
            showClose: true
          });
        }
      });
    },
    handleSearch() {
      this.currentPage = 1;
      this.fetchVoiceCloneList();
    },
    formatDate,
    getTrainStatusText(row) {
      if (!row.hasVoice) {
        return this.$t('voiceClone.waitingUpload');
      }
      switch (row.trainStatus) {
        case 0:
          return this.$t('voiceClone.waitingTraining');
        case 2:
          return this.$t('voiceClone.trainSuccess');
        case 3:
          return this.$t('voiceClone.trainFailed');
        default:
          return '';
      }
    },
    getStatusButtonClass(row) {
      if (!row.hasVoice || row.trainStatus === 0) {
        return 'status-waiting';
      } else if (row.trainStatus === 2) {
        return 'status-success';
      } else if (row.trainStatus === 3) {
        return 'status-failed';
      }
      return '';
    },
    handleClone(row) {
      if (row._cloning) {
        return;
      }
      this.$set(row, '_cloning', true);
      const params = { cloneId: row.id };
      try {
        Api.voiceClone.cloneAudio(params, (res) => {
          try {
            res = res.data;
            if (res.code === 0) {
              this.$message.success(this.$t('message.success'));
              this.fetchVoiceCloneList();
            } else {
              this.$message.error(res.msg || this.$t('message.error'));
              this.fetchVoiceCloneList();
            }
          } catch (error) {
            console.error('处理响应时出错:', error);
            this.$message.error('处理响应时出错');
            this.fetchVoiceCloneList();
          } finally {
            this.$set(row, '_cloning', false);
          }
        }, (error) => {
          console.error('API调用失败:', error);
          this.$message.error(this.$t('voiceClone.cloneErrorTip'));
          this.fetchVoiceCloneList();
          this.$set(row, '_cloning', false);
        });
      } catch (error) {
        console.error('调用API时出错:', error);
        this.$message.error(this.$t('voiceClone.apiError'));
        this.fetchVoiceCloneList();
        this.$set(row, '_cloning', false);
      }
    },
    handleCloneSuccess() {
      this.fetchVoiceCloneList();
    },
    handleEditName(row) {
      this.$set(row, 'isEdit', true);
      this.$nextTick(() => {
        const input = this.$refs.nameInput;
        if (input) {
          if (Array.isArray(input)) {
            const idx = this.voiceCloneList.indexOf(row);
            if (input[idx]) {
              input[idx].focus();
            }
          } else {
            input.focus();
          }
        }
      });
    },
    submitName(row) {
      if (row._submitting) {
        return;
      }
      row._submitting = true;
      const params = { id: row.id, name: row.name };
      Api.voiceClone.updateName(params, (res) => {
        res = res.data;
        if (res.code === 0) {
          this.$message.success(this.$t('voiceClone.updateNameSuccess') || '名称更新成功');
        } else {
          this.$message.error(res.msg || this.$t('voiceClone.updateNameFailed') || '名称更新失败');
          this.fetchVoiceCloneList();
        }
        row._submitting = false;
      });
    },
    handleEditButtonClick(row) {
      if (row.isEdit) {
        row.isEdit = false;
        this.submitName(row);
      } else {
        this.handleEditName(row);
      }
    },
    onNameEnter(row) {
      row.isEdit = false;
      this.submitName(row);
    },
    handlePlay(row) {
      if (this.playingRowId === row.id && this.currentAudio) {
        this.stopCurrentAudio();
        return;
      }
      this.stopCurrentAudio();
      Api.voiceClone.getAudioId(row.id, (res) => {
        res = res.data;
        if (res.code === 0) {
          const uuid = res.data;
          const audioUrl = Api.voiceClone.getPlayVoiceUrl(uuid);
          const audio = new Audio(audioUrl);
          this.currentAudio = audio;
          this.playingRowId = row.id;
          audio.addEventListener('ended', () => {
            this.playingRowId = null;
            this.currentAudio = null;
          });
          audio.addEventListener('error', () => {
            this.playingRowId = null;
            this.currentAudio = null;
          });
          audio.play().catch(err => {
            console.error('播放失败:', err);
            this.$message.error(this.$t('voiceClone.playFailed') || '播放失败');
            this.playingRowId = null;
            this.currentAudio = null;
          });
        } else {
          this.$message.error(res.msg || this.$t('voiceClone.audioNotExist') || '音频不存在');
        }
      });
    },
    stopCurrentAudio() {
      if (this.currentAudio) {
        this.currentAudio.pause();
        this.currentAudio.currentTime = 0;
        this.currentAudio = null;
      }
      this.playingRowId = null;
    },
    handleUpload(row) {
      this.currentVoiceClone = row;
      this.cloneDialogVisible = true;
    }
  }
};
</script>

<style lang="scss" scoped>
@import '@/styles/global.scss';

.welcome {
  min-width: 900px;
  min-height: 506px;
  height: 100vh;
  display: flex;
  position: relative;
  flex-direction: column;
  background-size: cover;
  background: var(--apple-bg);
  -webkit-background-size: cover;
  -o-background-size: cover;
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
  font-weight: 600;
  font-size: 24px;
  margin: 0;
  color: var(--apple-text);
}

.right-operations {
  display: flex;
  gap: 10px;
  margin-left: auto;
}

.search-input {
  width: 240px;
}

.content-panel {
  display: flex;
  overflow: hidden;
  height: 100%;
  border-radius: var(--apple-radius-lg);
  background: var(--apple-surface);
  box-shadow: var(--apple-shadow-card);
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

.params-card {
  background: var(--apple-surface);
  flex: 1;
  display: flex;
  flex-direction: column;
  border: none;
  box-shadow: none;
  overflow: hidden;

  ::v-deep .el-card__body {
    padding: 14px 20px;
    display: flex;
    flex-direction: column;
    flex: 1;
    overflow: hidden;
  }
}

.status-pill {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  padding: 4px 12px;
  border-radius: var(--apple-radius-full);
  font-size: 12px;
  font-weight: 500;
  line-height: 1.4;
  white-space: nowrap;
}

.status-waiting {
  background-color: rgba(0, 0, 0, 0.05);
  color: var(--apple-text-2);
  border: 1px solid transparent;
}

.status-success {
  background-color: rgba(52, 199, 89, 0.14);
  color: #1f7a3a;
  border: 1px solid transparent;
}

.status-failed {
  background-color: rgba(255, 59, 48, 0.12);
  color: #b3261e;
  border: 1px solid transparent;
}

.voice-clone-grid {
  padding: 10px 16px;
  margin-bottom: 20px;
  height: calc(100% - 90px);
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(360px, 1fr));
  gap: 16px;
  align-content: start;
  min-height: 200px;
  overflow: auto;
  @include scrollbar-style;
}

.voice-clone-card {
  display: flex;
  flex-direction: column;
  gap: 14px;
  padding: 16px 16px 8px;
  background: var(--apple-surface);
  box-shadow: var(--apple-shadow-card);
  border-radius: var(--apple-radius-lg);
  border: 1px solid var(--apple-divider-soft);
  transition:
    box-shadow var(--apple-duration-normal) var(--apple-ease-standard),
    transform var(--apple-duration-normal) var(--apple-ease-standard),
    border-color var(--apple-duration-normal) var(--apple-ease-standard);

  &:hover {
    transform: translateY(-2px);
    box-shadow: var(--apple-shadow-raised);
    border-color: var(--apple-brand-tint);
  }
}

.card-top {
  display: flex;
  gap: 12px;
}

.voice-wave {
  display: flex;
  align-items: center;
  justify-content: center;
  gap: 5px;
  flex: 1;
  min-height: 40px;
  width: 100%;
  padding: 0 4px;

  .wave-bar {
    width: 3px;
    height: 50%;
    max-height: 80px;
    background: linear-gradient(to top, var(--apple-brand-tint) 0%, var(--apple-brand) 100%);
    border-radius: 2px;
    transform-origin: center;
    animation: voice-wave 1.2s ease-in-out infinite paused;

    &:nth-child(1) { animation-delay: -0s; }
    &:nth-child(2) { animation-delay: -0.12s; }
    &:nth-child(3) { animation-delay: -0.24s; }
    &:nth-child(4) { animation-delay: -0.36s; }
    &:nth-child(5) { animation-delay: -0.48s; }
    &:nth-child(6) { animation-delay: -0.6s; }
    &:nth-child(7) { animation-delay: -0.72s; }
    &:nth-child(8) { animation-delay: -0.84s; }
    &:nth-child(9) { animation-delay: -0.96s; }
    &:nth-child(10) { animation-delay: -1.08s; }
    &:nth-child(11) { animation-delay: -1.2s; }
  }

  &.is-playing .wave-bar {
    animation-play-state: running;
    background: linear-gradient(to top, var(--apple-brand) 0%, var(--apple-brand-hover) 100%);
  }
}

@keyframes voice-wave {
  0%, 100% { transform: scaleY(0.2); }
  50% { transform: scaleY(0.95); }
}

.card-info {
  flex: 1;
  min-width: 0;
  display: flex;
  gap: 12px;
}

.info-left {
  flex: 1;
  min-width: 0;
  display: flex;
  flex-direction: column;
  justify-content: space-evenly;
}

.info-right {
  display: flex;
  flex-direction: column;
  align-items: flex-end;
  gap: 10px;
  flex-shrink: 0;
  min-width: 100px;
}

.info-title {
  min-width: 0;
  font-size: 16px;
  font-weight: 600;
  color: var(--apple-text);

  .name-text {
    line-height: 28px;
    text-align: left;
    display: block;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
  }
}

.info-grid {
  display: flex;
  flex-direction: column;
  gap: 4px;
}

.info-row {
  display: flex;
  align-items: center;
  font-size: 13px;
  line-height: 1.5;
}

.info-label {
  margin-right: 10px;
  color: var(--apple-text-3);
  flex-shrink: 0;
}

.info-value {
  color: var(--apple-text);
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.card-actions {
  display: flex;
  align-items: center;
  flex-wrap: wrap;
  gap: 2px 4px;
  padding-top: 10px;
  border-top: 1px solid var(--apple-divider-soft);
}

::v-deep .card-action-btn.ui-btn--text {
  padding: 4px 10px;
  font-size: 13px;
  line-height: 1;
  border-radius: var(--apple-radius-sm);
  height: auto;
  min-height: 0;
  margin: 0;
  display: inline-flex;
  align-items: center;

  &:hover {
    background: var(--apple-brand-tint);
    color: var(--apple-brand-hover);
  }
}

.empty-state {
  grid-column: 1 / -1;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 60px 0;
  color: var(--apple-text-3);
  font-size: 14px;
}
</style>