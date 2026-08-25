-- 添加阿里百炼Paraformer实时语音识别服务配置
delete from `ai_model_provider` where id = 'SYSTEM_ASR_AliyunBLStream';
INSERT INTO `ai_model_provider` (`id`, `model_type`, `provider_code`, `name`, `fields`, `sort`, `creator`, `create_date`, `updater`, `update_date`) VALUES
('SYSTEM_ASR_AliyunBLStream', 'ASR', 'aliyunbl_stream', '阿里百炼Paraformer实时语音识别', '[{"key":"api_key","label":"API密钥","type":"password"},{"key":"model","label":"模型名称","type":"string"},{"key":"format","label":"音频格式","type":"string"},{"key":"sample_rate","label":"采样率","type":"number"},{"key":"output_dir","label":"输出目录","type":"string"}]', 18, 1, NOW(), 1, NOW());

delete from `ai_model_config` where id = 'ASR_AliyunBLStream';
INSERT INTO `ai_model_config` VALUES ('ASR_AliyunBLStream', 'ASR', 'AliyunBLStream', '阿里百炼Paraformer实时语音识别', 0, 1, '{"type": "aliyunbl_stream", "api_key": "", "model": "paraformer-realtime-v2", "format": "pcm", "sample_rate": 16000, "disfluency_removal_enabled": false, "semantic_punctuation_enabled": false, "max_sentence_silence": 200, "multi_threshold_mode_enabled": false, "punctuation_prediction_enabled": true, "inverse_text_normalization_enabled": true, "output_dir": "tmp/"}', 'https://help.aliyun.com/zh/model-studio/websocket-for-paraformer-real-time-service', '支持多语言、热词定制、语义断句等高级功能', 21, NULL, NULL, NULL, NULL);

-- 更新阿里百炼Paraformer模型配置的说明文档
UPDATE `ai_model_config` SET
`doc_link` = 'https://help.aliyun.com/zh/model-studio/websocket-for-paraformer-real-time-service',
`remark` = '阿里百炼Paraformer实时语音识别配置说明：
1. 登录阿里云百炼平台 https://bailian.console.aliyun.com/
2. 创建API-KEY https://bailian.console.aliyun.com/#/api-key
3. 支持模型：paraformer-realtime-v2(推荐)、paraformer-realtime-8k-v2、paraformer-realtime-v1、paraformer-realtime-8k-v1
4. 功能特性：
   - 多语言支持(中文含方言、英文、日语、韩语、德语、法语、俄语)
   - 热词定制(vocabulary_id参数)，详细说明请参考：https://help.aliyun.com/zh/model-studio/custom-hot-words?
   - 语义断句/VAD断句(semantic_punctuation_enabled参数)
   - 自动标点符号、ITN、过滤语气词等
5. 参数说明：
   - model: 模型名称，推荐paraformer-realtime-v2
   - sample_rate: 采样率(Hz)，v2支持任意采样率，v1仅支持16000，8k版本仅支持8000
   - semantic_punctuation_enabled: false为VAD断句(低延迟)，true为语义断句(高准确)
   - max_sentence_silence: VAD断句静音时长阈值(200-6000ms)
' WHERE `id` = 'ASR_AliyunBLStream';


-- 更新豆包流式ASR供应器，增加配置
delete from `ai_model_provider` where id = 'SYSTEM_ASR_DoubaoStreamASR';
INSERT INTO `ai_model_provider` (`id`, `model_type`, `provider_code`, `name`, `fields`, `sort`, `creator`, `create_date`, `updater`, `update_date`) VALUES
('SYSTEM_ASR_DoubaoStreamASR', 'ASR', 'doubao_stream', '火山引擎语音识别(流式)', '[{"key":"appid","label":"应用ID","type":"string"},{"key":"access_token","label":"访问令牌","type":"string"},{"key":"cluster","label":"集群","type":"string"},{"key":"boosting_table_name","label":"热词文件名称","type":"string"},{"key":"correct_table_name","label":"替换词文件名称","type":"string"},{"key":"output_dir","label":"输出目录","type":"string"},{"key":"end_window_size","label":"静音判定时长(ms)","type":"number"},{"key":"enable_multilingual","label":"是否开启多语种识别模式","type":"boolean"},{"key":"language","label":"指定语言编码","type":"string"}]', 3, 1, NOW(), 1, NOW());
UPDATE `ai_model_config` SET 
`remark` = '豆包ASR配置说明：
1. 豆包ASR和豆包(流式)ASR的区别是：豆包ASR是按次收费，豆包(流式)ASR是按时收费
2. 一般来说按次收费的更便宜，但是豆包(流式)ASR使用了大模型技术，效果更好
3. 需要在火山引擎控制台创建应用并获取appid和access_token
4. 支持中文语音识别
5. 需要网络连接
6. 输出文件保存在tmp/目录
申请步骤：
1. 访问 https://console.volcengine.com/speech/app
2. 创建新应用
3. 获取appid和access_token
4. 填入配置文件中
如需设置热词，请参考：https://www.volcengine.com/docs/6561/155738
如开启多语种识别模式，请设置language当该键为空时，该模型支持中英文、上海话、闽南语，四川、陕西、粤语识别。其他语种请参考：https://www.volcengine.com/docs/6561/1354869
' WHERE `id` = 'ASR_DoubaoStreamASR';

-- 更新豆包流式ASR模型配置，增加enable_multilingual默认值
UPDATE `ai_model_config` SET
`config_json` = JSON_SET(
    `config_json`, 
    '$.enable_multilingual', false,
    '$.language', 'zh-CN'
)
WHERE `id` = 'ASR_DoubaoStreamASR' 
AND JSON_EXTRACT(`config_json`, '$.enable_multilingual') IS NULL 
AND JSON_EXTRACT(`config_json`, '$.language') IS NULL;


-- 更新HuoshanDoubleStreamTTS供应器配置，增加多情感音色参数
UPDATE `ai_model_provider`
SET `fields` = '[{"key": "ws_url", "type": "string", "label": "WebSocket地址"}, {"key": "appid", "type": "string", "label": "应用ID"}, {"key": "access_token", "type": "string", "label": "访问令牌"}, {"key": "resource_id", "type": "string", "label": "资源ID"}, {"key": "speaker", "type": "string", "label": "默认音色"}, {"key": "enable_ws_reuse", "type": "boolean", "label": "是否开启链接复用", "default": true}, {"key": "speech_rate", "type": "number", "label": "语速(-50~100)"}, {"key": "loudness_rate", "type": "number", "label": "音量(-50~100)"}, {"key": "pitch", "type": "number", "label": "音高(-12~12)"}, {"key": "emotion_scale", "type": "number", "label": "情感强度(1-5)"}, {"key": "emotion", "type": "string", "label": "情感类型"}]'
WHERE `id` = 'SYSTEM_TTS_HSDSTTS';

-- 更新默认值
UPDATE `ai_model_config` SET
`config_json` = JSON_SET(
    `config_json`,
    '$.emotion', 'neutral',
    '$.emotion_scale', 4
)
WHERE `id` = 'TTS_HuoshanDoubleStreamTTS'
AND JSON_EXTRACT(`config_json`, '$.emotion') IS NULL 
AND JSON_EXTRACT(`config_json`, '$.emotion_scale') IS NULL;

-- 增加文档链接和备注
UPDATE `ai_model_config` SET 
`doc_link` = 'https://console.volcengine.com/speech/service/10007',
`remark` = '火山引擎语音合成服务配置说明：
1. 访问 https://www.volcengine.com/ 注册并开通火山引擎账号
2. 访问 https://console.volcengine.com/speech/service/10007 开通语音合成大模型，购买音色
3. 在页面底部获取appid和access_token
5. 资源ID固定为：volc.service_type.10029（大模型语音合成及混音）
6. 链接复用：开启WebSocket连接复用，默认true减少链接损耗（注意：复用后设备处于聆听状态时空闲链接会占并发数）
7. 语速：-50~100，可不填，正常默认值0，可填-50~100
8. 音量：-50~100，可不填，正常默认值0，可填-50~100
9. 音高：-12~12，可不填，正常默认值0，可填-12~12
10. 多情感参数（当前仅部分音色支持设置情感）：
   相关音色列表：https://www.volcengine.com/docs/6561/1257544
    - emotion_scale：情感强度，可选值为：1~5，默认值为4
    - emotion：情感类型，可选值为：neutral、happy、sad、angry、fearful、disgusted、surprised
' WHERE `id` = 'TTS_HuoshanDoubleStreamTTS';
