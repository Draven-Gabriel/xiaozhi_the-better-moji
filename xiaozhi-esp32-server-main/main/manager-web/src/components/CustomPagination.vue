<template>
  <div class="pagination-wrapper">
    <el-pagination
      :current-page="currentPage"
      :page-size="pageSize"
      :page-sizes="pageSizeOptions"
      :total="total"
      layout="sizes, prev, pager, next, total"
      background
      @size-change="handleSizeChange"
      @current-change="handlePageChange"
    />
  </div>
</template>

<script>
export default {
  name: 'Pagination',
  props: {
    total: {
      type: Number,
      default: 0
    },
    currentPage: {
      type: Number,
      default: 1
    },
    pageSize: {
      type: Number,
      default: 10
    },
    pageSizeOptions: {
      type: Array,
      default: () => [10, 20, 50, 100]
    }
  },
  methods: {
    handleSizeChange(val) {
      this.$emit('size-change', val);
    },
    handlePageChange(page) {
      this.$emit('page-change', page);
    }
  }
};
</script>

<style lang="scss" scoped>
::v-deep .el-pagination {
  color: var(--apple-text-2);
  font-weight: var(--apple-font-weight-regular);

  .btn-next,
  .btn-prev,
  .el-pager li {
    background: var(--apple-surface) !important;
    min-width: 36px !important;
    color: var(--apple-text-2);
    border-radius: var(--apple-radius-sm);
    transition:
      background-color var(--apple-duration-fast) var(--apple-ease-standard),
      color var(--apple-duration-fast) var(--apple-ease-standard);
  }

  .btn-next:hover,
  .btn-prev:hover {
    color: var(--apple-brand) !important;
  }

  .el-pager li:not(.disabled):hover {
    color: var(--apple-brand) !important;
  }

  .el-pager li:not(.disabled).active {
    background: var(--apple-brand) !important;
    border-radius: var(--apple-radius-sm);
    color: #ffffff !important;
    font-weight: var(--apple-font-weight-semibold);
    box-shadow: 0 2px 8px rgba(0, 113, 227, 0.28);
  }

  .el-pagination__sizes .el-input .el-input__inner {
    border-radius: var(--apple-radius-sm);
    border-color: var(--apple-divider);
    background: var(--apple-surface);
    color: var(--apple-text);
  }

  .el-pagination__total {
    color: var(--apple-text-2);
    font-weight: var(--apple-font-weight-regular);
  }
}
</style>
