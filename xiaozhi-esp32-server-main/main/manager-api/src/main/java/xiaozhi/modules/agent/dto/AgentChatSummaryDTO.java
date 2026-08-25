package xiaozhi.modules.agent.dto;

import io.swagger.v3.oas.annotations.media.Schema;
import lombok.Data;

/**
 * 智能体聊天记录总结DTO
 */
@Data
@Schema(description = "智能体聊天记录总结对象")
public class AgentChatSummaryDTO {

    @Schema(description = "会话ID")
    private String sessionId;

    @Schema(description = "智能体ID")
    private String agentId;

    @Schema(description = "总结内容")
    private String summary;

    @Schema(description = "总结状态")
    private boolean success;

    @Schema(description = "错误信息")
    private String errorMessage;

    public AgentChatSummaryDTO() {
        this.success = true;
    }

    public AgentChatSummaryDTO(String sessionId, String agentId, String summary) {
        this.sessionId = sessionId;
        this.agentId = agentId;
        this.summary = summary;
        this.success = true;
    }

    public AgentChatSummaryDTO(String sessionId, String errorMessage) {
        this.sessionId = sessionId;
        this.errorMessage = errorMessage;
        this.success = false;
    }

}