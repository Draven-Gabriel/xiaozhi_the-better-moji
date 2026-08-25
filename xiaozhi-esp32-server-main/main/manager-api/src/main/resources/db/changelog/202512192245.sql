-- 为智能体聊天历史记录添加音频ID索引
ALTER TABLE ai_agent_chat_history ADD INDEX idx_ai_agent_chat_history_audio_id (audio_id);
