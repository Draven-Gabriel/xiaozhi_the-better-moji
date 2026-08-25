-- 更新小智参数中的默认采样率从 16000 改为 24000
UPDATE `sys_params`
SET `param_value` = '{
  "type": "hello",
  "version": 1,
  "transport": "websocket",
  "audio_params": {
    "format": "opus",
    "sample_rate": 24000,
    "channels": 1,
    "frame_duration": 60
  }
}'
WHERE `id` = 309 AND `param_code` = 'xiaozhi';
