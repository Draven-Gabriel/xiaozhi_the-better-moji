<template>
  <div
    :class="[
      'ui-input',
      `ui-input--${size}`,
      `ui-input--${type}`,
      {
        'is-disabled': disabled,
        'is-focus': focused,
        'is-readonly': readonly,
        'is-clearable': clearable && hasValue && !disabled && !readonly,
        'has-prefix': !!$slots.prefix,
        'has-suffix': showWordLimit || (clearable && hasValue && !disabled && !readonly)
      }
    ]"
    v-bind="$attrs"
    v-on="outerListeners"
  >
    <span v-if="$slots.prefix" class="ui-input__prefix">
      <slot name="prefix"></slot>
    </span>
    <textarea
      v-if="type === 'textarea'"
      ref="el"
      class="ui-input__inner ui-input__inner--textarea"
      :value="value"
      :placeholder="placeholder"
      :disabled="disabled"
      :readonly="readonly"
      :maxlength="maxlength"
      :autocomplete="autocomplete"
      :rows="rows"
      :autofocus="autofocus"
      @input="handleInput"
      @change="handleChange"
      @focus="handleFocus"
      @blur="handleBlur"
    ></textarea>
    <input
      v-else
      ref="el"
      class="ui-input__inner"
      :type="type"
      :value="value"
      :placeholder="placeholder"
      :disabled="disabled"
      :readonly="readonly"
      :maxlength="maxlength"
      :autocomplete="autocomplete"
      :autofocus="autofocus"
      @input="handleInput"
      @change="handleChange"
      @focus="handleFocus"
      @blur="handleBlur"
    />
    <span
      v-if="clearable && hasValue && !disabled && !readonly"
      class="ui-input__clear"
      @click="handleClear"
      @mousedown.prevent
    >
      <svg viewBox="0 0 16 16" width="14" height="14" aria-hidden="true">
        <path
          d="M4 4 L12 12 M12 4 L4 12"
          stroke="currentColor"
          stroke-width="1.6"
          stroke-linecap="round"
          fill="none"
        />
      </svg>
    </span>
    <span
      v-if="showWordLimit && maxlength && (type === 'textarea')"
      class="ui-input__count"
    >{{ valueLength }} / {{ maxlength }}</span>
  </div>
</template>

<script>
export default {
  name: 'UiInput',
  inheritAttrs: false,
  props: {
    value: { type: [String, Number], default: '' },
    type: {
      type: String,
      default: 'text',
      validator: (val) => ['text', 'password', 'textarea'].includes(val)
    },
    placeholder: { type: String, default: '' },
    disabled: { type: Boolean, default: false },
    clearable: { type: Boolean, default: false },
    maxlength: { type: [String, Number], default: undefined },
    showWordLimit: { type: Boolean, default: false },
    size: {
      type: String,
      default: 'medium',
      validator: (val) => ['small', 'medium', 'large'].includes(val)
    },
    autofocus: { type: Boolean, default: false },
    readonly: { type: Boolean, default: false },
    autocomplete: { type: String, default: 'off' },
    rows: { type: [String, Number], default: 3 }
  },
  data() {
    return { focused: false };
  },
  computed: {
    hasValue() {
      return this.value !== '' && this.value !== null && this.value !== undefined;
    },
    valueLength() {
      return typeof this.value === 'string' ? this.value.length : 0;
    },
    outerListeners() {
      const rest = { ...this.$listeners };
      delete rest.input;
      delete rest.change;
      delete rest.blur;
      delete rest.focus;
      delete rest.clear;
      return rest;
    }
  },
  methods: {
    handleInput(event) {
      this.$emit('input', event.target.value);
    },
    handleChange(event) {
      this.$emit('change', event.target.value);
    },
    handleFocus(event) {
      this.focused = true;
      this.$emit('focus', event);
    },
    handleBlur(event) {
      this.focused = false;
      this.$emit('blur', event);
    },
    handleClear() {
      this.$emit('input', '');
      this.$emit('clear');
      this.$nextTick(() => {
        if (this.$refs.el) this.$refs.el.focus();
      });
    },
    focus() {
      if (this.$refs.el) this.$refs.el.focus();
    },
    blur() {
      if (this.$refs.el) this.$refs.el.blur();
    }
  }
};
</script>

<style lang="scss" scoped>
@import '@/styles/tokens.scss';

.ui-input {
  position: relative;
  display: inline-flex;
  align-items: center;
  width: 100%;
  font-family: $apple-font-family;
  font-size: $apple-font-size-base;
  color: $apple-text;
  background-color: $apple-surface-2;
  border: 1px solid transparent;
  border-radius: $apple-radius-sm;
  box-sizing: border-box;
  transition:
    background-color $apple-duration-fast $apple-ease-standard,
    border-color $apple-duration-fast $apple-ease-standard,
    box-shadow $apple-duration-fast $apple-ease-standard;
  vertical-align: middle;

  &:hover:not(.is-disabled):not(.is-readonly) {
    background-color: $apple-surface;
    border-color: $apple-divider;
  }

  &.is-readonly {
    background-color: $apple-surface-2;
  }

  &.is-focus {
    background-color: $apple-surface;
    border-color: var(--apple-brand);
    box-shadow: 0 0 0 4px rgba(0, 113, 227, 0.12);
  }

  &.is-disabled {
    cursor: not-allowed;
    background-color: $apple-surface-2;
    color: $apple-text-disabled;

    .ui-input__inner {
      cursor: not-allowed;
      color: $apple-text-disabled;
    }
  }

  &--small {
    height: 36px;
    font-size: $apple-font-size-sm;
  }

  &--medium {
    height: 40px;
    font-size: $apple-font-size-base;
  }

  &--large {
    height: 44px;
    font-size: $apple-font-size-md;
  }

  &__prefix {
    display: inline-flex;
    align-items: center;
    padding: 0 0 0 $apple-space-3;
    color: $apple-text-3;
    flex-shrink: 0;
  }

  &__inner {
    flex: 1 1 auto;
    min-width: 0;
    height: 100%;
    width: 100%;
    padding: 0 $apple-space-3;
    border: none;
    outline: none;
    background: transparent;
    color: inherit;
    font: inherit;
    box-sizing: border-box;
    line-height: $apple-line-height-normal;

    &::placeholder {
      color: $apple-text-3;
    }

    &--textarea {
      padding: $apple-space-3;
      height: auto;
      min-height: 96px;
      resize: vertical;
      border-radius: $apple-radius-md;
      line-height: $apple-line-height-normal;
    }
  }

  &.has-prefix .ui-input__inner {
    padding-left: $apple-space-2;
  }

  &__clear {
    display: inline-flex;
    align-items: center;
    justify-content: center;
    width: 20px;
    height: 20px;
    margin-right: $apple-space-2;
    border-radius: 50%;
    color: $apple-text-3;
    background-color: transparent;
    cursor: pointer;
    transition: color $apple-duration-fast $apple-ease-standard,
                background-color $apple-duration-fast $apple-ease-standard;
    flex-shrink: 0;

    &:hover {
      color: #ffffff;
      background-color: var(--apple-brand);
    }
  }

  &__count {
    position: absolute;
    right: $apple-space-3;
    bottom: $apple-space-2;
    font-size: $apple-font-size-xs;
    color: $apple-text-3;
    background-color: $apple-surface;
    padding: 0 $apple-space-1;
    pointer-events: none;
  }

  &--textarea.has-prefix .ui-input__inner {
    padding-left: $apple-space-3;
  }

  &--textarea .ui-input__inner {
    min-height: 96px;
  }
}
</style>
