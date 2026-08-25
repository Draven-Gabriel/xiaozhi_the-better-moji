<template>
    <CustomDialog
        :title="$t('chatHistory.with') + agentName + $t('chatHistory.dialogTitle')"
        :visible.sync="dialogVisible"
        width="80%"
        :footer="false"
        :close-on-click-modal="false"
        custom-class="chat-history-dialog">
        <template v-slot:title>
            <span style="font-size: 18px;">
                {{ $t('chatHistory.with') + agentName + $t('chatHistory.dialogTitle') }}
                <template v-if="currentMacAddress">
                    [<MacAddressMask :macAddress="currentMacAddress" />]
                </template>
            </span>
        </template>
        <div class="chat-container">
            <div class="session-list" @scroll="handleScroll">
                <div v-for="session in sessions" :key="session.sessionId" class="session-item"
                    :class="{ active: currentSessionId === session.sessionId }" @click="selectSession(session)">
                    <img :src="getUserAvatar(session.sessionId)" class="avatar" />
                    <div class="session-info">
                        <div class="session-time">{{ session.title || formatTime(session.createdAt) }}</div>
                        <div class="message-count">{{ session.chatCount > 99 ? '99' : session.chatCount }}</div>
                    </div>
                </div>
                <div v-if="loading" class="loading">{{ $t('chatHistory.loading') }}</div>
                <div v-if="!hasMore" class="no-more">{{ $t('chatHistory.noMoreRecords') }}</div>
            </div>
            <div class="chat-content">
                <div v-if="currentSessionId" class="messages">
                    <div v-for="(message, index) in messagesWithTime" :key="message.id">
                        <div v-if="message.type === 'time'" class="time-divider">
                            {{ message.content }}
                        </div>
                        <div v-else class="message-item" :class="{ 'user-message': message.chatType === 1, 'tool-message': message.chatType === 3 }">
                            <img :src="message.chatType === 1 ? getUserAvatar(currentSessionId) : aiAvatarIcon"
                                class="avatar" />
                            <div class="message-content">
                                <template v-if="Array.isArray(extractContentFromString(message.content))">
                                    <div class="content-wrapper">
                                        <div v-for="(item, idx) in extractContentFromString(message.content)" :key="idx">
                                            <div v-if="item.type === 'text'" class="text-content">{{ item.text }}</div>
                                            <div v-else-if="item.type === 'tool'" class="tool-call-text">{{ item.text }}</div>
                                            <div v-else-if="item.type === 'tool_result'" class="tool-call-text">
                                                <div v-if="item.text && item.text.length > 80" class="tool-result-wrapper">
                                                    <div v-if="isToolResultCollapsed(index, idx)" class="tool-result-collapsed">
                                                        {{ getFirstLineText(item.text) }}
                                                    </div>
                                                    <div v-else class="tool-result-expanded">
                                                        {{ item.text }}
                                                    </div>
                                                    <span class="tool-toggle-btn" @click="toggleToolResult(index, idx)">
                                                        <i :class="isToolResultCollapsed(index, idx) ? 'el-icon-arrow-down' : 'el-icon-arrow-up'"></i>
                                                    </span>
                                                </div>
                                                <div v-else>{{ item.text }}</div>
                                            </div>
                                        </div>
                                    </div>
                                </template>
                                <template v-else>
                                    {{ extractContentFromString(message.content) }}
                                </template>
                                <i v-if="message.audioId" :class="getAudioIconClass(message)"
                                    @click="playAudio(message)" class="audio-icon"></i>
                            </div>
                        </div>
                    </div>
                </div>
                <div v-else class="no-session-selected">
                    {{ $t('chatHistory.selectSession') }}
                </div>
            </div>
        </div>
        <div v-if="currentSessionId" class="download-buttons">
            <el-button type="primary" plain size="small" @click="downloadCurrentSessionWithPrevious">
                {{ $t('chatHistory.downloadCurrentWithPreviousSessions') }}
            </el-button>
            <el-button type="primary" plain size="small" @click="downloadCurrentSession">
                {{ $t('chatHistory.downloadCurrentSession') }}
            </el-button>
        </div>
    </CustomDialog>
</template>

<script>
import { debounce } from '@/utils'
import Api from '@/apis/api';
import CustomDialog from '@/components/CustomDialog.vue';
import MacAddressMask from '@/components/MacAddressMask.vue';

export default {
    name: 'ChatHistoryDialog',
    props: {
        visible: {
            type: Boolean,
            default: false
        },
        agentId: {
            type: String,
            required: true
        },
        agentName: {
            type: String,
            required: true
        }
    },
    data() {
        return {
            dialogVisible: false,
            sessions: [],
            messages: [],
            currentSessionId: '',
            currentMacAddress: '',
            page: 1,
            limit: 20,
            loading: false,
            hasMore: true,
            scrollTimer: null,
            isFirstLoad: true,
            playingAudioId: null,
            audioElement: null,
            expandedToolResults: {} // 跟踪工具结果的展开状态
        };
    },
    components: {
        CustomDialog,
        MacAddressMask,
    },
    watch: {
        visible(val) {
            this.dialogVisible = val;
            if (val) {
                this.resetData();
                this.loadSessions();
            } else {
                this.audioElement?.pause();
                this.audioElement = null;
                this.playingAudioId = null;
            }
        },
        dialogVisible(val) {
            if (!val) {
                this.$emit('update:visible', false);
            }
        }
    },
    computed: {
        messagesWithTime() {
            if (!this.messages || this.messages.length === 0) return [];

            const result = [];
            const TIME_INTERVAL = 60 * 1000; // 1分钟的时间间隔（毫秒）

            // 添加第一条消息的时间标记
            if (this.messages[0]) {
                result.push({
                    type: 'time',
                    content: this.formatTime(this.messages[this.messages.length - 1].createdAt),
                    id: `time-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`
                });
            }

            // 处理消息列表
            for (let i = 0; i < this.messages.length; i++) {
                const currentMessage = this.messages[i];
                result.push(currentMessage);

                // 检查是否需要添加时间标记
                if (i < this.messages.length - 1) {
                    const currentTime = new Date(currentMessage.createdAt).getTime();
                    const nextTime = new Date(this.messages[i + 1].createdAt).getTime();

                    if (nextTime - currentTime > TIME_INTERVAL) {
                        result.push({
                            type: 'time',
                            content: this.formatTime(this.messages[i + 1].createdAt),
                            id: `time-${Date.now()}-${Math.random().toString(36).substr(2, 9)}`
                        });
                    }
                }
            }

            return result;
        },
        aiAvatarIcon() {
            const svg = '<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 64 64"><defs><linearGradient id="bg" x1="0%" y1="0%" x2="0%" y2="100%"><stop offset="0%" stop-color="#e8f1fd"/><stop offset="100%" stop-color="#d6e8fb"/></linearGradient></defs><circle cx="32" cy="32" r="32" fill="url(#bg)"/><rect x="18" y="20" width="28" height="24" rx="8" ry="8" fill="#ffffff"/><rect x="22" y="28" width="8" height="7" rx="1.8" fill="#9ec3f5"/><rect x="34" y="28" width="8" height="7" rx="1.8" fill="#9ec3f5"/><rect x="26" y="38" width="12" height="2.2" rx="1.1" fill="#9ec3f5"/></svg>';
            return 'data:image/svg+xml;utf8,' + encodeURIComponent(svg);
        }
    },
    methods: {
        /**
         * 从 content 字段中提取聊天内容
         * 如果 content 是 JSON 格式（如 {"speaker": "未知说话人", "content": "现在几点了。"}），则提取 content 字段
         * 如果 content 是普通字符串，则直接返回
         * 
         * @param {string} content 原始内容
         * @returns {string} 提取的聊天内容
         */
        extractContentFromString(content) {
            if (!content || content.trim() === '') {
                return content;
            }

            // 尝试解析为 JSON
            try {
                const jsonObj = JSON.parse(content);

                // 如果是数组格式（包含 text 和 tool）
                if (Array.isArray(jsonObj)) {
                    return jsonObj;
                }

                // 如果是对象且有 content 字段
                if (jsonObj && typeof jsonObj === 'object' && jsonObj.content) {
                    return jsonObj.content;
                }
            } catch (e) {
                // 如果不是有效的 JSON，直接返回原内容
            }

            // 如果不是 JSON 格式或没有 content 字段，直接返回原内容
            return content;
        },
        // 切换工具结果的展开/折叠状态
        toggleToolResult(messageIndex, itemIndex) {
            const key = `${messageIndex}-${itemIndex}`;
            this.$set(this.expandedToolResults, key, !this.expandedToolResults[key]);
        },
        // 判断工具结果是否处于折叠状态
        isToolResultCollapsed(messageIndex, itemIndex) {
            const key = `${messageIndex}-${itemIndex}`;
            // 默认折叠（true表示折叠）
            return !this.expandedToolResults[key];
        },
        // 获取截断的文本（只显示第一行）
        getFirstLineText(text) {
            if (!text) return '';
            const firstLine = text.split('\n')[0];
            return firstLine.length < text.length ? firstLine + '...' : text;
        },
        resetData() {
            this.sessions = [];
            this.messages = [];
            this.currentSessionId = '';
            this.currentMacAddress = '';
            this.page = 1;
            this.loading = false;
            this.hasMore = true;
            this.isFirstLoad = true;
            this.expandedToolResults = {};
        },
        loadSessions() {
            if (this.loading || (!this.isFirstLoad && !this.hasMore)) {
                return;
            }

            this.loading = true;
            const params = {
                page: this.page,
                limit: this.limit
            };

            Api.agent.getAgentSessions(this.agentId, params, (res) => {
                if (res.data && res.data.data && Array.isArray(res.data.data.list)) {
                    const list = res.data.data.list;
                    this.hasMore = list.length === this.limit;

                    this.sessions = [...this.sessions, ...list];
                    this.page++;

                    if (this.sessions.length > 0 && !this.currentSessionId) {
                        this.selectSession(this.sessions[0]);
                    }
                }
                this.loading = false;
                this.isFirstLoad = false;
            });
        },
        selectSession(session) {
            this.currentSessionId = session.sessionId;
            Api.agent.getAgentChatHistory(this.agentId, session.sessionId, (res) => {
                if (res.data && res.data.data) {
                    this.messages = res.data.data;
                    if (this.messages.length > 0 && this.messages[0].macAddress) {
                        this.currentMacAddress = this.messages[0].macAddress;
                    }
                    // 更新会话列表中的聊天记录数量
                    this.sessions = this.sessions.map(item => {
                        if (item.sessionId === session.sessionId) {
                            item.chatCount = this.messages.length;
                        }
                        return item;
                    })
                }
            });
        },
        handleScroll(e) {
            if (this.scrollTimer) {
                clearTimeout(this.scrollTimer);
            }

            this.scrollTimer = setTimeout(() => {
                const { scrollTop, scrollHeight, clientHeight } = e.target;
                // 当滚动到底部时加载更多
                if (scrollHeight - scrollTop <= clientHeight + 50) {
                    this.loadSessions();
                }
            }, 200);
        },
        formatTime(timestamp) {
            const date = new Date(timestamp);
            const now = new Date();
            const today = new Date(now.getFullYear(), now.getMonth(), now.getDate());
            const yesterday = new Date(today);
            yesterday.setDate(yesterday.getDate() - 1);

            const hours = date.getHours().toString().padStart(2, '0');
            const minutes = date.getMinutes().toString().padStart(2, '0');

            if (date >= today) {
                return `${this.$t('chatHistory.today')} ${hours}:${minutes}`;
            } else if (date >= yesterday) {
                return `${this.$t('chatHistory.yesterday')} ${hours}:${minutes}`;
            } else {
                const year = date.getFullYear();
                const month = (date.getMonth() + 1).toString().padStart(2, '0');
                const day = date.getDate().toString().padStart(2, '0');
                return `${year}-${month}-${day} ${hours}:${minutes}`;
            }
        },
        getAudioIconClass(message) {
            if (this.playingAudioId === message.audioId) {
                return 'el-icon-loading';
            }
            return 'el-icon-video-play';
        },
        playAudio: debounce(function(message) {
            if (this.playingAudioId === message.audioId) {
                // 如果正在播放当前音频，则停止播放
                if (this.audioElement) {
                    this.audioElement.pause();
                    this.audioElement = null;
                }
                this.playingAudioId = null;
                return;
            }

            // 停止当前正在播放的音频
            if (this.audioElement) {
                this.audioElement.pause();
                this.audioElement = null;
            }

            // 先获取音频下载ID
            this.playingAudioId = message.audioId;
            Api.agent.getAudioId(message.audioId, (res) => {
                if (res.data && res.data.data) {
                    if (!this.audioElement) {
                        this.audioElement = new Audio();
                    }
                    
                    // 使用获取到的下载ID播放音频
                    this.audioElement.src = Api.getServiceUrl() + `/agent/play/${res.data.data}`;
                    this.audioElement.onended = () => {
                        this.playingAudioId = null;
                        this.audioElement = null;
                    };

                    this.audioElement.play();
                }
            });
        }, 300),
        getUserAvatar(sessionId) {
            // 从 sessionId 中提取所有数字
            const numbers = sessionId.match(/\d+/g);
            if (!numbers) return require('@/assets/user-avatar1.png');

            // 将所有数字相加
            const sum = numbers.reduce((acc, num) => acc + parseInt(num), 0);

            // 计算模5并加1，得到1-5之间的数字
            const avatarIndex = (sum % 5) + 1;

            // 返回对应的头像图片
            return require(`@/assets/user-avatar${avatarIndex}.png`);
        },

        // 下载本会话聊天记录
        downloadCurrentSession() {
            Api.agent.getDownloadUrl(this.agentId, this.currentSessionId, (res) => {
                if (res && res.data && res.data.code === 0 && res.data.data) {
                    const uuid = res.data.data;
                    window.open(`${Api.getServiceUrl()}/agent/chat-history/download/${uuid}/current`, '_blank');
                } else {
                    this.$message.error(this.$t('chatHistory.downloadLinkFailed'));
                }
            });
        },

        // 下载本会话及前20条会话聊天记录
        downloadCurrentSessionWithPrevious() {
            Api.agent.getDownloadUrl(this.agentId, this.currentSessionId, (res) => {
                if (res && res.data && res.data.code === 0 && res.data.data) {
                    const uuid = res.data.data;
                    window.open(`${Api.getServiceUrl()}/agent/chat-history/download/${uuid}/previous`, '_blank');
                } else {
                    this.$message.error(this.$t('chatHistory.downloadLinkFailed'));
                }
            });
        }
    }
};
</script>

<style scoped>
.chat-container {
    display: flex;
    height: 100%;
}

.session-list {
    width: 250px;
    border-right: 1px solid var(--apple-divider-soft);
    overflow-y: auto;
    padding: 10px;
    background-color: var(--apple-bg);
}

.session-item {
    display: flex;
    align-items: center;
    padding: 10px;
    cursor: pointer;
    border-radius: var(--apple-radius-md);
    margin-bottom: 10px;
    transition: background-color var(--apple-duration-fast) var(--apple-ease-standard);
}

.session-item:hover {
    background-color: var(--apple-surface-2);
}

.session-item.active {
    background-color: var(--apple-brand-tint);
}

.avatar {
    width: 40px;
    height: 40px;
    border-radius: 50%;
    margin-right: 10px;
}

.session-info {
    width: calc(100% - 50px);
}

.session-time {
    font-size: 14px;
    color: var(--apple-text);
    float: left;
    height: 30px;
    line-height: 30px;
    width: calc(100% - 30px);
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
}

.message-count {
    font-size: 12px;
    color: var(--apple-surface);
    background-color: var(--apple-text-3);
    border-radius: var(--apple-radius-full);
    float: left;
    width: 22px;
    height: 22px;
    line-height: 22px;
    margin-top: 5px;
    margin-left: 5px;
    text-align: center;
}

.session-item.active .message-count {
    background-color: var(--apple-brand);
}

.chat-content {
    flex: 1;
    padding: 20px;
    overflow-y: auto;
    background-color: var(--apple-surface);
}

.message-item {
    display: flex;
    margin-bottom: 20px;
}

.message-item.user-message {
    flex-direction: row-reverse;
}

.message-content {
    max-width: 60%;
    padding: 10px 15px;
    border-radius: var(--apple-radius-md);
    background-color: var(--apple-surface-2);
    color: var(--apple-text);
    margin: 0 10px;
    text-align: left;
    line-height: 20px;
    position: relative;
    display: flex;
    align-items: center;
}

.audio-icon {
    font-size: 20px;
    cursor: pointer;
    margin: 0 5px;
    color: var(--apple-brand);
    transition: color var(--apple-duration-fast) var(--apple-ease-standard);

    &:hover {
        color: var(--apple-brand-hover);
    }
}

.user-message .message-content {
    background-color: var(--apple-brand);
    color: var(--apple-surface);
    flex-direction: row-reverse;
    box-shadow: 0 2px 8px rgba(0, 113, 227, 0.20);
}

.user-message .audio-icon {
    color: var(--apple-surface);
}

.content-wrapper {
    width: 100%;
}

.text-content {
    display: block;
    margin-bottom: 4px;
}

.tool-call-text {
    color: var(--apple-brand);
    font-family: 'Courier New', monospace;
    font-weight: var(--apple-font-weight-medium);
    font-size: 12px;
    display: block;
    margin-top: 4px;
}

.user-message .tool-call-text {
    color: rgba(255, 255, 255, 0.85);
}

.tool-message .message-content {
    background-color: var(--apple-surface-2);
}

.tool-result-wrapper {
    position: relative;
    padding-right: 20px;
}

.tool-result-collapsed {
    overflow: hidden;
    text-overflow: ellipsis;
    white-space: nowrap;
}

.tool-toggle-btn {
    position: absolute;
    right: 0;
    top: 0;
    cursor: pointer;
    color: var(--apple-brand);
    font-size: 12px;
    transition: color var(--apple-duration-fast) var(--apple-ease-standard);
}

.tool-toggle-btn:hover {
    color: var(--apple-brand-hover);
}

.loading,
.no-more {
    text-align: center;
    padding: 10px 10px 30px 10px;
    color: var(--apple-text-3);
}

.no-session-selected {
    display: flex;
    justify-content: center;
    align-items: center;
    height: 100%;
    color: var(--apple-text-3);
}

.time-divider {
    text-align: center;
    margin: 10px 0;
    color: var(--apple-text-3);
    font-size: 12px;
}

.time-divider::before,
.time-divider::after {
    content: '';
    display: inline-block;
    width: 30%;
    height: 1px;
    background-color: var(--apple-divider-soft);
    vertical-align: middle;
    margin: 0 10px;
}

.download-buttons {
    padding: 20px;
    display: flex;
    gap: 10px;
    border-top: 1px solid var(--apple-divider-soft);
    position: absolute;
    bottom: 0;
    left: 0;
    right: 0;
    background-color: var(--apple-surface);
    box-shadow: 0 -2px 8px rgba(0, 0, 0, 0.04);
}

.download-buttons .el-button {
    flex: 1;
    border-radius: var(--apple-radius-md);
}

.download-buttons .el-button--primary.is-plain {
    background-color: var(--apple-brand-tint);
    border-color: var(--apple-brand);
    color: var(--apple-brand);
}

.download-buttons .el-button--primary.is-plain:hover {
    background-color: var(--apple-brand);
    border-color: var(--apple-brand);
    color: var(--apple-surface);
}
</style>

<style>
.chat-history-dialog {
    display: flex;
    flex-direction: column;
    min-width: 700px;
    margin: 0 !important;
    position: absolute;
    top: 50%;
    left: 50%;
    transform: translate(-50%, -50%);
    height: 98vh;
    max-width: 85vw;
    border-radius: var(--apple-radius-lg);
    overflow: hidden;
    background-color: var(--apple-surface);
}

.chat-history-dialog .el-dialog__header {
    background-color: var(--apple-surface);
    border-bottom: 1px solid var(--apple-divider-soft);
}

.chat-history-dialog .el-dialog__body {
    padding: 0;
    overflow: hidden;
    height: calc(90vh - 54px);
}
</style>