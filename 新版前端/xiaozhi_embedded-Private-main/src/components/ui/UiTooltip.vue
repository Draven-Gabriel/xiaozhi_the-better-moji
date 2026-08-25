<template>
  <span
    :class="['ui-tooltip', { 'is-disabled': disabled }]"
    v-bind="$attrs"
    v-on="$listeners"
  >
    <span
      class="ui-tooltip__trigger"
      @mouseenter="show"
      @mouseleave="hide"
      @focusin="show"
      @focusout="hide"
    >
      <slot></slot>
    </span>
    <transition name="ui-tooltip-fade">
      <span
        v-show="visible && !disabled"
        ref="popper"
        :class="['ui-tooltip__popper', `ui-tooltip__popper--${placement}`]"
        role="tooltip"
      >
        <span class="ui-tooltip__arrow" aria-hidden="true"></span>
        <span class="ui-tooltip__content">
          <slot name="tooltip-content">{{ content }}</slot>
        </span>
      </span>
    </transition>
  </span>
</template>

<script>
export default {
  name: 'UiTooltip',
  inheritAttrs: false,
  props: {
    content: { type: String, default: '' },
    placement: {
      type: String,
      default: 'top',
      validator: (val) => ['top', 'bottom', 'left', 'right'].includes(val)
    },
    disabled: { type: Boolean, default: false },
    effect: {
      type: String,
      default: 'dark',
      validator: (val) => ['dark', 'light'].includes(val)
    }
  },
  data() {
    return { visible: false };
  },
  methods: {
    show() {
      if (this.disabled) return;
      this.visible = true;
    },
    hide() {
      this.visible = false;
    }
  }
};
</script>

<style lang="scss" scoped>
@import '@/styles/tokens.scss';

.ui-tooltip {
  position: relative;
  display: inline-block;
  max-width: 100%;
  vertical-align: middle;

  &__trigger {
    display: inline-block;
    max-width: 100%;
  }

  &__popper {
    position: absolute;
    z-index: $apple-z-tooltip;
    display: inline-block;
    padding: 6px $apple-space-3;
    font-family: $apple-font-family;
    font-size: $apple-font-size-xs;
    line-height: $apple-line-height-normal;
    color: #ffffff;
    background-color: rgba(29, 29, 31, 0.92);
    border-radius: $apple-radius-sm;
    box-shadow: $apple-shadow-overlay;
    pointer-events: none;
    white-space: nowrap;
    max-width: 280px;
    opacity: 0;
    transform-origin: center center;
    transition:
      opacity $apple-duration-fast $apple-ease-standard,
      transform $apple-duration-fast $apple-ease-standard;

    &--top {
      bottom: calc(100% + 8px);
      left: 50%;
      transform: translateX(-50%) translateY(4px);

      .ui-tooltip__arrow {
        bottom: -4px;
        left: 50%;
        margin-left: -4px;

        &::before,
        &::after {
          border-top: none;
          border-left: none;
        }
      }
    }

    &--bottom {
      top: calc(100% + 8px);
      left: 50%;
      transform: translateX(-50%) translateY(-4px);

      .ui-tooltip__arrow {
        top: -4px;
        left: 50%;
        margin-left: -4px;

        &::before,
        &::after {
          border-bottom: none;
          border-right: none;
        }
      }
    }

    &--left {
      right: calc(100% + 8px);
      top: 50%;
      transform: translateY(-50%) translateX(4px);

      .ui-tooltip__arrow {
        right: -4px;
        top: 50%;
        margin-top: -4px;

        &::before,
        &::after {
          border-bottom: none;
          border-left: none;
        }
      }
    }

    &--right {
      left: calc(100% + 8px);
      top: 50%;
      transform: translateY(-50%) translateX(-4px);

      .ui-tooltip__arrow {
        left: -4px;
        top: 50%;
        margin-top: -4px;

        &::before,
        &::after {
          border-top: none;
          border-right: none;
        }
      }
    }
  }

  &__arrow {
    position: absolute;
    width: 8px;
    height: 8px;
    background-color: transparent;
    pointer-events: none;

    &::before,
    &::after {
      content: '';
      position: absolute;
      top: 0;
      left: 0;
      width: 8px;
      height: 8px;
      background-color: rgba(29, 29, 31, 0.92);
      border: 1px solid rgba(29, 29, 31, 0.92);
      transform: rotate(45deg);
    }

    &::after {
      background-color: transparent;
      width: 6px;
      height: 6px;
      top: 1px;
      left: 1px;
    }
  }

  &__content {
    display: inline-block;
    text-align: center;
  }
}

.ui-tooltip-fade-enter-active,
.ui-tooltip-fade-leave-active {
  opacity: 1;
  transform: var(--ui-tooltip-transform, none);
  transition:
    opacity $apple-duration-fast $apple-ease-standard,
    transform $apple-duration-fast $apple-ease-standard;
}

.ui-tooltip-fade-enter,
.ui-tooltip-fade-leave-to {
  opacity: 0;
}

.ui-tooltip__popper--top.ui-tooltip-fade-enter,
.ui-tooltip__popper--top.ui-tooltip-fade-leave-to {
  transform: translateX(-50%) translateY(4px);
}

.ui-tooltip__popper--bottom.ui-tooltip-fade-enter,
.ui-tooltip__popper--bottom.ui-tooltip-fade-leave-to {
  transform: translateX(-50%) translateY(-4px);
}

.ui-tooltip__popper--left.ui-tooltip-fade-enter,
.ui-tooltip__popper--left.ui-tooltip-fade-leave-to {
  transform: translateY(-50%) translateX(4px);
}

.ui-tooltip__popper--right.ui-tooltip-fade-enter,
.ui-tooltip__popper--right.ui-tooltip-fade-leave-to {
  transform: translateY(-50%) translateX(-4px);
}
</style>
