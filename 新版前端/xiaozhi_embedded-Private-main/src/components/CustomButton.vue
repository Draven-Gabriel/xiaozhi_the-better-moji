<template>
  <el-button
    :class="['custom-button', `custom-button--${type}`]"
    v-bind="$attrs"
    @click="$emit('click', $event)"
  >
    <slot />
  </el-button>
</template>

<script>
export default {
  name: 'CustomButton',
  props: {
    type: {
      type: String,
      default: 'default',
      validator: (val) => ['default', 'confirm', 'delete', 'add'].includes(val)
    }
  }
};
</script>

<style lang="scss" scoped>
.custom-button--default {
  width: fit-content;
  background: var(--apple-surface);
  color: var(--apple-text);
  border: 1px solid var(--apple-divider);
  border-radius: var(--apple-radius-md);
  transition:
    background-color var(--apple-duration-fast) var(--apple-ease-standard),
    border-color var(--apple-duration-fast) var(--apple-ease-standard),
    color var(--apple-duration-fast) var(--apple-ease-standard),
    box-shadow var(--apple-duration-fast) var(--apple-ease-standard),
    transform var(--apple-duration-fast) var(--apple-ease-standard);

  &.el-button--small {
    padding: 9px 14px;
    font-size: 12px;
  }
  &.el-button--medium {
    padding: 12px 20px;
  }

  &:hover {
    background: var(--apple-surface-2);
    color: var(--apple-text);
    border-color: var(--apple-divider);
    box-shadow: var(--apple-shadow-card);
    transform: translateY(-1px);
  }

  &.is-disabled,
  &.is-disabled:hover {
    background: var(--apple-surface-2);
    color: var(--apple-text-disabled);
    border-color: var(--apple-divider-soft);
    box-shadow: none;
    transform: none;
    cursor: not-allowed;
    opacity: 1;
  }
}

.custom-button--confirm {
  width: fit-content;
  background: var(--apple-brand);
  color: #ffffff;
  border: 1px solid var(--apple-brand);
  border-radius: var(--apple-radius-md);
  transition:
    background-color var(--apple-duration-fast) var(--apple-ease-standard),
    border-color var(--apple-duration-fast) var(--apple-ease-standard),
    box-shadow var(--apple-duration-fast) var(--apple-ease-standard),
    transform var(--apple-duration-fast) var(--apple-ease-standard);

  &.el-button--small {
    padding: 9px 14px;
    font-size: 12px;
  }
  &.el-button--medium {
    padding: 12px 20px;
  }

  &:hover {
    background: var(--apple-brand-hover);
    color: #ffffff;
    border-color: var(--apple-brand-hover);
    box-shadow: 0 4px 12px rgba(0, 113, 227, 0.28);
    transform: translateY(-1px);
  }

  &.is-disabled,
  &.is-disabled:hover {
    background: var(--apple-surface-2);
    color: var(--apple-text-disabled);
    border-color: var(--apple-divider-soft);
    box-shadow: none;
    transform: none;
    cursor: not-allowed;
  }
}

.custom-button--delete {
  width: fit-content;
  background: var(--apple-surface);
  color: var(--apple-danger);
  border: 1px solid var(--apple-divider);
  border-radius: var(--apple-radius-md);
  transition:
    background-color var(--apple-duration-fast) var(--apple-ease-standard),
    border-color var(--apple-duration-fast) var(--apple-ease-standard),
    color var(--apple-duration-fast) var(--apple-ease-standard),
    box-shadow var(--apple-duration-fast) var(--apple-ease-standard),
    transform var(--apple-duration-fast) var(--apple-ease-standard);

  &.el-button--small {
    padding: 9px 14px;
    font-size: 12px;
  }
  &.el-button--medium {
    padding: 12px 20px;
  }

  &:hover {
    background: rgba(255, 59, 48, 0.10);
    color: var(--apple-danger);
    border-color: rgba(255, 59, 48, 0.30);
    box-shadow: var(--apple-shadow-card);
    transform: translateY(-1px);
  }

  &.is-disabled,
  &.is-disabled:hover {
    background: var(--apple-surface-2);
    color: var(--apple-text-disabled);
    border-color: var(--apple-divider-soft);
    box-shadow: none;
    transform: none;
    cursor: not-allowed;
  }
}

.custom-button--add {
  width: fit-content;
  background: var(--apple-brand);
  color: #ffffff;
  border: 1px solid var(--apple-brand);
  border-radius: var(--apple-radius-md);
  transition:
    background-color var(--apple-duration-fast) var(--apple-ease-standard),
    border-color var(--apple-duration-fast) var(--apple-ease-standard),
    box-shadow var(--apple-duration-fast) var(--apple-ease-standard),
    transform var(--apple-duration-fast) var(--apple-ease-standard);

  &.el-button--small {
    padding: 9px 14px;
    font-size: 12px;
  }
  &.el-button--medium {
    padding: 12px 20px;
  }

  &:hover {
    background: var(--apple-brand-hover);
    color: #ffffff;
    border-color: var(--apple-brand-hover);
    box-shadow: 0 4px 12px rgba(0, 113, 227, 0.28);
    transform: translateY(-1px);
  }

  &.is-disabled,
  &.is-disabled:hover {
    background: var(--apple-surface-2);
    color: var(--apple-text-disabled);
    border-color: var(--apple-divider-soft);
    box-shadow: none;
    transform: none;
    cursor: not-allowed;
  }
}
</style>
