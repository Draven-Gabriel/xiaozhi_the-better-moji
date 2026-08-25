package xiaozhi.modules.device.dto;

import java.util.Map;

import jakarta.validation.constraints.NotBlank;
import lombok.Data;

@Data
public class DeviceToolsCallReqDTO {

    @NotBlank(message = "工具名称不能为空")
    private String name;

    private Map<String, Object> arguments;
}