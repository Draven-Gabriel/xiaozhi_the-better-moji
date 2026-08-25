<template>
    <div class="welcome">
        <HeaderBar />
        <div class="main-wrapper">
            <div class="content-panel">
                <div class="content-area">
                    <el-card class="voice-print-card" shadow="never">
                        <div class="operation-header">
                            <h2 class="page-title">{{ $t('voicePrint.pageTitle') }}</h2>
                            <el-select
                                v-model="agentSwitcherValue"
                                size="small"
                                class="agent-switcher"
                                @change="handleAgentSwitch"
                            >
                                <el-option
                                    v-for="a in agentOptions"
                                    :key="a.id"
                                    :label="a.agentName"
                                    :value="a.id"
                                />
                            </el-select>
                        </div>
                        <CustomTable
                            ref="paramsTable"
                            :data="voicePrintList"
                            :columns="tableColumns"
                            :loading="loading"
                            :loading-text="$t('voicePrint.loading')"
                            :show-operations="true"
                            :operations-label="$t('voicePrint.action')"
                            :total="total"
                            :show-pagination="false"
                            :current-page="currentPage"
                            :page-size="pageSize"
                            :page-size-options="pageSizeOptions"
                            @size-change="handlePageSizeChange"
                            @page-change="goToPage"
                        >
                            <template slot="operations" slot-scope="scope">
                                <UiButton type="text" size="small" class="edit-btn" @click="editVoicePrint(scope.row)">{{ $t('voicePrint.edit') }}</UiButton>
                                <UiButton type="text" size="small" class="delete-btn" @click="deleteVoicePrint(scope.row.id)">{{ $t('voicePrint.delete') }}</UiButton>
                            </template>
                            <template slot="footer-btns">
                                <div class="ctrl_btn">
                                    <CustomButton icon="el-icon-plus" type="add" size="small" @click="showAddDialog">{{ $t('voicePrint.add') }}</CustomButton>
                                </div>
                            </template>
                        </CustomTable>
                    </el-card>
                </div>
            </div>
        </div>

        <!-- 新增/编辑参数对话框 -->
        <voice-print-dialog :title="dialogTitle" :visible.sync="dialogVisible" :agentId="agentId" :form="paramForm"
            @submit="handleSubmit" @cancel="dialogVisible = false" />
        <el-footer>
            <version-footer />
        </el-footer>
    </div>
</template>

<script>
import Api from "@/apis/api";
import HeaderBar from "@/components/HeaderBar.vue";
import VersionFooter from "@/components/VersionFooter.vue";
import VoicePrintDialog from "@/components/VoicePrintDialog.vue";
import CustomButton from "@/components/CustomButton.vue";
import CustomTable from "@/components/CustomTable.vue";
import UiButton from "@/components/ui/UiButton.vue";

export default {
    components: { HeaderBar, VoicePrintDialog, VersionFooter, CustomButton, CustomTable, UiButton },
    data() {
        return {
            voicePrintList: [],
            allVoicePrints: [],
            loading: false,
            dialogVisible: false,
            dialogTitle: this.$t('voicePrint.addSpeaker'),
            currentPage: 1,
            pageSize: 10,
            pageSizeOptions: [10, 20, 50, 100],
            total: 0,
            paramForm: {
                id: null,
                audioId: '',
                sourceName: '',
                introduce: ''
            },
            agentId: "1",
            agentSwitcherValue: "",
            agentOptions: [],
            tableColumns: []
        };
    },
    created() {
        this.initTableColumns();
    },
    mounted() {
        this.fetchAgentOptions();
    },
    fetchAgentOptions() {
        Api.agent.getAgentList(({ data }) => {
            const list = Array.isArray(data && data.data) ? data.data : [];
            this.agentOptions = list;
            const queryAgentId = this.$route.query.agentId;
            let targetId = queryAgentId;
            if (!targetId && list.length > 0) {
                targetId = list[0].id;
                this.$router.replace({ path: '/voice-print', query: { agentId: targetId } });
            }
            this.agentSwitcherValue = targetId || '';
            if (targetId) {
                this.agentId = targetId;
                this.fetchVoicePrints();
            }
        });
    },
    handleAgentSwitch(agentId) {
        if (!agentId || agentId === this.$route.query.agentId) {
            return;
        }
        this.agentId = agentId;
        this.$router.replace({ path: '/voice-print', query: { agentId } });
        this.fetchVoicePrints();
    },
    methods: {
        initTableColumns() {
            this.tableColumns = [
                { prop: 'sourceName', label: this.$t('voicePrint.name'), align: 'center' },
                { prop: 'introduce', label: this.$t('voicePrint.description'), align: 'center' },
                { prop: 'createDate', label: this.$t('voicePrint.createTime'), align: 'center' }
            ];
        },
        fetchVoicePrints() {
            this.loading = true;
            Api.agent.getAgentVoicePrintList(this.agentId,
                ({ data }) => {
                    this.loading = false;
                    if (data.code === 0) {
                        this.allVoicePrints = data.data;
                        this.total = data.data.length;
                        this.currentPage = 1;
                        this.updatePagedData();
                    } else {
                        this.$message.error({
                            message: data.msg || this.$t('voicePrint.fetchFailed'),
                            showClose: true
                        });
                    }
                }
            );
        },
        updatePagedData() {
            const start = (this.currentPage - 1) * this.pageSize;
            this.voicePrintList = this.allVoicePrints.slice(start, start + this.pageSize);
        },
        handlePageSizeChange(val) {
            this.pageSize = val;
            this.currentPage = 1;
            this.updatePagedData();
        },
        goToPage(page) {
            if (page !== this.currentPage) {
                this.currentPage = page;
                this.updatePagedData();
            }
        },
        showAddDialog() {
            this.dialogTitle = this.$t('voicePrint.addSpeaker');
            this.paramForm = {
                id: null,
                audioId: '',
                sourceName: '',
                introduce: ''
            };
            this.dialogVisible = true;
        },
        editVoicePrint(row) {
            this.dialogTitle = this.$t('voicePrint.editSpeaker');
            this.paramForm = { ...row };
            this.dialogVisible = true;
        },
        handleSubmit({ form, done }) {
            if (form.id) {
                Api.agent.updateAgentVoicePrint(form, ({ data }) => {
                    if (data.code === 0) {
                        this.$message.success({
                            message: this.$t('voicePrint.updateSuccess'),
                            showClose: true
                        });
                        this.dialogVisible = false;
                        this.fetchVoicePrints();
                    }
                    done && done();
                });
            } else {
                Api.agent.addAgentVoicePrint({
                    agentId: this.agentId,
                    audioId: form.audioId,
                    sourceName: form.sourceName,
                    introduce: form.introduce
                }, ({ data }) => {
                    if (data.code === 0) {
                        this.$message.success({
                            message: this.$t('voicePrint.addSuccess'),
                            showClose: true
                        });
                        this.dialogVisible = false;
                        this.fetchVoicePrints();
                    }
                    done && done();
                });
            }
        },
        deleteVoicePrint(id) {
            this.$confirm(this.$t('voicePrint.confirmDelete'), this.$t('voicePrint.warning'), {
                confirmButtonText: this.$t('voicePrint.confirm'),
                cancelButtonText: this.$t('voicePrint.cancel'),
                type: 'warning',
                distinguishCancelAndClose: true
            }).then(() => {
                Api.agent.deleteAgentVoicePrint(id, ({ data }) => {
                    if (data.code === 0) {
                        this.$message.success({
                            message: this.$t('voicePrint.deleteSuccess'),
                            showClose: true
                        });
                        this.fetchVoicePrints();
                    } else {
                        this.$message.error({
                            message: data.msg || this.$t('voicePrint.deleteFailed'),
                            showClose: true
                        });
                    }
                });
            }).catch(action => {
                if (action === 'cancel') {
                    this.$message({
                        type: 'info',
                        message: this.$t('voicePrint.cancelDelete'),
                        duration: 1000
                    });
                } else {
                    this.$message({
                        type: 'info',
                        message: this.$t('voicePrint.closeOperation'),
                        duration: 1000
                    });
                }
            });
        }
    }
};
</script>

<style lang="scss" scoped>
@import '@/styles/tokens.scss';

.welcome {
    min-width: 900px;
    min-height: 506px;
    height: 100vh;
    display: flex;
    position: relative;
    flex-direction: column;
    background-size: cover;
    background: var(--apple-bg);
    -webkit-background-size: cover;
    -o-background-size: cover;
    overflow: hidden;
}

.main-wrapper {
    height: calc(100vh - 63px - 35px);
    padding: 20px 22px 0;
    position: relative;
    display: flex;
    flex-direction: column;
    box-sizing: border-box;
}

.operation-header {
    height: 56px;
    display: flex;
    align-items: center;
    justify-content: space-between;
    padding: 0 0 16px 0;
    box-sizing: border-box;
}

.page-title {
    font-weight: 600;
    font-size: 24px;
    margin: 0;
    color: var(--apple-text);
}

.agent-switcher {
    width: 200px;
    margin-left: auto;
}
.agent-switcher ::v-deep .el-input__inner {
    border-radius: 8px;
    background: $apple-surface-2;
    border-color: $apple-divider-soft;
    color: #3d4566;
    font-weight: 500;
}
.agent-switcher ::v-deep .el-input__inner:hover {
    border-color: $apple-brand;
}
.agent-switcher ::v-deep .el-input.is-focus .el-input__inner {
    border-color: $apple-brand;
    box-shadow: 0 0 0 2px rgba(0, 113, 227, 0.12);
}

.content-panel {
    display: flex;
    overflow: hidden;
    height: 100%;
    border-radius: var(--apple-radius-lg);
    background: var(--apple-surface);
    box-shadow: var(--apple-shadow-card);
}

.content-area {
    flex: 1;
    height: 100%;
    min-width: 600px;
    overflow: auto;
    background-color: var(--apple-surface);
    display: flex;
    flex-direction: column;
}

.voice-print-card {
    background: var(--apple-surface);
    flex: 1;
    display: flex;
    flex-direction: column;
    border: none;
    box-shadow: none;
    overflow: hidden;

    ::v-deep .el-card__body {
        padding: 14px 20px;
        display: flex;
        flex-direction: column;
        flex: 1;
        overflow: hidden;
    }
}

.ctrl_btn {
    display: flex;
}

::v-deep .edit-btn.ui-btn--text {
    padding: 4px 10px;
    font-size: 13px;
    line-height: 1;
    border-radius: var(--apple-radius-sm);
    height: auto;
    min-height: 0;
    margin: 0 2px;

    &:hover {
        background: var(--apple-brand-tint);
        color: var(--apple-brand-hover);
    }
}

::v-deep .delete-btn.ui-btn--text {
    padding: 4px 10px;
    font-size: 13px;
    line-height: 1;
    border-radius: var(--apple-radius-sm);
    height: auto;
    min-height: 0;
    margin: 0 2px;
    color: var(--apple-danger);

    &:hover {
        background: rgba(255, 59, 48, 0.10);
        color: var(--apple-danger);
    }
}
</style>
