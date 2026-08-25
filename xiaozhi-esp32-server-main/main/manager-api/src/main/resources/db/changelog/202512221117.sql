-- 更新豆包流式ASR供应器，增加end_window_size配置
delete from `ai_model_provider` where id = 'SYSTEM_ASR_DoubaoStreamASR';
INSERT INTO `ai_model_provider` (`id`, `model_type`, `provider_code`, `name`, `fields`, `sort`, `creator`, `create_date`, `updater`, `update_date`) VALUES
('SYSTEM_ASR_DoubaoStreamASR', 'ASR', 'doubao_stream', '火山引擎语音识别(流式)', '[{"key":"appid","label":"应用ID","type":"string"},{"key":"access_token","label":"访问令牌","type":"string"},{"key":"cluster","label":"集群","type":"string"},{"key":"boosting_table_name","label":"热词文件名称","type":"string"},{"key":"correct_table_name","label":"替换词文件名称","type":"string"},{"key":"output_dir","label":"输出目录","type":"string"},{"key":"end_window_size","label":"静音判定时长(ms)","type":"number"}]', 3, 1, NOW(), 1, NOW());


-- 更新豆包流式ASR模型配置，增加end_window_size默认值
UPDATE `ai_model_config` SET
`config_json` = JSON_SET(`config_json`, '$.end_window_size', 200)
WHERE `id` = 'ASR_DoubaoStreamASR' AND JSON_EXTRACT(`config_json`, '$.end_window_size') IS NULL;
