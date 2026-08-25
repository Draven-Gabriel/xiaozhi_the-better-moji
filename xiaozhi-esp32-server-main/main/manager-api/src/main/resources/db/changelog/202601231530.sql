-- 更新HuoshanDoubleStreamTTS供应器配置，将分散的参数改为JSON字典配置
-- 将 speech_rate, loudness_rate, pitch, emotion, emotion_scale 等参数整合为 audio_params, additions, mix_speaker 三个JSON字典

UPDATE `ai_model_provider`
SET `fields` = '[
  {"key": "ws_url", "type": "string", "label": "WebSocket地址"},
  {"key": "appid", "type": "string", "label": "应用ID"},
  {"key": "access_token", "type": "string", "label": "访问令牌"},
  {"key": "resource_id", "type": "string", "label": "资源ID"},
  {"key": "speaker", "type": "string", "label": "默认音色"},
  {"key": "enable_ws_reuse", "type": "boolean", "label": "是否开启链接复用", "default": true},
  {"key": "audio_params", "type": "dict", "label": "音频输出配置"},
  {"key": "additions", "type": "dict", "label": "高级文本处理配置"},
  {"key": "mix_speaker", "type": "dict", "label": "混音控制配置"}
]'
WHERE `id` = 'SYSTEM_TTS_HSDSTTS';

-- 更新现有配置，将旧的分散参数迁移到新的JSON字典结构
UPDATE `ai_model_config`
SET `config_json` = JSON_SET(
    `config_json`,
    '$.audio_params', JSON_OBJECT(
        'speech_rate', CAST(COALESCE(NULLIF(JSON_UNQUOTE(JSON_EXTRACT(`config_json`, '$.speech_rate')), ''), '0') AS SIGNED),
        'loudness_rate', CAST(COALESCE(NULLIF(JSON_UNQUOTE(JSON_EXTRACT(`config_json`, '$.loudness_rate')), ''), '0') AS SIGNED)
    ),
    '$.additions', JSON_OBJECT(
        'aigc_metadata', JSON_OBJECT(),
        'cache_config', JSON_OBJECT(),
        'post_process', JSON_OBJECT(
            'pitch', CAST(COALESCE(NULLIF(JSON_UNQUOTE(JSON_EXTRACT(`config_json`, '$.pitch')), ''), '0') AS SIGNED)
        )
    ),
    '$.mix_speaker', JSON_OBJECT()
)
WHERE `id` = 'TTS_HuoshanDoubleStreamTTS';

-- 删除旧的分散参数字段
UPDATE `ai_model_config`
SET `config_json` = JSON_REMOVE(
    `config_json`,
    '$.speech_rate',
    '$.loudness_rate',
    '$.pitch',
    '$.emotion',
    '$.emotion_scale'
)
WHERE `id` = 'TTS_HuoshanDoubleStreamTTS';

-- 更新文档链接和备注说明
UPDATE `ai_model_config` SET
`doc_link` = 'https://www.volcengine.com/docs/6561/1329505',
`remark` = '火山引擎双向流式TTS配置说明：
1. 访问 https://www.volcengine.com/ 注册并开通火山引擎账号
2. 访问 https://console.volcengine.com/speech/service/10007 开通语音合成大模型，购买音色
3. 在页面底部获取appid和access_token
4. 资源ID固定为：volc.service_type.10029（大模型语音合成及混音）
5. 链接复用：开启WebSocket连接复用，默认true减少链接损耗（注意：复用后设备处于聆听状态时空闲链接会占并发数）

详细参数文档：https://www.volcengine.com/docs/6561/1329505
【audio_params】音频输出配置 - 用户可自定义添加火山引擎支持的任何音频参数
  - speech_rate: 语速(-50~100)，默认0
  - loudness_rate: 音量(-50~100)，默认0
  - emotion: 情感类型（仅部分音色支持），可选值：neutral、happy、sad、angry、fearful、disgusted、surprised
  - emotion_scale: 情感强度(1~5)，默认4
  示例：{"speech_rate": 10, "loudness_rate": 5, "emotion": "happy", "emotion_scale": 4}

【additions】高级文本处理配置 - 用户可自定义添加火山引擎支持的任何高级参数
  - post_process.pitch: 音高(-12~12)，默认0
  - aigc_metadata: AIGC元数据配置
  - cache_config: 缓存配置
  示例：{"post_process": {"pitch": 2}, "aigc_metadata": {}, "cache_config": {}}

【mix_speaker】混音控制配置 - 多音色混合（仅 TTS 1.0）
  示例：
    {"speakers": [
      {"source_speaker": "zh_male_bvlazysheep","mix_factor": 0.3}, 
      {"source_speaker": "BV120_streaming","mix_factor": 0.3}, 
      {"source_speaker": "zh_male_ahu_conversation_wvae_bigtts","mix_factor": 0.4}
    ]}

注意：
- 多情感音色参数（emotion、emotion_scale）仅部分音色支持
- 相关音色列表：https://www.volcengine.com/docs/6561/1257544
- 用户可根据火山引擎API文档自行添加更多参数
- 混音功能主要适用于豆包语音合成模型1.0的音色，使用时需要将req_params.speaker设置为custom_mix_bigtts
'
WHERE `id` = 'TTS_HuoshanDoubleStreamTTS';
