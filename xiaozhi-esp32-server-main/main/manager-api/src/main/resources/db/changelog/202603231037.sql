-- 新增仅上报聊天记录记忆模型供应器

delete from `ai_model_provider` where `id` = 'SYSTEM_Memory_mem_report_only';
delete from `ai_model_config` where `id` = 'Memory_mem_report_only';

INSERT INTO `ai_model_provider` VALUES ('SYSTEM_Memory_mem_report_only', 'Memory', 'mem_report_only', '仅上报聊天记录', '[]', 4, 1, NOW(), 1, NOW());
INSERT INTO `ai_model_config` VALUES ('Memory_mem_report_only', 'Memory', 'mem_report_only', '仅上报聊天记录', 0, 1, '{"type": "mem_report_only"}', NULL, '仅上报聊天记录，不总结记忆', 3, NULL, NULL, NULL, NULL);
