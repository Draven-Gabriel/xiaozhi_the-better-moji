<template>
  <span
    :class="['ui-tag', `ui-tag--${type}`, `ui-tag--${size}`, { 'is-closable': closable }]"
    v-bind="$attrs"
    v-on="$listeners"
  >
    <slot></slot>
    <span v-if="closable" class="ui-tag__close" role="button" @click.stop="handleClose">
      <svg viewBox="0 0 12 12" width="10" height="10" aria-hidden="true">
        <path
          d="M3 3 L9 9 M9 3 L3 9"
          stroke="currentColor"
          stroke-width="1.6"
          stroke-linecap="round"
          fill="none"
        />
      </svg>
    </span>
  </span>
</template>

<script>
export default {
  name: 'UiTag',
  inheritAttrs: false,
  props: {
    type: {
      type: String,
      default: 'default',
      validator: (val) => ['default', 'primary', 'success', 'warning', 'danger'].includes(val)
    },
    size: {
      type: String,
      default: 'medium',
      validator: (val) => ['small', 'medium'].includes(val)
    },
    closable: { type: Boolean, default: false },
    disableTransitions: { type: Boolean, default: false }
  },
  methods: {
    handleClose(event) {
      this.$emit('close', event);
    }
  }
};
</script>

<style lang="scss" scoped>
@import '@/styles/tokens.scss';

.ui-tag {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  gap: $apple-space-2;
  font-family: $apple-font-family;
  font-weight: $apple-font-weight-medium;
  font-size: $apple-font-size-xs;
  line-height: 1;
  padding: 6px 12px;
  border-radius: $apple-radius-full;
  background-color: $apple-surface-2;
  color: $apple-text-2;
  border: 1px solid transparent;
  white-space: nowrap;
  user-select: none;
  transition:
    background-color $apple-duration-fast $apple-ease-standard,
    color $apple-duration-fast $apple-ease-standard;

  &--small {
    font-size: 12px;
    padding: 4px 10px;
  }

  &--medium {
    font-size: 12px;
    padding: 6px 12px;
  }

  &--default {
    background-color: rgba(0, 0, 0, 0.05);
    color: $apple-text-2;
  }

  &--primary {
    background-color: var(--apple-brand-tint);
    color: var(--apple-brand);
  }

  &--success {
    background-color: rgba(52, 199, 89, 0.12);
    color: #1f7a3a;
  }

  &--warning {
    background-color: rgba(255, 159, 10, 0.14);
    color: #a45a00;
  }

  &--danger {
    background-color: rgba(255, 59, 48, 0.12);
    color: #b3261e;
  }

  &.is-closable {
    padding-right: 8px;
  }

  &__close {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    width: 16px;
    height: 16px;
    border-radius: 50%;
    color: inherit;
    cursor: pointer;
    opacity: 0.6;
    transition: opacity $apple-duration-fast $apple-ease-standard,
                background-color $apple-duration-fast $apple-ease-standard,
                color $apple-duration-fast $apple-ease-standard;

    &:hover {
      opacity: 1;
      background-color: rgba(0, 0, 0, 0.08);
      color: $apple-text;
    }
  }
}
</style>
