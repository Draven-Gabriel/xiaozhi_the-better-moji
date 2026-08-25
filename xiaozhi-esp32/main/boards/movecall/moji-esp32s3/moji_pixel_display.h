#pragma once

#include "lcd_display.h"

#include <lvgl.h>

#include <string>

class MojiPixelDisplay : public SpiLcdDisplay {
public:
    MojiPixelDisplay(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_handle_t panel,
                     int width, int height, int offset_x, int offset_y, bool mirror_x,
                     bool mirror_y, bool swap_xy);
    ~MojiPixelDisplay() override;

    void SetupUI() override;
    void SetTheme(Theme* theme) override;
    void SetStatus(const char* status) override;
    void ShowNotification(const char* notification, int duration_ms = 3000) override;
    void SetEmotion(const char* emotion) override;
    void SetChatMessage(const char* role, const char* content) override;
    void ClearChatMessages() override;
    void UpdateStatusBar(bool update_all = false) override;
    void SetPowerSaveMode(bool on) override;

private:
    lv_obj_t* bg_canvas_ = nullptr;
    lv_color_t* bg_buf_ = nullptr;
    lv_obj_t* face_canvas_ = nullptr;
    lv_color_t* face_buf_ = nullptr;
    lv_obj_t* time_canvas_ = nullptr;
    lv_color_t* time_buf_ = nullptr;
    lv_obj_t* battery_canvas_ = nullptr;
    lv_color_t* battery_buf_ = nullptr;
    lv_obj_t* date_label_ = nullptr;
    lv_obj_t* status_label_ = nullptr;
    lv_obj_t* chat_label_ = nullptr;
    lv_timer_t* ui_timer_ = nullptr;

    std::string emotion_ = "idle";
    std::string status_text_ = "??";
    std::string chat_text_;
    std::string last_period_;
    uint32_t anim_phase_ = 0;
    int battery_level_ = -1;
    bool charging_ = false;
    bool has_battery_ = false;
    int last_battery_level_ = -1;
    bool last_charging_ = false;
    bool battery_dirty_ = true;

    void RedrawBackground();
    void RedrawFace();
    void DrawRect(lv_obj_t* canvas, int x, int y, int w, int h, lv_color_t color);
    void DrawPixelChar(lv_obj_t* canvas, char ch, int x, int y, int scale, lv_color_t color);
    void DrawPixelText(lv_obj_t* canvas, const std::string& text, int x, int y, int scale,
                       lv_color_t color);
    void DrawClock();
    void DrawBatteryIcon();
    static void TimerCb(lv_timer_t* timer);
};
