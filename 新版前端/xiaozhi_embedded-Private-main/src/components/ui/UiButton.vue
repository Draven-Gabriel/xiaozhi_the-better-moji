<template>
  <button
    :class="[
      'ui-btn',
      `ui-btn--${type}`,
      `ui-btn--${size}`,
      {
        'is-loading': loading,
        'is-disabled': disabled,
        'is-round': round,
        'is-circle': circle
      }
    ]"
    :disabled="disabled || loading"
    :type="nativeType"
    v-bind="$attrs"
    v-on="outerListeners"
    @click="handleClick"
  >
    <span v-if="loading" class="ui-btn__spinner" aria-hidden="true"></span>
    <i v-else-if="icon" :class="icon" aria-hidden="true"></i>
    <span class="ui-btn__content">
      <slot></slot>
    </span>
  </button>
</template>

<script>
export default {
  name: 'UiButton',
  inheritAttrs: false,
  props: {
    type: {
      type: String,
      default: 'default',
      validator: (val) => ['primary', 'default', 'danger', 'text', 'link'].includes(val)
    },
    size: {
      type: String,
      default: 'medium',
      validator: (val) => ['small', 'medium', 'large'].includes(val)
    },
    loading: { type: Boolean, default: false },
    disabled: { type: Boolean, default: false },
    round: { type: Boolean, default: false },
    circle: { type: Boolean, default: false },
    icon: { type: String, default: '' },
    nativeType: {
      type: String,
      default: 'button',
      validator: (val) => ['button', 'submit', 'reset'].includes(val)
    }
  },
  computed: {
    outerListeners() {
      const rest = { ...this.$listeners };
      delete rest.click;
      return rest;
    }
  },
  methods: {
    handleClick(event) {
      if (this.disabled || this.loading) return;
      this.$emit('click', event);
    }
  }
};
</script>

<style lang="scss" scoped>
@import '@/styles/tokens.scss';

.ui-btn {
  display: inline-flex;
  align-items: center;
  justify-content: center;
  gap: $apple-space-2;
  font-family: $apple-font-family;
  font-weight: $apple-font-weight-medium;
  font-size: $apple-font-size-sm;
  line-height: 1;
  padding: 0 $apple-space-4;
  height: 36px;
  border-radius: $apple-radius-md;
  border: 1px solid transparent;
  background-color: transparent;
  color: $apple-text;
  cursor: pointer;
  user-select: none;
  white-space: nowrap;
  vertical-align: middle;
  outline: none;
  transition:
    background-color $apple-duration-fast $apple-ease-standard,
    border-color $apple-duration-fast $apple-ease-standard,
    color $apple-duration-fast $apple-ease-standard,
    box-shadow $apple-duration-fast $apple-ease-standard,
    transform $apple-duration-fast $apple-ease-standard;

  &:focus-visible {
    box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.24);
  }

  &--small {
    height: 32px;
    padding: 0 $apple-space-3;
    font-size: $apple-font-size-xs;
  }

  &--medium {
    height: 36px;
    padding: 0 $apple-space-4;
    font-size: $apple-font-size-sm;
  }

  &--large {
    height: 44px;
    padding: 0 $apple-space-5;
    font-size: $apple-font-size-base;
  }

  &--primary {
    background-color: var(--apple-brand);
    color: #ffffff;
    border-color: var(--apple-brand);

    &:hover:not(.is-disabled):not(.is-loading) {
      background-color: var(--apple-brand-hover);
      border-color: var(--apple-brand-hover);
      transform: translateY(-1px);
      box-shadow: 0 4px 12px rgba(0, 113, 227, 0.28);
    }

    &:active:not(.is-disabled):not(.is-loading) {
      background-color: var(--apple-brand-active);
      border-color: var(--apple-brand-active);
      transform: translateY(0);
      box-shadow: 0 1px 2px rgba(0, 113, 227, 0.20);
    }
  }

  &--default {
    background-color: $apple-surface;
    color: $apple-text;
    border-color: $apple-divider;

    &:hover:not(.is-disabled):not(.is-loading) {
      background-color: $apple-surface-2;
      border-color: $apple-divider;
      transform: translateY(-1px);
      box-shadow: $apple-shadow-card;
    }

    &:active:not(.is-disabled):not(.is-loading) {
      background-color: $apple-surface-2;
      transform: translateY(0);
    }
  }

  &--danger {
    background-color: var(--apple-danger);
    color: #ffffff;
    border-color: var(--apple-danger);

    &:hover:not(.is-disabled):not(.is-loading) {
      background-color: #ff4f46;
      border-color: #ff4f46;
      transform: translateY(-1px);
      box-shadow: 0 4px 12px rgba(255, 59, 48, 0.28);
    }

    &:active:not(.is-disabled):not(.is-loading) {
      background-color: #d9322a;
      border-color: #d9322a;
      transform: translateY(0);
    }
  }

  &--text,
  &--link {
    background-color: transparent;
    border-color: transparent;
    color: var(--apple-brand);
    height: auto;
    padding: 0 $apple-space-2;

    &:hover:not(.is-disabled):not(.is-loading) {
      background-color: var(--apple-brand-tint);
      color: var(--apple-brand-hover);
    }

    &:active:not(.is-disabled):not(.is-loading) {
      background-color: rgba(0, 113, 227, 0.16);
    }
  }

  &--link {
    &:hover:not(.is-disabled):not(.is-loading) {
      background-color: transparent;
      text-decoration: underline;
    }
  }

  &.is-round {
    border-radius: $apple-radius-full;
  }

  &.is-circle {
    width: 36px;
    height: 36px;
    padding: 0;
    border-radius: 50%;

    &.ui-btn--small { width: 32px; height: 32px; }
    &.ui-btn--medium { width: 36px; height: 36px; }
    &.ui-btn--large { width: 44px; height: 44px; }
  }

  &.is-disabled,
  &.is-loading {
    cursor: not-allowed;
    pointer-events: none;
    opacity: 0.55;
  }

  &.is-disabled {
    box-shadow: none;
    transform: none;
  }

  &__spinner {
    display: inline-block;
    width: 14px;
    height: 14px;
    border: 2px solid currentColor;
    border-right-color: transparent;
    border-radius: 50%;
    animation: ui-btn-spin 0.7s linear infinite;
  }

  &__content {
    display: inline-flex;
    align-items: center;
  }
}

@keyframes ui-btn-spin {
  to { transform: rotate(360deg); }
}
</style>
