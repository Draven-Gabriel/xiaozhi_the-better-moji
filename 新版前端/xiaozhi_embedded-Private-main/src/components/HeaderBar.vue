<template>
  <el-header class="header">
    <div class="header-container">
      <!-- 左侧元素 -->
      <div class="header-left" @click="handleRouter('home')">
        <GradientBrandText size="18" />
      </div>

      <!-- 中间导航菜单 -->
      <div class="header-center">
        <div class="nav-item" :class="{
          'active-tab':
            $route.path === '/home' ||
            $route.path === '/role-config' ||
            $route.path === '/device-management',
        }" @click="handleRouter('home')">
          <img loading="lazy" alt="" src="@/assets/header/robot.png" />
          <span class="nav-text">{{ $t("header.smartManagement") }}</span>
        </div>
        <!-- 普通用户显示音色克隆 -->
        <div v-if="!userInfo.superAdmin && featureStatus.voiceClone" class="nav-item"
          :class="{ 'active-tab': $route.path === '/voice-clone-management' }"
          @click="handleRouter('voiceCloneManagement')">
          <img loading="lazy" alt="" src="@/assets/header/voice.png" />
          <span class="nav-text">{{ $t("header.voiceCloneManagement") }}</span>
        </div>

        <!-- 超级管理员显示音色克隆下拉菜单 -->
        <el-dropdown v-if="userInfo.superAdmin && featureStatus.voiceClone" trigger="click"
          class="nav-item more-dropdown" :class="{
            'active-tab':
              $route.path === '/voice-clone-management' ||
              $route.path === '/voice-resource-management',
          }" @visible-change="handleVoiceCloneDropdownVisibleChange">
          <span class="el-dropdown-link">
            <img loading="lazy" alt="" src="@/assets/header/voice.png" />
            <span class="nav-text">{{ $t("header.voiceCloneManagement") }}</span>
            <i class="el-icon-arrow-down" :class="{ 'rotate-down': voiceCloneDropdownVisible }"></i>
          </span>
          <el-dropdown-menu slot="dropdown">
            <el-dropdown-item @click.native="handleRouter('voiceCloneManagement')">
              {{ $t("header.voiceCloneManagement") }}
            </el-dropdown-item>
            <el-dropdown-item @click.native="handleRouter('voiceResourceManagement')">
              {{ $t("header.voiceResourceManagement") }}
            </el-dropdown-item>
          </el-dropdown-menu>
        </el-dropdown>

        <div v-if="userInfo.superAdmin" class="nav-item"
          :class="{ 'active-tab': $route.path === '/model-config' }" @click="handleRouter('modelConfig')">
          <img loading="lazy" alt="" src="@/assets/header/model_config.png" />
          <span class="nav-text">{{ $t("header.modelConfig") }}</span>
        </div>
        <div v-if="featureStatus.knowledgeBase" class="nav-item"
          :class="{ 'active-tab': $route.path === '/knowledge-base-management' || $route.path === '/knowledge-file-upload' }"
          @click="handleRouter('knowledgeBaseManagement')">
          <img loading="lazy" alt="" src="@/assets/header/knowledge_base.png" />
          <span class="nav-text">{{ $t("header.knowledgeBase") }}</span>
        </div>
        <div v-if="featureStatus.addressBook" class="nav-item"
          :class="{ 'active-tab': $route.path === '/address-book-management' }"
          @click="handleRouter('addressBookManagement')">
          <img loading="lazy" alt="" src="@/assets/header/address_book.png" />
          <span class="nav-text">{{ $t("header.addressBook") }}</span>
        </div>
        <el-dropdown v-if="userInfo.superAdmin" trigger="click" class="nav-item more-dropdown" :class="{
          'active-tab': isParamDropdownActive
        }" @visible-change="handleParamDropdownVisibleChange">
          <span class="el-dropdown-link">
            <img :src="require('@/assets/header/param_management.png')" :style="{ filter: isParamDropdownActive ? 'brightness(0) invert(1)' : 'None' }" />
            <span class="nav-text">{{ $t("header.paramDictionary") }}</span>
            <i class="el-icon-arrow-down" :class="{ 'rotate-down': paramDropdownVisible }"></i>
          </span>
          <el-dropdown-menu slot="dropdown">
            <el-dropdown-item @click.native="handleRouter('paramManagement')">
              {{ $t("header.paramManagement") }}
            </el-dropdown-item>
            <el-dropdown-item @click.native="handleRouter('userManagement')">
              {{ $t("header.userManagement") }}
            </el-dropdown-item>
            <el-dropdown-item @click.native="handleRouter('otaManagement')">
              {{ $t("header.otaManagement") }}
            </el-dropdown-item>
            <el-dropdown-item @click.native="handleRouter('dictManagement')">
              {{ $t("header.dictManagement") }}
            </el-dropdown-item>
            <el-dropdown-item @click.native="handleRouter('providerManagement')">
              {{ $t("header.providerManagement") }}
            </el-dropdown-item>
            <el-dropdown-item @click.native="handleRouter('agentTemplate')">
              {{ $t("header.agentTemplate") }}
            </el-dropdown-item>
            <el-dropdown-item @click.native="handleRouter('replacementWordManagement')">
              {{ $t("header.replacementWordManagement") }}
            </el-dropdown-item>
            <el-dropdown-item @click.native="handleRouter('serverSideManagement')">
              {{ $t("header.serverSideManagement") }}
            </el-dropdown-item>
            <el-dropdown-item @click.native="handleRouter('featureManagement')">
              {{ $t("header.featureManagement") }}
            </el-dropdown-item>
          </el-dropdown-menu>
        </el-dropdown>
      </div>

      <!-- 右侧元素 -->
      <div class="header-right">
        <img loading="lazy" alt="" src="@/assets/home/avatar.png" class="avatar-img" @click="handleAvatarClick" />
        <span class="user-name" @click="handleAvatarClick">
          {{ userInfo.username || "加载中..." }}
          <i class="el-icon-arrow-down el-icon--right" :class="{ 'rotate-down': userMenuVisible }"></i>
        </span>
        <el-cascader :options="userMenuOptions" trigger="click" :props="cascaderProps"
          placement="bottom-end" popper-class="user-menu-popper"
          style="width: 0px; height: 0px; overflow: visible; position: absolute; right: 0; top: 100%;"
          :show-all-levels="false" @change="handleCascaderChange"
          @visible-change="handleUserMenuVisibleChange" ref="userCascader">
          <template slot-scope="{ data }">
            <span>{{ data.label }}</span>
          </template>
        </el-cascader>
      </div>
    </div>

    <!-- 修改密码弹窗 -->
    <ChangePasswordDialog v-model="isChangePasswordDialogVisible" />
  </el-header>
</template>
<script>
import i18n, { changeLanguage } from "@/i18n";
import featureManager from "@/utils/featureManager"; // 引入功能管理工具类
import { mapActions, mapState } from "vuex";
import ChangePasswordDialog from "./ChangePasswordDialog.vue"; // 引入修改密码弹窗组件
import GradientBrandText from "./GradientBrandText.vue";

export default {
  name: "HeaderBar",
  components: {
    ChangePasswordDialog,
    GradientBrandText,
  },
  props: ["devices"], // 接收父组件设备列表
  data() {
    return {
      search: "",
      isChangePasswordDialogVisible: false, // 控制修改密码弹窗的显示
      paramDropdownVisible: false,
      voiceCloneDropdownVisible: false,
      userMenuVisible: false, // 添加用户菜单可见状态
      menuVisibleTimer: null, // 菜单显示定时器，防止够快触发
      // Cascader 配置
      cascaderProps: {
        expandTrigger: "click",
        value: "value",
        label: "label",
        children: "children",
      },
      // 跳转页面配置
      routerPaths: {
        home: "/home",
        modelConfig: "/model-config",
        knowledgeBaseManagement: "/knowledge-base-management",
        addressBookManagement: "/address-book-management",
        voiceCloneManagement: "/voice-clone-management",
        voiceResourceManagement: "/voice-resource-management",
        paramManagement: "/params-management",
        userManagement: "/user-management",
        otaManagement: "/ota-management",
        dictManagement: "/dict-management",
        providerManagement: "/provider-management",
        agentTemplate: "/agent-template-management",
        replacementWordManagement: "/replacement-word-management",
        serverSideManagement: "/server-side-management",
        featureManagement: "/feature-management",
      }
    };
  },
  computed: {
    ...mapState({
      featureStatus: (state) => ({
        voiceClone: state.pubConfig.systemWebMenu?.features?.voiceClone?.enabled, // 音色克隆功能状态
        knowledgeBase: state.pubConfig.systemWebMenu?.features?.knowledgeBase?.enabled, // 知识库功能状态
        addressBook: state.pubConfig.systemWebMenu?.features?.addressBook?.enabled, // 通讯录功能状态
      }),
      userInfo: (state) => state.userInfo,
    }),
    // 获取当前语言
    currentLanguage() {
      return i18n.locale || "zh_CN";
    },
    // 获取当前语言显示文本
    currentLanguageText() {
      const currentLang = this.currentLanguage;
      switch (currentLang) {
        case "zh_CN":
          return this.$t("language.zhCN");
        case "zh_TW":
          return this.$t("language.zhTW");
        case "en":
          return this.$t("language.en");
        case "de":
          return this.$t("language.de");
        case "vi":
          return this.$t("language.vi");
        case "pt_BR":
          return this.$t("language.ptBR");
        default:
          return this.$t("language.zhCN");
      }
    },
    // 用户菜单选项
    userMenuOptions() {
      return [
        {
          label: this.currentLanguageText,
          value: "language",
          children: [
            {
              label: this.$t("language.zhCN"),
              value: "zh_CN",
            },
            {
              label: this.$t("language.zhTW"),
              value: "zh_TW",
            },
            {
              label: this.$t("language.en"),
              value: "en",
            },
            {
              label: this.$t("language.de"),
              value: "de",
            },
            {
              label: this.$t("language.vi"),
              value: "vi",
            },
            {
              label: this.$t("language.ptBR"),
              value: "pt_BR",
            },
          ],
        },
        {
          label: this.$t("header.changePassword"),
          value: "changePassword",
        },
        {
          label: this.$t("header.logout"),
          value: "logout",
        },
      ];
    },
    // 参数设置下拉激活判定
    isParamDropdownActive() {
      const p = this.$route.path;
      return [
        "/params-management",
        "/feature-management",
        "/server-side-management",
        "/user-management",
        "/ota-management",
        "/dict-management",
        "/provider-management",
        "/agent-template-management",
        "/replacement-word-management",
      ].includes(p);
    },
  },
  async mounted() {
    // 等待featureManager初始化完成后再加载功能状态
    await this.loadFeatureStatus();
  },
  methods: {
    handleRouter(type) {
      this.$router.push(this.routerPaths[type]);
    },
    // 加载功能状态
    async loadFeatureStatus() {
      // 等待featureManager初始化完成
      await featureManager.waitForInitialization();
    },
    // 显示修改密码弹窗
    showChangePasswordDialog() {
      this.isChangePasswordDialogVisible = true;
      // 添加：显示修改密码弹窗后重置用户菜单可见状态
      this.userMenuVisible = false;
    },
    // 退出登录
    async handleLogout() {
      try {
        // 调用 Vuex 的 logout action
        await this.logout();
        this.$message.success({
          message: this.$t("message.success"),
          showClose: true,
        });
      } catch (error) {
        console.error("退出登录失败:", error);
        this.$message.error({
          message: this.$t("message.error"),
          showClose: true,
        });
      }
    },
    handleParamDropdownVisibleChange(visible) {
      this.paramDropdownVisible = visible;
    },

    // 监听音色克隆下拉菜单的可见状态变化
    handleVoiceCloneDropdownVisibleChange(visible) {
      this.voiceCloneDropdownVisible = visible;
    },
    // 在data中添加一个key用于强制重新渲染组件
    // 处理 Cascader 选择变化
    handleCascaderChange(value) {
      if (!value || value.length === 0) {
        return;
      }

      const action = value[value.length - 1];

      // 处理语言切换
      if (value.length === 2 && value[0] === "language") {
        this.changeLanguage(action);
      } else {
        // 处理其他操作
        switch (action) {
          case "changePassword":
            this.showChangePasswordDialog();
            break;
          case "logout":
            this.handleLogout();
            break;
        }
      }

      // 操作完成后立即清空选择
      setTimeout(() => {
        this.completeResetCascader();
      }, 300);
    },

    // 切换语言
    changeLanguage(lang) {
      changeLanguage(lang);
      this.$message.success({
        message: this.$t("message.success"),
        showClose: true,
      });
      // 添加：切换语言后重置用户菜单可见状态
      this.userMenuVisible = false;
    },

    // 完全重置级联选择器
    completeResetCascader() {
      if (this.$refs.userCascader) {
        try {
          // 尝试所有可能的方法来清空选择
          // 1. 尝试使用组件提供的clearValue方法
          if (this.$refs.userCascader.clearValue) {
            this.$refs.userCascader.clearValue();
          }

          // 2. 直接清空内部属性
          if (this.$refs.userCascader.$data) {
            this.$refs.userCascader.$data.selectedPaths = [];
            this.$refs.userCascader.$data.displayLabels = [];
            this.$refs.userCascader.$data.inputValue = "";
            this.$refs.userCascader.$data.checkedValue = [];
            this.$refs.userCascader.$data.showAllLevels = false;
          }

          // 3. 操作DOM清除选中状态
          const menuElement = this.$refs.userCascader.$refs.menu;
          if (menuElement && menuElement.$el) {
            const activeItems = menuElement.$el.querySelectorAll(
              ".el-cascader-node.is-active"
            );
            activeItems.forEach((item) => item.classList.remove("is-active"));

            const checkedItems = menuElement.$el.querySelectorAll(
              ".el-cascader-node.is-checked"
            );
            checkedItems.forEach((item) => item.classList.remove("is-checked"));
          }

          console.log("Cascader values cleared");
        } catch (error) {
          console.error("清空选择值失败:", error);
        }
      }
    },

    // 点击头像触发cascader下拉菜单
    handleAvatarClick() {
      if (this.$refs.userCascader) {
        // 切换菜单可见状态
        this.userMenuVisible = !this.userMenuVisible;

        // 菜单收起时清空选择值
        if (!this.userMenuVisible) {
          this.completeResetCascader();
        }

        // 直接设置菜单的显隐状态
        try {
          // 尝试使用toggleDropDownVisible方法
          this.$refs.userCascader.toggleDropDownVisible(this.userMenuVisible);
        } catch (error) {
          // 如果toggle方法失败，尝试直接设置属性
          if (this.$refs.userCascader.$refs.menu) {
            this.$refs.userCascader.$refs.menu.showMenu(this.userMenuVisible);
          } else {
            console.error("Cannot access menu component");
          }
        }
      }
    },

    // 处理用户菜单可见性变化
    handleUserMenuVisibleChange(visible) {
      if (this.menuVisibleTimer) return;
      this.menuVisibleTimer = setTimeout(() => {
        this.userMenuVisible = visible;
        clearTimeout(this.menuVisibleTimer);
        this.menuVisibleTimer = null;
      }, 100);

      // 如果菜单关闭了，也要清空选择值
      if (!visible) {
        this.completeResetCascader();
      }
    },

    // 使用 mapActions 引入 Vuex 的 logout action
    ...mapActions(["logout"]),
  },
};
</script>

<style lang="scss" scoped>
@import "@/styles/tokens.scss";

.header {
  position: sticky;
  top: 0;
  z-index: var(--apple-z-sticky);
  @include apple-glass;
  height: var(--apple-header-height) !important;
  padding: 0 !important;
  min-width: 900px;
  box-sizing: border-box;
  overflow: visible;
}

.header-container {
  display: flex;
  justify-content: space-between;
  align-items: center;
  height: 100%;
  max-width: var(--apple-container-max);
  margin: 0 auto;
  padding: 0 var(--apple-container-padding-x);
}

.header-left {
  display: flex;
  align-items: center;
  cursor: pointer;
  flex-shrink: 0;
}

.header-center {
  display: flex;
  align-items: center;
  gap: $apple-space-1;
  flex: 1;
  justify-content: center;
  padding: 0 $apple-space-6;
}

.header-right {
  display: flex;
  align-items: center;
  gap: $apple-space-2;
  justify-content: flex-end;
  flex-shrink: 0;
  position: relative;

  ::v-deep .el-cascader .el-input {
    display: none;
  }
}

.nav-item {
  display: flex;
  justify-content: center;
  align-items: center;
  gap: $apple-space-2;
  padding: $apple-space-2 14px;
  border-radius: var(--apple-radius-full);
  font-size: var(--apple-font-size-sm);
  font-weight: var(--apple-font-weight-medium);
  color: var(--apple-text-2);
  background-color: transparent;
  cursor: pointer;
  flex-shrink: 0;
  position: relative;
  transform-origin: center bottom;
  transition:
    background-color var(--apple-duration-fast) var(--apple-ease-standard),
    color var(--apple-duration-fast) var(--apple-ease-standard),
    opacity var(--apple-duration-fast) var(--apple-ease-standard),
    transform var(--apple-duration-normal) var(--apple-ease-out-elastic);
  outline: none;

  &:hover {
    background-color: var(--apple-surface-2);
    color: var(--apple-text);
    transform: translateY(-1px) scale(1.08);

    img {
      opacity: 1;
      filter: none;
    }
  }

  &:active {
    transform: translateY(0) scale(1);
  }

  &.active-tab {
    background-color: var(--apple-brand-tint);
    color: var(--apple-brand);

    img {
      opacity: 1;
      filter: none;
    }
  }

  img {
    width: 18px;
    height: 18px;
    opacity: 0.85;
    filter: none;
    transition:
      opacity var(--apple-duration-fast) var(--apple-ease-standard),
      filter var(--apple-duration-fast) var(--apple-ease-standard);
  }
}

.nav-item.more-dropdown {
  position: relative;
}

.nav-text {
  white-space: nowrap;
  line-height: 1.2;
}

.el-dropdown-link {
  display: flex;
  align-items: center;
  gap: $apple-space-2;
}

.avatar-img {
  width: 28px;
  height: 28px;
  border-radius: var(--apple-radius-full);
  border: 1px solid rgba(0, 0, 0, 0.06);
  flex-shrink: 0;
  cursor: pointer;
  transition: transform var(--apple-duration-fast) var(--apple-ease-standard),
              box-shadow var(--apple-duration-fast) var(--apple-ease-standard);

  &:hover {
    transform: translateY(-1px);
    box-shadow: var(--apple-shadow-card);
  }
}

.user-name {
  display: inline-flex;
  align-items: center;
  gap: $apple-space-1;
  cursor: pointer;
  color: var(--apple-text);
  font-size: var(--apple-font-size-sm);
  font-weight: var(--apple-font-weight-medium);
  transition: color var(--apple-duration-fast) var(--apple-ease-standard);

  &:hover {
    color: var(--apple-text-2);
  }

  .el-icon-arrow-down {
    font-size: var(--apple-font-size-xs);
  }
}

.rotate-down {
  transform: rotate(180deg);
  transition: transform var(--apple-duration-normal) var(--apple-ease-standard);
}

.el-icon-arrow-down {
  font-size: var(--apple-font-size-xs);
  transition: transform var(--apple-duration-normal) var(--apple-ease-standard);
}

.nav-item.more-dropdown .el-dropdown-menu {
  position: absolute;
  right: 0;
  left: auto;
  min-width: 180px;
  margin-top: $apple-space-2;
}

::v-deep .el-dropdown-menu {
  background-color: var(--apple-surface);
  border: none;
  border-radius: var(--apple-radius-md);
  box-shadow: var(--apple-shadow-overlay);
  padding: $apple-space-1;
  overflow: hidden;
}

::v-deep .el-dropdown-menu__item {
  min-width: unset;
  padding: $apple-space-2 $apple-space-3;
  font-size: var(--apple-font-size-sm);
  font-weight: var(--apple-font-weight-regular);
  color: var(--apple-text);
  background-color: transparent;
  border-radius: var(--apple-radius-sm);
  line-height: var(--apple-line-height-normal);
  white-space: nowrap;
  transition:
    background-color var(--apple-duration-fast) var(--apple-ease-standard),
    color var(--apple-duration-fast) var(--apple-ease-standard);

  &:hover,
  &:focus {
    background-color: var(--apple-surface-2);
    color: var(--apple-brand);
  }
}

::v-deep .el-popper .popper__arrow,
::v-deep .popper__arrow {
  display: none;
}

::v-deep .el-cascader-dropdown {
  background-color: var(--apple-surface);
  border: none;
  border-radius: var(--apple-radius-md);
  box-shadow: var(--apple-shadow-overlay);
}

::v-deep .user-menu-popper {
  margin-top: $apple-space-2;
}

::v-deep .el-cascader-panel {
  background-color: var(--apple-surface);
  border-radius: var(--apple-radius-md);
  overflow: hidden;
}

::v-deep .el-cascader-menu {
  padding: $apple-space-1;
  min-width: 180px;
}

::v-deep .el-cascader-node {
  padding: 0 $apple-space-3;
  height: 34px;
  line-height: 34px;
  border-radius: var(--apple-radius-sm);
  font-size: var(--apple-font-size-sm);
  color: var(--apple-text);
  transition:
    background-color var(--apple-duration-fast) var(--apple-ease-standard),
    color var(--apple-duration-fast) var(--apple-ease-standard);

  &:hover,
  &:focus {
    background-color: var(--apple-surface-2);
    color: var(--apple-brand);
  }

  &.is-active {
    background-color: var(--apple-brand-tint);
    color: var(--apple-brand);
    font-weight: var(--apple-font-weight-medium);
  }

  &.is-checked {
    color: var(--apple-brand);
  }
}

@media (max-width: 1280px) {
  .header-container {
    padding: 0 $apple-space-6;
  }

  .nav-item {
    padding: $apple-space-2 10px;
  }

  .header-center {
    gap: $apple-space-1;
  }
}

@media (max-width: 1024px) {
  .nav-item {
    font-size: var(--apple-font-size-xs);
  }

  .nav-item img {
    width: 16px;
    height: 16px;
  }
}
</style>
