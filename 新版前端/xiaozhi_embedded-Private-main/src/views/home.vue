<template>
  <div class="welcome">
    <HeaderBar :devices="devices" />
    <el-main class="welcome-main">
      <div class="welcome-container">
        <section class="welcome-hero" ref="heroRef">
          <div class="welcome-hero__content">
            <div class="welcome-titles" ref="titleRef">
              <h1 class="welcome-greeting">{{ $t('home.greeting') }}</h1>
              <p class="welcome-wish">{{ $t('home.wish') }}</p>
              <p class="welcome-hint">let's have a wonderful day!</p>
            </div>
            <div class="welcome-search-row" ref="searchRowRef">
            <div class="search-wrapper">
              <ui-input
                v-model="search"
                :placeholder="$t('header.searchPlaceholder')"
                clearable
                class="welcome-search-input"
                ref="searchInput"
                @keyup.enter="handleSearch"
                @clear="handleSearchReset"
                @focus="showSearchHistory"
                @blur="hideSearchHistory"
              >
                <template #prefix>
                  <i class="el-icon-search welcome-search-icon" @click="handleSearch"></i>
                </template>
              </ui-input>
              <div v-if="showHistory && searchHistory.length > 0" class="search-history-dropdown">
                <div class="search-history-header">
                  <span>{{ $t("header.searchHistory") }}</span>
                  <el-button type="text" size="small" class="clear-history-btn" @click="clearSearchHistory">
                    {{ $t("header.clearHistory") }}
                  </el-button>
                </div>
                <div class="search-history-list">
                  <div v-for="(item, index) in searchHistory" :key="index" class="search-history-item"
                    @click.stop="selectSearchHistory(item)">
                    <span class="history-text">{{ item }}</span>
                    <i class="el-icon-close clear-item-icon" @click.stop="removeSearchHistory(index)"></i>
                  </div>
                </div>
              </div>
            </div>
            <ui-button
              type="primary"
              size="large"
              round
              icon="el-icon-plus"
              class="welcome-add-btn"
              @click="showAddDialog"
            >
              {{ $t('home.addAgent') }}
            </ui-button>
            </div>
          </div>
          <div class="photo-card" ref="photoCardRef">
            <img
              v-show="!photoFailed"
              :src="homePhotoUrl"
              class="photo-card__img"
              alt="landscape"
              loading="lazy"
              @error="photoFailed = true"
            />
            <div class="photo-card__veil" aria-hidden="true"></div>
            <span v-show="!photoFailed" class="photo-card__caption">{{ $t('home.photoCaption') }}</span>
          </div>
        </section>

        <div class="llm-dist" v-if="llmDistribution.length" ref="llmDistRef">
          <span class="llm-dist__label">{{ $t('home.llmDistribution') }}</span>
          <div class="llm-dist__pills">
            <span v-for="(item, i) in llmDistribution" :key="i" class="llm-dist__pill" :title="item.name">
              {{ item.name }} × {{ item.count }}
            </span>
          </div>
        </div>

        <div class="device-list-container">
          <template v-if="isLoading">
            <div v-for="i in skeletonCount" :key="'skeleton-' + i" class="skeleton-item">
              <div class="skeleton-head">
                <div class="skeleton-image"></div>
                <div class="skeleton-content">
                  <div class="skeleton-line"></div>
                  <div class="skeleton-line-short"></div>
                </div>
              </div>
              <div class="skeleton-row">
                <div class="skeleton-pill"></div>
                <div class="skeleton-pill"></div>
                <div class="skeleton-pill skeleton-pill--short"></div>
              </div>
            </div>
          </template>

          <template v-else-if="devices.length">
            <DeviceItem v-for="(item, index) in devices" :key="index" :device="item" :feature-status="featureStatus"
              @configure="goToRoleConfig" @deviceManage="handleDeviceManage" @delete="handleDeleteAgent"
              @chat-history="handleShowChatHistory" />
          </template>

          <template v-else>
            <div class="empty-state" ref="emptyStateRef">
              <div class="empty-state__icon">
                <i class="el-icon-plus"></i>
              </div>
              <div class="empty-state__title">{{ $t('home.emptyTitle') }}</div>
              <div class="empty-state__desc">{{ $t('home.emptyDesc') }}</div>
              <ui-button type="primary" size="large" round icon="el-icon-plus" class="empty-state__btn" @click="showAddDialog">
                {{ $t('home.addAgent') }}
              </ui-button>
            </div>
          </template>
        </div>
      </div>

      <AddWisdomBodyDialog :visible.sync="addDeviceDialogVisible" @confirm="handleWisdomBodyAdded" />
      <el-dialog
        :visible.sync="deleteAgentDialogVisible"
        :close-on-click-modal="!isDeletingAgent"
        :close-on-press-escape="!isDeletingAgent"
        :show-close="!isDeletingAgent"
        width="520px"
        append-to-body
        class="delete-agent-dialog"
        @closed="resetDeleteAgentDialog"
      >
        <template slot="title">
          <div class="delete-agent-title">
            <img src="@/assets/knowledge-base/level.png" class="delete-agent-title-icon" />
            <span>{{ $t('home.deleteConfirmTitle') }}</span>
          </div>
        </template>
        <div class="delete-agent-content">
          <i class="el-icon-warning-outline delete-agent-warning"></i>
          <div class="delete-agent-message">
            <div class="delete-agent-copy-guard" @copy.prevent @cut.prevent @contextmenu.prevent>
              {{ $t('home.confirmDeleteAgent', { agentName: deleteTargetAgentName }) }}
            </div>
            <div class="delete-agent-target delete-agent-copy-guard" @copy.prevent @cut.prevent @contextmenu.prevent>
              {{ deleteTargetAgentName }}
            </div>
            <el-input
              ref="deleteAgentConfirmInput"
              v-model="deleteAgentConfirmText"
              class="delete-agent-input"
              :placeholder="$t('home.deleteAgentNamePlaceholder')"
              clearable
              @paste.native.prevent="handleDeleteAgentPaste"
              @drop.native.prevent="handleDeleteAgentPaste"
              @contextmenu.native.prevent
              @keyup.enter.native="confirmDeleteAgent"
            />
            <div v-if="deleteAgentConfirmText && !isDeleteAgentNameMatched" class="delete-agent-helper">
              {{ $t('home.deleteAgentNameMismatch') }}
            </div>
          </div>
        </div>
        <span slot="footer" class="delete-agent-footer">
          <el-button class="delete-agent-cancel" :disabled="isDeletingAgent" @click="closeDeleteAgentDialog">{{ $t('button.cancel') }}</el-button>
          <el-button
            class="delete-agent-confirm"
            type="primary"
            :loading="isDeletingAgent"
            :disabled="!isDeleteAgentNameMatched"
            @click="confirmDeleteAgent"
          >
            {{ $t('button.ok') }}
          </el-button>
        </span>
      </el-dialog>
    </el-main>
    <el-footer class="welcome-footer">
      <version-footer />
    </el-footer>
    <chat-history-dialog :visible.sync="showChatHistory" :agent-id="currentAgentId" :agent-name="currentAgentName" />
  </div>

</template>

<script>
import Api from '@/apis/api';
import { mapState } from "vuex";
import AddWisdomBodyDialog from '@/components/AddWisdomBodyDialog.vue';
import ChatHistoryDialog from '@/components/ChatHistoryDialog.vue';
import DeviceItem from '@/components/DeviceItem.vue';
import HeaderBar from '@/components/HeaderBar.vue';
import VersionFooter from '@/components/VersionFooter.vue';
import UiInput from '@/components/ui/UiInput.vue';
import UiButton from '@/components/ui/UiButton.vue';
import { gsap } from '@/utils/gsap';
import featureManager from '@/utils/featureManager';

export default {
  name: 'HomePage',
  components: { DeviceItem, AddWisdomBodyDialog, HeaderBar, VersionFooter, ChatHistoryDialog, UiInput, UiButton },
  data() {
    return {
      addDeviceDialogVisible: false,
      photoFailed: false,
      devices: [],
      originalDevices: [],
      isSearching: false,
      searchRegex: null,
      isLoading: true,
      skeletonCount: localStorage.getItem('skeletonCount') || 8,
      showChatHistory: false,
      currentAgentId: '',
      currentAgentName: '',
      featureStatus: {
        voiceprintRecognition: false,
        voiceClone: false,
        knowledgeBase: false
      },
      search: "",
      showHistory: false,
      searchHistory: [],
      deleteAgentDialogVisible: false,
      deleteTargetAgentId: '',
      deleteTargetAgentName: '',
      deleteAgentConfirmText: '',
      isDeletingAgent: false,
    }
  },

  computed: {
    ...mapState({
      userInfo: (state) => state.userInfo,
    }),
    isDeleteAgentNameMatched() {
      return !!this.deleteTargetAgentName && this.deleteAgentConfirmText === this.deleteTargetAgentName;
    },
    homePhotoUrl() {
      return (process.env.BASE_URL || '/') + 'images/home-landscape.jpg';
    },
    llmDistribution() {
      if (!this.devices.length) return [];
      const counts = {};
      this.devices.forEach((d) => {
        const key = d.llmModelName || this.$t('home.llmDistribution');
        counts[key] = (counts[key] || 0) + 1;
      });
      return Object.keys(counts)
        .map((name) => ({ name, count: counts[name] }))
        .sort((a, b) => b.count - a.count)
        .slice(0, 4);
    }
  },

  async mounted() {
    this.playEnterAnimation();
    this.fetchAgentList();
    await this.loadFeatureStatus();
    this.loadSearchHistory();
  },

  beforeDestroy() {
    this.killEnterAnimation();
  },

  methods: {
    async loadFeatureStatus() {
      await featureManager.waitForInitialization();
      const config = featureManager.getConfig();
      this.featureStatus = {
        voiceprintRecognition: config.voiceprintRecognition,
        voiceClone: config.voiceClone,
        knowledgeBase: config.knowledgeBase
      };
    },
    prefersReducedMotion() {
      if (typeof window === 'undefined' || !window.matchMedia) return false;
      return window.matchMedia('(prefers-reduced-motion: reduce)').matches;
    },

    showAddDialog() {
      this.addDeviceDialogVisible = true
    },
    goToRoleConfig() {
      this.$router.push('/role-config')
    },
    handleWisdomBodyAdded(res) {
      this.fetchAgentList();
      this.addDeviceDialogVisible = false;
    },
    handleDeviceManage() {
      this.$router.push('/device-management');
    },
    handleSearchReset() {
      this.isSearching = false;
      this.devices = [...this.originalDevices];
    },

    handleSearchResult(filteredList) {
      this.devices = filteredList;
    },
    fetchAgentList() {
      this.isLoading = true;
      Api.agent.getAgentList(({ data }) => {
        if (data?.data) {
          this.originalDevices = data.data.map(item => ({
            ...item,
            agentId: item.id
          })).sort((a, b) => this.compareAgentsByActivity(a, b));

          this.skeletonCount = Math.min(
            Math.max(this.originalDevices.length, 3),
            10
          );

          this.handleSearchReset();
        }
        this.isLoading = false;
        this.$nextTick(() => {
          this.playListEnter();
          this.updatePageAccents();
        });
      }, (error) => {
        console.error('Failed to fetch agent list:', error);
        this.isLoading = false;
      });
    },
    handleDeleteAgent(device) {
      const targetAgent = typeof device === 'object'
        ? device
        : this.devices.find((item) => item.agentId === device || item.id === device);
      const agentId = targetAgent?.agentId || targetAgent?.id;
      const agentName = targetAgent?.agentName || '';

      if (!agentId || !agentName) {
        this.$message.error(this.$t('home.deleteAgentMissingInfo'));
        return;
      }

      this.deleteTargetAgentId = agentId;
      this.deleteTargetAgentName = agentName;
      this.deleteAgentConfirmText = '';
      this.deleteAgentDialogVisible = true;
      this.$nextTick(() => {
        if (this.$refs.deleteAgentConfirmInput) {
          this.$refs.deleteAgentConfirmInput.focus();
        }
      });
    },
    handleDeleteAgentPaste() {
      this.$message.warning(this.$t('home.deleteAgentPasteForbidden'));
    },
    closeDeleteAgentDialog() {
      if (this.isDeletingAgent) return;
      this.deleteAgentDialogVisible = false;
    },
    resetDeleteAgentDialog() {
      this.deleteTargetAgentId = '';
      this.deleteTargetAgentName = '';
      this.deleteAgentConfirmText = '';
      this.isDeletingAgent = false;
    },
    confirmDeleteAgent() {
      if (!this.isDeleteAgentNameMatched || this.isDeletingAgent) return;

      this.isDeletingAgent = true;
      Api.agent.deleteAgent(this.deleteTargetAgentId, (res) => {
        this.isDeletingAgent = false;
        if (res.data.code === 0) {
          this.$message.success({
            message: this.$t('home.deleteSuccess'),
            showClose: true
          });
          this.deleteAgentDialogVisible = false;
          this.fetchAgentList();
        } else {
          this.$message.error({
            message: res.data.msg || this.$t('home.deleteFailed'),
            showClose: true
          });
        }
      });
    },
    handleShowChatHistory({ agentId, agentName }) {
      this.currentAgentId = agentId;
      this.currentAgentName = agentName;
      this.showChatHistory = true;
    },
    handleSearch() {
      const searchValue = this.search.trim();

      if (!searchValue) {
        this.handleSearchReset();
        return;
      }

      this.saveSearchHistory(searchValue);

      if (this.$refs.searchInput) {
        this.$refs.searchInput.blur();
      }

      this.isSearching = true;
      this.isLoading = true;
      const isMac = /^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$/.test(searchValue)
      const searchType = isMac ? 'mac' : 'name';
      Api.agent.searchAgent(searchValue, searchType, ({ data }) => {
        if (data?.data) {
          this.devices = data.data.map(item => ({
            ...item,
            agentId: item.id
          }));
        }
        this.isLoading = false;
        this.$nextTick(() => {
          this.playListEnter();
          this.updatePageAccents();
        });
      }, (error) => {
        console.error('搜索智能体失败:', error);
        this.isLoading = false;
        this.$message.error(this.$t('message.searchFailed'));
      });
    },

    showSearchHistory() {
      this.showHistory = true;
    },

    hideSearchHistory() {
      setTimeout(() => {
        this.showHistory = false;
      }, 200);
    },

    loadSearchHistory() {
      try {
        const history = localStorage.getItem(this.SEARCH_HISTORY_KEY);
        if (history) {
          this.searchHistory = JSON.parse(history);
        }
      } catch (error) {
        console.error("加载搜索历史失败:", error);
        this.searchHistory = [];
      }
    },

    saveSearchHistory(keyword) {
      if (!keyword || this.searchHistory.includes(keyword)) {
        return;
      }

      this.searchHistory.unshift(keyword);

      if (this.searchHistory.length > this.MAX_HISTORY_COUNT) {
        this.searchHistory = this.searchHistory.slice(0, this.MAX_HISTORY_COUNT);
      }

      try {
        localStorage.setItem(this.SEARCH_HISTORY_KEY, JSON.stringify(this.searchHistory));
      } catch (error) {
        console.error("保存搜索历史失败:", error);
      }
    },

    selectSearchHistory(keyword) {
      this.search = keyword;
      this.handleSearch();
    },

    removeSearchHistory(index) {
      this.searchHistory.splice(index, 1);
      try {
        localStorage.setItem(this.SEARCH_HISTORY_KEY, JSON.stringify(this.searchHistory));
      } catch (error) {
        console.error("更新搜索历史失败:", error);
      }
    },

    clearSearchHistory() {
      this.searchHistory = [];
      try {
        localStorage.removeItem(this.SEARCH_HISTORY_KEY);
      } catch (error) {
        console.error("清空搜索历史失败:", error);
      }
    },

    playEnterAnimation() {
      const title = this.$refs.titleRef;
      const hero = this.$refs.heroRef;
      const searchRow = this.$refs.searchRowRef;
      const greeting = this.$el ? this.$el.querySelector('.welcome-greeting') : null;
      const wishHint = this.$el ? this.$el.querySelectorAll('.welcome-wish, .welcome-hint') : [];

      this.killHeroTweens();
      const reducedMotion = this.prefersReducedMotion();

      if (greeting && !reducedMotion) {
        gsap.fromTo(
          greeting,
          { autoAlpha: 0, y: 18, filter: 'blur(10px)' },
          {
            autoAlpha: 1,
            y: 0,
            filter: 'blur(0px)',
            duration: 0.7,
            ease: 'power3.out',
            delay: 0.05,
            onComplete: () => gsap.set(greeting, { clearProps: 'transform' })
          }
        );
      } else if (greeting) {
        gsap.set(greeting, { autoAlpha: 1 });
      }

      const tailEnter = () => {
        if (wishHint && wishHint.length) {
          gsap.fromTo(
            wishHint,
            { autoAlpha: 0, y: 14 },
            {
              autoAlpha: 1,
              y: 0,
              duration: 0.5,
              ease: 'power3.out',
              stagger: 0.06,
              onComplete: () => gsap.set(wishHint, { clearProps: 'transform' })
            }
          );
        }
        if (hero) {
          gsap.fromTo(
            hero,
            { autoAlpha: 0, y: -8 },
            { autoAlpha: 1, y: 0, duration: 0.55, ease: 'power3.out' }
          );
        }
        if (searchRow) {
          gsap.fromTo(
            searchRow,
            { autoAlpha: 0, y: -10 },
            { autoAlpha: 1, y: 0, duration: 0.5, ease: 'power3.out', delay: 0.08 }
          );
        }
        const photoCard = this.$refs.photoCardRef;
        if (photoCard) {
          gsap.fromTo(
            photoCard,
            { autoAlpha: 0, x: 28, scale: 0.96 },
            {
              autoAlpha: 1,
              x: 0,
              scale: 1,
              duration: 0.7,
              ease: 'power3.out',
              delay: 0.12,
              onComplete: () => gsap.set(photoCard, { clearProps: 'transform' })
            }
          );
        }
      };

      if (title) {
        gsap.set(title, { clearProps: 'transform,filter' });
      }

      if (greeting && !reducedMotion) {
        gsap.delayedCall(0.7, tailEnter);
      } else {
        tailEnter();
      }
    },

    playListEnter() {
      if (this.isLoading) return;
      const items = this.$el ? this.$el.querySelectorAll('.device-item') : [];
      if (!items || items.length === 0) return;
      gsap.killTweensOf(items);
      gsap.fromTo(
        items,
        { autoAlpha: 0, y: 24 },
        {
          autoAlpha: 1,
          y: 0,
          duration: 0.55,
          ease: 'power3.out',
          stagger: 0.06,
          delay: 0.05,
          onComplete: () => {
            gsap.set(items, { clearProps: 'transform' });
          }
        }
      );
    },

    killHeroTweens() {
      const refs = ['titleRef', 'heroRef', 'searchRowRef', 'photoCardRef'];
      refs.forEach((key) => {
        const el = this.$refs[key];
        if (el) gsap.killTweensOf(el);
      });
    },

    killEnterAnimation() {
      this.killHeroTweens();
      if (this.$el) {
        const items = this.$el.querySelectorAll('.device-item');
        gsap.killTweensOf(items);
        const photoCard = this.$el.querySelectorAll('.photo-card');
        gsap.killTweensOf(photoCard);
        const llmDist = this.$el.querySelectorAll('.llm-dist');
        gsap.killTweensOf(llmDist);
        const llmPills = this.$el.querySelectorAll('.llm-dist__pill');
        gsap.killTweensOf(llmPills);
        const emptyState = this.$el.querySelectorAll('.empty-state');
        gsap.killTweensOf(emptyState);
      }
    },

    compareAgentsByActivity(a, b) {
      const aTime = a && a.lastConnectedAt ? new Date(a.lastConnectedAt).getTime() : NaN;
      const bTime = b && b.lastConnectedAt ? new Date(b.lastConnectedAt).getTime() : NaN;
      const aValid = Number.isFinite(aTime);
      const bValid = Number.isFinite(bTime);
      if (aValid && bValid) return bTime - aTime;
      if (aValid) return -1;
      if (bValid) return 1;
      return 0;
    },

    updatePageAccents() {
      const reducedMotion = this.prefersReducedMotion();
      const tweens = [];
      this.updateLlmDist(reducedMotion, tweens);
      this.updateEmptyState(reducedMotion, tweens);
      return tweens;
    },

    updateLlmDist(reducedMotion, tweens) {
      const root = this.$refs.llmDistRef;
      if (!this.llmDistribution.length || !root) return;
      if (reducedMotion) {
        gsap.set(root, { autoAlpha: 1 });
        return;
      }
      gsap.killTweensOf(root);
      gsap.set(root, { autoAlpha: 0, y: 8 });
      tweens.push(
        gsap.to(root, {
          autoAlpha: 1,
          y: 0,
          duration: 0.4,
          ease: 'power3.out',
          delay: 0.42,
          onComplete: () => gsap.set(root, { clearProps: 'transform' })
        })
      );
    },

    updateEmptyState(reducedMotion, tweens) {
      const root = this.$refs.emptyStateRef;
      if (!root || this.isLoading || this.devices.length) return;
      if (reducedMotion) {
        gsap.set(root, { autoAlpha: 1 });
        return;
      }
      gsap.killTweensOf(root);
      gsap.set(root, { autoAlpha: 0 });
      tweens.push(
        gsap.to(root, {
          autoAlpha: 1,
          duration: 0.45,
          ease: 'power3.out',
          delay: 0.15,
          onComplete: () => gsap.set(root, { clearProps: 'opacity,visibility' })
        })
      );
    },
  }
}
</script>

<style lang="scss" scoped>
@import '@/styles/tokens.scss';

.welcome {
  min-width: 900px;
  min-height: 506px;
  height: 100vh;
  display: flex;
  flex-direction: column;
  background: var(--apple-bg);
  background-size: cover;
  background-position: center;
  -webkit-background-size: cover;
  -o-background-size: cover;
}

.welcome-main {
  position: relative;
  padding: 0 !important;
  display: flex;
  flex-direction: column;
  flex: 1 1 auto;
  overflow-x: hidden;
}

.welcome-container {
  position: relative;
  z-index: 1;
  width: 100%;
  max-width: var(--apple-container-max);
  margin: 0 auto;
  padding: 0 var(--apple-container-padding-x);
  box-sizing: border-box;
}

.welcome-hero {
  padding: 28px 0 20px;
  display: grid;
  grid-template-columns: minmax(0, 1fr) minmax(320px, 440px);
  gap: 48px;
  align-items: center;

  &__content {
    display: flex;
    flex-direction: column;
    gap: 20px;
    min-width: 0;
  }
}

.welcome-titles {
  display: flex;
  flex-direction: column;
  gap: 6px;
  text-align: left;
}

.welcome-greeting {
  margin: 0;
  font-family: $apple-font-family;
  font-size: var(--apple-font-size-2xl);
  font-weight: var(--apple-font-weight-bold);
  color: var(--apple-text);
  line-height: var(--apple-line-height-tight);
  letter-spacing: -0.02em;
}

.welcome-wish {
  margin: 4px 0 0;
  font-family: $apple-font-family;
  font-size: var(--apple-font-size-lg);
  font-weight: var(--apple-font-weight-medium);
  color: var(--apple-text-2);
  line-height: 1.45;
}

.welcome-hint {
  margin: 4px 0 0;
  font-family: $apple-font-family;
  font-size: 13px;
  font-weight: var(--apple-font-weight-regular);
  color: var(--apple-text-3);
  line-height: 1.5;
  letter-spacing: 0.04em;
}

.welcome-search-row {
  display: flex;
  align-items: center;
  flex-wrap: wrap;
  gap: 12px;
}

.search-wrapper {
  position: relative;
  flex: 0 1 auto;
}

.welcome-add-btn {
  flex: 0 0 auto;
}

.photo-card {
  position: relative;
  width: min(100%, 460px);
  aspect-ratio: 16 / 10;
  align-self: center;
  border-radius: var(--apple-radius-2xl);
  overflow: hidden;
  box-shadow: var(--apple-shadow-raised);
  background: var(--apple-gradient-brand);
  will-change: transform;
  cursor: default;

  &__img {
    position: absolute;
    inset: 0;
    width: 100%;
    height: 100%;
    object-fit: cover;
    object-position: center;
    display: block;
    transition: transform var(--apple-duration-slower) var(--apple-ease-in-out);
  }

  &__veil {
    position: absolute;
    inset: 0;
    background: linear-gradient(180deg, rgba(29, 29, 31, 0) 52%, rgba(29, 29, 31, 0.42) 100%);
    pointer-events: none;
  }

  &__caption {
    position: absolute;
    left: 20px;
    bottom: 16px;
    z-index: 1;
    font-family: $apple-font-family;
    font-size: 13px;
    font-weight: var(--apple-font-weight-medium);
    color: rgba(255, 255, 255, 0.92);
    letter-spacing: 0.02em;
    text-align: left;
    text-overflow: ellipsis;
    white-space: nowrap;
    overflow: hidden;
    max-width: calc(100% - 40px);
  }

  &:hover {
    .photo-card__img {
      transform: scale(1.04);
    }
  }
}

::v-deep .welcome-search-input.ui-input {
  width: 400px;
  max-width: 100%;
  height: 44px;
  padding: 0 6px 0 0;
  background-color: var(--apple-surface);
  border: 1px solid var(--apple-divider-soft);
  border-radius: var(--apple-radius-full);
  transition:
    background-color var(--apple-duration-fast) var(--apple-ease-standard),
    border-color var(--apple-duration-fast) var(--apple-ease-standard),
    box-shadow var(--apple-duration-fast) var(--apple-ease-standard);

  &:hover:not(.is-disabled):not(.is-readonly) {
    background-color: var(--apple-surface);
    border-color: var(--apple-divider);
  }

  &.is-focus {
    background-color: var(--apple-surface);
    border-color: var(--apple-brand);
    box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.18);
  }
}

::v-deep .welcome-search-input .ui-input__inner {
  height: 100%;
  font-size: var(--apple-font-size-base);
  color: var(--apple-text);
  padding: 0 12px;
}

::v-deep .welcome-search-input .ui-input__prefix {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  padding: 0 4px 0 16px;
  color: var(--apple-text-3);
  cursor: pointer;
}

::v-deep .welcome-search-input .ui-input__clear {
  margin-right: 8px;
  color: var(--apple-text-3);
}

.welcome-search-icon {
  font-size: 16px;
  pointer-events: auto;
  transition: color var(--apple-duration-fast) var(--apple-ease-standard);

  &:hover {
    color: var(--apple-brand);
  }
}

.search-history-dropdown {
  position: absolute;
  top: calc(100% + 8px);
  left: 0;
  right: 0;
  background: var(--apple-surface);
  border: 1px solid var(--apple-divider-soft);
  border-radius: var(--apple-radius-lg);
  box-shadow: var(--apple-shadow-overlay);
  z-index: var(--apple-z-dropdown);
  overflow: hidden;
}

.search-history-header {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 10px 14px;
  border-bottom: 1px solid var(--apple-divider-soft);
  font-size: var(--apple-font-size-xs);
  color: var(--apple-text-3);
  font-weight: var(--apple-font-weight-medium);
  letter-spacing: 0.02em;
}

.clear-history-btn {
  color: var(--apple-text-3);
  font-size: var(--apple-font-size-xs);
  padding: 0;
  height: auto;

  &:hover {
    color: var(--apple-brand);
  }
}

.search-history-list {
  max-height: 220px;
  overflow-y: auto;
}

.search-history-item {
  display: flex;
  justify-content: space-between;
  align-items: center;
  padding: 10px 14px;
  cursor: pointer;
  font-size: var(--apple-font-size-sm);
  color: var(--apple-text);
  transition: background-color var(--apple-duration-fast) var(--apple-ease-standard);

  &:hover {
    background-color: var(--apple-surface-2);

    .clear-item-icon {
      visibility: visible;
    }
  }
}

.history-text {
  text-align: left;
  flex: 1;
  min-width: 0;
  overflow: hidden;
  text-overflow: ellipsis;
  white-space: nowrap;
}

.clear-item-icon {
  font-size: 12px;
  color: var(--apple-text-3);
  visibility: hidden;
  transition: color var(--apple-duration-fast) var(--apple-ease-standard);
  cursor: pointer;

  &:hover {
    color: var(--apple-brand);
  }
}

.device-list-container {
  display: grid;
  grid-template-columns: repeat(auto-fill, minmax(320px, 1fr));
  gap: 24px;
  padding: 20px 0 40px;
}

.device-item {
  margin: 0 !important;
  width: auto !important;
}

.llm-dist {
  display: flex;
  align-items: center;
  gap: $apple-space-3;
  padding: $apple-space-4 0 0;
  flex-wrap: wrap;

  &__label {
    font-size: 13px;
    font-weight: var(--apple-font-weight-medium);
    color: var(--apple-text-2);
    flex-shrink: 0;
    letter-spacing: 0.01em;
  }

  &__pills {
    display: inline-flex;
    align-items: center;
    gap: $apple-space-2;
    flex-wrap: wrap;
    min-width: 0;
  }

  &__pill {
    display: inline-flex;
    align-items: center;
    max-width: 160px;
    padding: 4px 10px;
    background: var(--apple-surface-2);
    color: var(--apple-text-2);
    border-radius: var(--apple-radius-full);
    font-size: 12px;
    font-weight: var(--apple-font-weight-medium);
    line-height: 1.4;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
    transition:
      background-color var(--apple-duration-fast) var(--apple-ease-standard),
      color var(--apple-duration-fast) var(--apple-ease-standard);
    cursor: default;

    &:hover {
      background: var(--apple-divider-strong);
      color: var(--apple-text);
    }
  }
}

.empty-state {
  grid-column: 1 / -1;
  background: var(--apple-surface);
  border-radius: var(--apple-radius-xl);
  box-shadow: var(--apple-shadow-card);
  padding: 64px 32px;
  display: flex;
  flex-direction: column;
  align-items: center;
  justify-content: center;
  gap: $apple-space-3;
  text-align: center;
  font-family: $apple-font-family;

  &__icon {
    width: 64px;
    height: 64px;
    border-radius: 50%;
    background: var(--apple-brand-tint);
    color: var(--apple-brand);
    display: inline-flex;
    align-items: center;
    justify-content: center;
    font-size: 28px;
    margin-bottom: $apple-space-2;
  }

  &__title {
    font-size: 17px;
    font-weight: var(--apple-font-weight-semibold);
    color: var(--apple-text);
    line-height: 1.3;
    letter-spacing: -0.01em;
  }

  &__desc {
    font-size: 13px;
    font-weight: var(--apple-font-weight-regular);
    color: var(--apple-text-3);
    line-height: 1.5;
    max-width: 360px;
  }

  &__btn {
    margin-top: $apple-space-4;
    min-width: 140px;
  }
}

.welcome-footer {
  padding: 6px 0 14px !important;
  text-align: center;
}

::v-deep .welcome-footer .copyright,
::v-deep .welcome-footer .beian-text,
::v-deep .welcome-footer .footer-content {
  color: var(--apple-text-3);
  font-size: var(--apple-font-size-xs);
}

@keyframes welcome-shimmer {
  100% {
    transform: translateX(220%);
  }
}

.skeleton-item {
  background: var(--apple-surface);
  border-radius: var(--apple-radius-xl);
  padding: 24px;
  height: 168px;
  position: relative;
  overflow: hidden;
  box-shadow: var(--apple-shadow-card);
  display: flex;
  flex-direction: column;
  gap: 18px;
  box-sizing: border-box;
}

.skeleton-head {
  display: flex;
  align-items: center;
  gap: 14px;
}

.skeleton-image {
  width: 52px;
  height: 52px;
  background: var(--apple-surface-2);
  border-radius: var(--apple-radius-md);
  position: relative;
  overflow: hidden;
  flex-shrink: 0;
}

.skeleton-content {
  flex: 1;
  min-width: 0;
}

.skeleton-line {
  height: 14px;
  background: var(--apple-surface-2);
  border-radius: var(--apple-radius-sm);
  margin-bottom: 10px;
  width: 70%;
  position: relative;
  overflow: hidden;
}

.skeleton-line-short {
  height: 12px;
  background: var(--apple-surface-2);
  border-radius: var(--apple-radius-sm);
  width: 50%;
  position: relative;
  overflow: hidden;
}

.skeleton-row {
  display: flex;
  gap: 10px;
  margin-top: auto;
}

.skeleton-pill {
  height: 22px;
  width: 92px;
  background: var(--apple-surface-2);
  border-radius: var(--apple-radius-full);
  position: relative;
  overflow: hidden;

  &--short {
    width: 64px;
  }
}

.skeleton-image::after,
.skeleton-line::after,
.skeleton-line-short::after,
.skeleton-pill::after {
  content: '';
  position: absolute;
  top: 0;
  left: 0;
  width: 60%;
  height: 100%;
  background: linear-gradient(
    90deg,
    rgba(255, 255, 255, 0) 0%,
    rgba(255, 255, 255, 0.65) 50%,
    rgba(255, 255, 255, 0) 100%
  );
  animation: welcome-shimmer 1.4s ease-in-out infinite;
}

.delete-agent-content {
  display: flex;
  gap: 12px;
  align-items: flex-start;
}

.delete-agent-title {
  display: inline-flex;
  align-items: center;
  font-size: var(--apple-font-size-md);
  font-weight: var(--apple-font-weight-semibold);
  color: var(--apple-text);
  letter-spacing: -0.01em;
}

.delete-agent-title-icon {
  width: 24px;
  height: 24px;
  margin-right: 8px;
}

.delete-agent-warning {
  color: var(--apple-warning);
  font-size: 24px;
  margin-top: 4px;
}

.delete-agent-message {
  flex: 1;
  color: var(--apple-text-2);
  font-size: var(--apple-font-size-sm);
  line-height: var(--apple-line-height-normal);
}

.delete-agent-target {
  margin-top: 14px;
  padding: 10px 12px;
  border: 1px solid var(--apple-divider-soft);
  border-radius: var(--apple-radius-md);
  background: var(--apple-surface-2);
  color: var(--apple-text);
  font-weight: var(--apple-font-weight-semibold);
  word-break: break-all;
  font-size: var(--apple-font-size-sm);
}

.delete-agent-copy-guard {
  user-select: none;
  -webkit-user-select: none;
}

.delete-agent-input {
  margin-top: 16px;
}

::v-deep .delete-agent-input .el-input__inner {
  height: 42px;
  border-color: var(--apple-divider);
  border-radius: var(--apple-radius-md);
  background: var(--apple-surface);
  color: var(--apple-text);
  font-size: var(--apple-font-size-sm);
  transition:
    border-color var(--apple-duration-fast) var(--apple-ease-standard),
    box-shadow var(--apple-duration-fast) var(--apple-ease-standard);
}

::v-deep .delete-agent-input .el-input.is-focus .el-input__inner {
  border-color: var(--apple-brand);
  box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.16);
}

.delete-agent-helper {
  min-height: 18px;
  margin-top: 6px;
  color: var(--apple-danger);
  font-size: var(--apple-font-size-xs);
}

.delete-agent-footer {
  display: inline-flex;
  gap: 10px;
}

::v-deep .delete-agent-dialog .el-dialog {
  border-radius: var(--apple-radius-xl);
  overflow: hidden;
  box-shadow: var(--apple-shadow-overlay);
}

::v-deep .delete-agent-dialog .el-dialog__header {
  padding: 18px 24px 14px;
  background: var(--apple-surface);
  text-align: left;
  border-bottom: 1px solid var(--apple-divider-soft);
}

::v-deep .delete-agent-dialog .el-dialog__headerbtn {
  top: 16px;
  right: 18px;
  width: 32px;
  height: 32px;
  border: none;
  border-radius: 50%;
  background: var(--apple-surface-2);
  display: flex;
  align-items: center;
  justify-content: center;
  transition:
    background-color var(--apple-duration-fast) var(--apple-ease-standard),
    color var(--apple-duration-fast) var(--apple-ease-standard);
}

::v-deep .delete-agent-dialog .el-dialog__headerbtn .el-dialog__close {
  font-size: 16px;
  color: var(--apple-text-2);
  position: static;
  transform: none;
  line-height: 1;
}

::v-deep .delete-agent-dialog .el-dialog__headerbtn:hover {
  background: var(--apple-brand-tint);

  .el-dialog__close {
    color: var(--apple-brand);
  }
}

::v-deep .delete-agent-dialog .el-dialog__body {
  padding: 26px 28px 18px;
  background: var(--apple-surface);
}

::v-deep .delete-agent-dialog .el-dialog__footer {
  padding: 12px 28px 22px;
  background: var(--apple-surface);
}

::v-deep .delete-agent-cancel,
::v-deep .delete-agent-confirm {
  min-width: 96px;
  height: 40px;
  padding: 0 22px;
  border-radius: var(--apple-radius-md);
  font-size: var(--apple-font-size-sm);
  font-weight: var(--apple-font-weight-medium);
}

::v-deep .delete-agent-cancel {
  color: var(--apple-text);
  background: var(--apple-surface-2);
  border: 1px solid var(--apple-divider);
  transition:
    background-color var(--apple-duration-fast) var(--apple-ease-standard),
    color var(--apple-duration-fast) var(--apple-ease-standard);

  &:hover,
  &:focus {
    color: var(--apple-text);
    background: var(--apple-surface);
    border-color: var(--apple-divider);
  }
}

::v-deep .delete-agent-confirm {
  background: var(--apple-danger);
  border-color: var(--apple-danger);

  &:hover,
  &:focus {
    background: #ff4f46;
    border-color: #ff4f46;
  }
}

::v-deep .delete-agent-confirm.is-disabled,
::v-deep .delete-agent-confirm.is-disabled:hover,
::v-deep .delete-agent-confirm.is-disabled:focus {
  background: var(--apple-danger);
  border: none;
  opacity: 0.45;
}

@media (max-width: 880px) {
  .welcome-container {
    padding: 0 24px;
  }

  .welcome-hero {
    grid-template-columns: 1fr;
    gap: 20px;
  }

  .photo-card {
    width: 100%;
    max-width: none;
    justify-self: stretch;
  }

  .device-list-container {
    grid-template-columns: 1fr;
    gap: 20px;
  }

  .welcome-greeting {
    font-size: var(--apple-font-size-xl);
  }

  .empty-state {
    padding: 48px 24px;
  }
}
</style>
