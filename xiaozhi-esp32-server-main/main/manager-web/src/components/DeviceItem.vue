<template>
  <article class="device-card device-item" @mousemove="handleGlareMove" @mouseleave="handleGlareLeave">
    <header class="device-card__head">
      <div class="device-card__identity">
        <div class="device-card__avatar">
          <img v-if="avatarUrl" :src="avatarUrl" :alt="device.agentName" class="device-card__avatar-img" loading="lazy" />
          <span v-else class="device-card__avatar-text">{{ avatarInitial }}</span>
          <span class="device-card__status" :class="{ 'device-card__status--online': isOnline }" aria-hidden="true"></span>
        </div>
        <el-tooltip :content="device.agentName" placement="top" effect="light">
          <h3 class="device-card__name">{{ device.agentName }}</h3>
        </el-tooltip>
      </div>
      <div class="device-card__head-actions">
        <el-tooltip class="item" effect="light" :content="device.systemPrompt || device.agentName" placement="top"
          popper-class="device-item-tooltip">
          <button type="button" class="device-card__icon-btn">
            <i class="el-icon-info"></i>
          </button>
        </el-tooltip>
        <button type="button" class="device-card__icon-btn device-card__icon-btn--danger"
          @click.stop="handleDelete">
          <i class="el-icon-delete"></i>
        </button>
      </div>
    </header>

    <ul class="device-card__meta">
      <li class="device-card__meta-item">
        <span class="device-card__meta-label">{{ $t('home.languageModel') }}</span>
        <span class="device-card__meta-value">{{ device.llmModelName }}</span>
      </li>
      <li class="device-card__meta-item">
        <span class="device-card__meta-label">{{ $t('home.voiceModel') }}</span>
        <span class="device-card__meta-value">{{ device.ttsModelName }} <span class="device-card__meta-voice">({{ device.ttsVoiceName }})</span></span>
      </li>
    </ul>

    <div class="device-card__actions">
      <ui-button type="primary" size="small" round class="device-card__action device-card__action--primary"
        @click="handleConfigure">
        {{ $t('home.configureRole') }}
      </ui-button>
      <ui-button v-if="featureStatus.voiceprintRecognition" type="default" size="small" round
        class="device-card__action" @click="handleVoicePrint">
        {{ $t('home.voiceprintRecognition') }}
      </ui-button>
      <ui-button type="default" size="small" round class="device-card__action" @click="handleDeviceManage">
        {{ $t('home.deviceManagement') }} ({{ device.deviceCount }})
      </ui-button>
      <el-tooltip v-if="device.memModelId === 'Memory_nomem'" effect="light" :content="$t('home.enableMemory')"
        placement="top">
        <ui-button type="default" size="small" round
          class="device-card__action device-card__action--inactive"
          @click="handleChatHistory">
          {{ $t('home.chatHistory') }}
        </ui-button>
      </el-tooltip>
      <ui-button v-else type="default" size="small" round class="device-card__action"
        @click="handleChatHistory">
        {{ $t('home.chatHistory') }}
      </ui-button>
    </div>

    <footer class="device-card__foot">
      <div class="device-card__time">
        <span class="device-card__time-label">{{ $t('home.lastConversation') }}</span>
        <span class="device-card__time-value">{{ formattedLastConnectedTime }}</span>
      </div>
      <el-tooltip :content="tags.join()" placement="top" effect="light">
        <div class="device-card__tags">
          <template v-if="tags.length">
            <span v-for="(tag, idx) in displayTags" :key="idx" class="device-card__tag">{{ tag }}</span>
            <span v-if="tags.length > displayTags.length" class="device-card__tag device-card__tag--more">+{{ tags.length - displayTags.length }}</span>
          </template>
        </div>
      </el-tooltip>
    </footer>
  </article>
</template>

<script>
import i18n from '@/i18n';
import UiButton from '@/components/ui/UiButton.vue';

export default {
  name: 'DeviceItem',
  components: { UiButton },
  props: {
    device: { type: Object, required: true },
    featureStatus: {
      type: Object,
      default: () => ({
        voiceprintRecognition: false,
        voiceClone: false,
        knowledgeBase: false
      })
    }
  },
  data() {
    return { switchValue: false }
  },
  computed: {
    formattedLastConnectedTime() {
      if (!this.device.lastConnectedAt) return this.$t('home.noConversation');

      const lastTime = new Date(this.device.lastConnectedAt);
      if (isNaN(lastTime.getTime())) return this.$t('home.noConversation');
      const now = new Date();
      const diffMinutes = Math.floor((now - lastTime) / (1000 * 60));

      if (diffMinutes <= 1) {
        return this.$t('home.justNow');
      } else if (diffMinutes < 60) {
        return this.$t('home.minutesAgo', { minutes: diffMinutes });
      } else if (diffMinutes < 24 * 60) {
        const hours = Math.floor(diffMinutes / 60);
        const minutes = diffMinutes % 60;
        return this.$t('home.hoursAgo', { hours, minutes });
      } else {
        return this.device.lastConnectedAt;
      }
    },
    tags() {
      if (!this.device.tags) return [];
      return this.device.tags.map((tag) => tag.tagName);
    },
    displayTags() {
      return this.tags.slice(0, 3);
    },
    avatarInitial() {
      const name = (this.device && this.device.agentName) || '?';
      if (!name) return '?';
      return String(name).trim().charAt(0).toUpperCase();
    },
    avatarUrl() {
      const source = this.device && (
        this.device.avatarUrl ||
        this.device.avatar ||
        this.device.headImageUrl ||
        this.device.imageUrl ||
        this.device.photoUrl
      );
      if (!source || typeof source !== 'string') return '';
      return /^(https?:)?\/\//i.test(source.trim()) ? source.trim() : '';
    },
    isOnline() {
      if (!this.device || !this.device.lastConnectedAt) return false;
      const t = new Date(this.device.lastConnectedAt).getTime();
      if (isNaN(t)) return false;
      return Date.now() - t < 5 * 60 * 1000;
    }
  },
  methods: {
    prefersReducedMotion() {
      if (typeof window === 'undefined' || !window.matchMedia) return false;
      return window.matchMedia('(prefers-reduced-motion: reduce)').matches;
    },
    handleGlareMove(event) {
      const card = this.$el;
      if (!card || this.prefersReducedMotion()) return;
      const rect = card.getBoundingClientRect();
      const x = event.clientX - rect.left;
      const y = event.clientY - rect.top;
      card.style.setProperty('--glare-x', `${x}px`);
      card.style.setProperty('--glare-y', `${y}px`);
    },
    handleGlareLeave() {
      const card = this.$el;
      if (!card) return;
      card.style.removeProperty('--glare-x');
      card.style.removeProperty('--glare-y');
    },
    handleDelete() {
      this.$emit('delete', this.device)
    },
    handleConfigure() {
      this.$router.push({ path: '/role-config', query: { agentId: this.device.agentId } });
    },
    handleVoicePrint() {
      this.$router.push({ path: '/voice-print', query: { agentId: this.device.agentId } });
    },
    handleDeviceManage() {
      this.$router.push({ path: '/device-management', query: { agentId: this.device.agentId } });
    },
    handleChatHistory() {
      if (this.device.memModelId === 'Memory_nomem') {
        return
      }
      this.$emit('chat-history', { agentId: this.device.agentId, agentName: this.device.agentName })
    }
  },
}
</script>
<style lang="scss" scoped>
@import '@/styles/tokens.scss';

.device-card {
  position: relative;
  width: 100%;
  background: var(--apple-surface);
  border-radius: var(--apple-radius-xl);
  padding: 24px;
  box-sizing: border-box;
  box-shadow: var(--apple-shadow-card);
  font-family: $apple-font-family;
  display: flex;
  flex-direction: column;
  gap: 18px;
  transition:
    transform var(--apple-duration-normal) var(--apple-ease-standard),
    box-shadow var(--apple-duration-normal) var(--apple-ease-standard);
  will-change: transform;
  overflow: hidden;

  &::before {
    content: '';
    position: absolute;
    inset: 0;
    z-index: 2;
    pointer-events: none;
    opacity: 0;
    background: radial-gradient(
      340px circle at var(--glare-x, 50%) var(--glare-y, 50%),
      rgba(255, 255, 255, 0.45) 0%,
      rgba(255, 255, 255, 0) 70%
    );
    transition: opacity var(--apple-duration-normal) var(--apple-ease-standard);
  }

  &::after {
    content: '';
    position: absolute;
    top: 0;
    left: 0;
    width: 60%;
    height: 100%;
    background: linear-gradient(
      105deg,
      rgba(255, 255, 255, 0) 0%,
      rgba(255, 255, 255, 0.55) 50%,
      rgba(255, 255, 255, 0) 100%
    );
    transform: translateX(-120%);
    pointer-events: none;
    transition: transform 0.9s var(--apple-ease-standard);
  }

  &:hover {
    transform: translateY(-4px);
    box-shadow: var(--apple-shadow-raised);

    &::before {
      opacity: 1;
    }

    &::after {
      transform: translateX(220%);
    }
  }

  &__head {
    display: flex;
    justify-content: space-between;
    align-items: flex-start;
    gap: 12px;
  }

  &__identity {
    display: flex;
    align-items: center;
    gap: 12px;
    flex: 1;
    min-width: 0;
  }

  &__avatar {
    position: relative;
    width: 46px;
    height: 46px;
    border-radius: var(--apple-radius-md);
    background: var(--apple-gradient-brand);
    display: inline-flex;
    align-items: center;
    justify-content: center;
    color: #ffffff;
    font-size: var(--apple-font-size-md);
    font-weight: var(--apple-font-weight-semibold);
    flex-shrink: 0;
    overflow: visible;
  }

  &__avatar-text {
    line-height: 1;
  }

  &__avatar-img {
    width: 100%;
    height: 100%;
    border-radius: inherit;
    object-fit: cover;
    display: block;
  }

  &__status {
    position: absolute;
    right: -2px;
    bottom: -2px;
    width: 13px;
    height: 13px;
    border-radius: 50%;
    background: var(--apple-text-disabled);
    border: 2px solid var(--apple-surface);
    box-sizing: border-box;
    transition: background-color var(--apple-duration-fast) var(--apple-ease-standard);

    &--online {
      background: var(--apple-success);
      animation: device-status-breathe 2.4s ease-in-out infinite;
    }
  }

  &__name {
    margin: 0;
    font-size: var(--apple-font-size-md);
    font-weight: var(--apple-font-weight-semibold);
    color: var(--apple-text);
    letter-spacing: -0.01em;
    text-align: left;
    text-overflow: ellipsis;
    white-space: nowrap;
    overflow: hidden;
    line-height: 1.3;
  }

  &__head-actions {
    display: inline-flex;
    align-items: center;
    gap: 4px;
    flex-shrink: 0;
  }

  &__icon-btn {
    width: 30px;
    height: 30px;
    border-radius: var(--apple-radius-sm);
    background-color: transparent;
    border: none;
    color: var(--apple-text-3);
    display: inline-flex;
    align-items: center;
    justify-content: center;
    cursor: pointer;
    padding: 0;
    transition:
      background-color var(--apple-duration-fast) var(--apple-ease-standard),
      color var(--apple-duration-fast) var(--apple-ease-standard),
      transform var(--apple-duration-fast) var(--apple-ease-standard);

    &:hover {
      background-color: var(--apple-surface-2);
      color: var(--apple-text);
    }

    &:active {
      transform: scale(0.96);
    }

    i {
      font-size: 15px;
      line-height: 1;
    }

    &--danger {
      &:hover {
        background-color: rgba(255, 59, 48, 0.10);
        color: var(--apple-danger);
      }
    }
  }

  &__meta {
    list-style: none;
    margin: 0;
    padding: 0;
    display: flex;
    flex-direction: column;
    gap: 6px;
  }

  &__meta-item {
    display: flex;
    align-items: baseline;
    gap: 8px;
    font-size: var(--apple-font-size-sm);
    color: var(--apple-text-2);
    line-height: 1.5;
    min-width: 0;
  }

  &__meta-label {
    flex: 0 0 auto;
    color: var(--apple-text-3);
    font-weight: var(--apple-font-weight-medium);
    letter-spacing: 0.01em;
  }

  &__meta-value {
    flex: 1 1 auto;
    min-width: 0;
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
    color: var(--apple-text);
  }

  &__meta-voice {
    color: var(--apple-text-2);
    font-size: var(--apple-font-size-xs);
  }

  &__actions {
    display: grid;
    grid-template-columns: repeat(2, 1fr);
    gap: 8px;

    > :last-child:nth-child(odd) {
      grid-column: span 2;
    }
  }

  &__foot {
    display: flex;
    flex-wrap: wrap;
    justify-content: space-between;
    align-items: center;
    gap: 8px 12px;
    padding-top: 14px;
    border-top: 1px solid var(--apple-divider-soft);
    font-size: var(--apple-font-size-xs);
    color: var(--apple-text-3);
  }

  &__time {
    display: inline-flex;
    align-items: baseline;
    gap: 6px;
    flex: 1 1 0;
    min-width: 0;
  }

  &__time-label {
    color: var(--apple-text-3);
    flex-shrink: 0;
  }

  &__time-value {
    color: var(--apple-text-2);
    font-weight: var(--apple-font-weight-medium);
    flex: 1 1 0;
    min-width: 0;
    white-space: nowrap;
    overflow: hidden;
    text-overflow: ellipsis;
  }

  &__tags {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    flex-wrap: nowrap;
    overflow: hidden;
    max-width: 60%;
    flex-shrink: 1;
    min-width: 0;
    justify-content: flex-end;
  }

  &__tag {
    display: inline-flex;
    align-items: center;
    height: 22px;
    padding: 0 10px;
    border-radius: var(--apple-radius-full);
    background-color: var(--apple-surface-2);
    color: var(--apple-text-2);
    font-size: 12px;
    line-height: 1;
    font-weight: var(--apple-font-weight-medium);
    white-space: nowrap;
    max-width: 160px;
    overflow: hidden;
    text-overflow: ellipsis;

    &--more {
      background-color: transparent;
      color: var(--apple-text-3);
      padding: 0 4px;
    }
  }
}

::v-deep .device-card__action.ui-btn {
  height: 30px;
  padding: 0 14px;
  font-size: var(--apple-font-size-xs);
  font-weight: var(--apple-font-weight-medium);
  border-radius: var(--apple-radius-full);
  background-color: var(--apple-surface-2);
  color: var(--apple-text);

  &:hover:not(.is-disabled):not(.is-loading) {
    background-color: rgba(0, 113, 227, 0.10);
    color: var(--apple-brand);
    border-color: rgba(0, 113, 227, 0.10);
    box-shadow: none;
    transform: translateY(0);
  }
}

::v-deep .device-card__action--primary.ui-btn.ui-btn--primary {
  padding: 0 18px;
  box-shadow: 0 1px 2px rgba(0, 113, 227, 0.20);
}

::v-deep .device-card__action--inactive.ui-btn {
  background-color: var(--apple-surface-2);
  color: var(--apple-text-disabled);
  cursor: not-allowed;

  &:hover {
    background-color: var(--apple-surface-2);
    color: var(--apple-text-disabled);
  }
}

.more-tag {
  cursor: pointer;
  flex-shrink: 0;
}

.all-tags-popover {
  display: flex;
  flex-wrap: wrap;
  gap: 4px;
}

.disabled-btn {
  background: var(--apple-surface-2);
  color: var(--apple-text-disabled);
  cursor: not-allowed;
}

@keyframes device-status-breathe {
  0%, 100% { box-shadow: 0 0 0 0 rgba(52, 199, 89, 0.45); }
  50% { box-shadow: 0 0 0 5px rgba(52, 199, 89, 0); }
}

@media (prefers-reduced-motion: reduce) {
  .device-card::after {
    transition: none;
  }

  .device-card:hover::after {
    transform: translateX(-120%);
  }

  .device-card::before {
    transition: none;
    opacity: 1;
  }

  .device-card__status--online {
    animation: none;
  }
}
</style>

<style>
.device-item-tooltip {
  max-height: 60vh !important;
  max-width: 400px !important;
  overflow-y: auto !important;
  scrollbar-width: thin;
  word-break: break-word;
  border-radius: var(--apple-radius-md);
}

.device-item-tooltip .popper__arrow {
  display: none !important;
}

.device-item-tooltip[x-placement^="top"] .popper__arrow {
  border-top-color: transparent !important;
}

.device-item-tooltip[x-placement^="bottom"] .popper__arrow {
  border-bottom-color: transparent !important;
}
</style>
