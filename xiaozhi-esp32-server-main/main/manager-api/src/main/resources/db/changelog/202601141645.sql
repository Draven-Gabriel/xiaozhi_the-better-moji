-- 批量清理 ai_model_provider 中的 sample_rate 字段定义
UPDATE `ai_model_provider` ap
JOIN (
    SELECT 
        id,
        JSON_ARRAYAGG(
            JSON_OBJECT('key', jt.k, 'label', jt.l, 'type', jt.t)
        ) AS new_fields
    FROM `ai_model_provider`,
         JSON_TABLE(`fields`, '$[*]' COLUMNS (
             k VARCHAR(50) PATH '$.key',
             l VARCHAR(100) PATH '$.label',
             t VARCHAR(20) PATH '$.type'
         )) AS jt
    WHERE `model_type` = 'TTS' 
      AND jt.k != 'sample_rate'
    GROUP BY id
) filtered ON ap.id = filtered.id
SET ap.fields = filtered.new_fields;

-- 清理 config_json 顶层的 sample_rate
UPDATE `ai_model_config`
SET `config_json` = JSON_REMOVE(`config_json`, '$.sample_rate')
WHERE `model_type` = 'TTS'
  AND JSON_EXTRACT(`config_json`, '$.sample_rate') IS NOT NULL;

-- 清理Minimax流式TTS的sample_rate参数（位于audio_setting内部）
UPDATE `ai_model_config` SET 
`config_json` = JSON_SET(`config_json`, '$.audio_setting', JSON_REMOVE(JSON_EXTRACT(`config_json`, '$.audio_setting'), '$.sample_rate'))
WHERE `id` = 'TTS_MinimaxStreamTTS'
AND JSON_EXTRACT(`config_json`, '$.audio_setting.sample_rate') IS NOT NULL;