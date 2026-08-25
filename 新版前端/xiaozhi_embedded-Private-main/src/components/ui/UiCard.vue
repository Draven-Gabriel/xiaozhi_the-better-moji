<template>
  <div
    :class="[
      'ui-card',
      `ui-card--shadow-${shadow}`,
      `ui-card--radius-${radius}`,
      { 'is-clickable': clickable }
    ]"
    :style="{ padding: padding }"
    v-bind="$attrs"
    v-on="outerListeners"
    @click="handleClick"
  >
    <div v-if="$slots.header" class="ui-card__header">
      <slot name="header"></slot>
    </div>
    <div class="ui-card__body">
      <slot></slot>
    </div>
  </div>
</template>

<script>
export default {
  name: 'UiCard',
  inheritAttrs: false,
  props: {
    shadow: {
      type: String,
      default: 'hover',
      validator: (val) => ['never', 'hover', 'always'].includes(val)
    },
    radius: {
      type: String,
      default: 'lg',
      validator: (val) => ['md', 'lg', 'xl'].includes(val)
    },
    padding: { type: String, default: '24px' },
    clickable: { type: Boolean, default: false }
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
      if (!this.clickable) return;
      this.$emit('click', event);
    }
  }
};
</script>

<style lang="scss" scoped>
@import '@/styles/tokens.scss';

.ui-card {
  position: relative;
  display: block;
  background-color: $apple-surface;
  color: $apple-text;
  font-family: $apple-font-family;
  box-sizing: border-box;
  overflow: hidden;
  transition:
    transform $apple-duration-normal $apple-ease-out-elastic,
    box-shadow $apple-duration-normal $apple-ease-out-elastic;

  &--radius-md { border-radius: $apple-radius-md; }
  &--radius-lg { border-radius: $apple-radius-lg; }
  &--radius-xl { border-radius: $apple-radius-xl; }

  &--shadow-never {
    box-shadow: none;
  }

  &--shadow-always {
    box-shadow: $apple-shadow-card;
  }

  &--shadow-hover {
    box-shadow: $apple-shadow-card;

    &:hover {
      transform: translateY(-2px);
      box-shadow: $apple-shadow-raised;
    }
  }

  &.is-clickable {
    cursor: pointer;
    user-select: none;

    &:active {
      transform: translateY(-1px);
      box-shadow: $apple-shadow-card;
    }
  }

  &__header {
    padding-bottom: $apple-space-4;
    margin-bottom: $apple-space-4;
    border-bottom: 1px solid $apple-divider-soft;
    font-weight: $apple-font-weight-semibold;
    font-size: $apple-font-size-md;
    color: $apple-text;
  }

  &__body {
    display: block;
    color: $apple-text;
  }
}
</style>
