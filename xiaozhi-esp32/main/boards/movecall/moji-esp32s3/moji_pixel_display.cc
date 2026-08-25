#include "moji_pixel_display.h"

#include "application.h"
#include "board.h"

#include <esp_heap_caps.h>
#include <esp_log.h>
#include <esp_psram.h>
#include <lvgl.h>
#include "src/misc/lv_timer_private.h"

#include <cmath>
#include <cstring>
#include <ctime>

#define TAG "MojiPixelDisplay"

LV_FONT_DECLARE(font_noto_sans_basic_16_4);
LV_FONT_DECLARE(font_noto_sans_basic_14_1);

namespace {

constexpr int kScreenSize = 240;
constexpr int kFaceX = 80;
constexpr int kFaceY = 104;
constexpr int kFaceW = 80;
constexpr int kFaceH = 76;
constexpr int kTimeW = 120;
constexpr int kTimeH = 34;
constexpr int kBatteryW = 92;
constexpr int kBatteryH = 38;

lv_color_t Color(uint32_t rgb) {
    return lv_color_hex(rgb);
}

const char* PeriodOfHour(int hour) {
    if (hour >= 6 && hour < 9) return "dawn";
    if (hour >= 9 && hour < 17) return "day";
    if (hour >= 17 && hour < 20) return "dusk";
    return "night";
}

lv_color_t SkyColor(const char* period) {
    if (strcmp(period, "dawn") == 0) return Color(0xf6a1b5);
    if (strcmp(period, "day") == 0) return Color(0x8fd0f5);
    if (strcmp(period, "dusk") == 0) return Color(0xff9f68);
    return Color(0x0b1026);
}

lv_color_t ForegroundColor(const char* period) {
    // ???????????????
    return strcmp(period, "night") == 0 ? Color(0xf3e9b8) : Color(0x4a2f1d);
}

}  // namespace

MojiPixelDisplay::MojiPixelDisplay(esp_lcd_panel_io_handle_t panel_io,
                                   esp_lcd_panel_handle_t panel, int width, int height,
                                   int offset_x, int offset_y, bool mirror_x, bool mirror_y,
                                   bool swap_xy)
    : SpiLcdDisplay(panel_io, panel, width, height, offset_x, offset_y, mirror_x, mirror_y,
                    swap_xy) {}

MojiPixelDisplay::~MojiPixelDisplay() {
    if (ui_timer_ != nullptr) {
        lv_timer_delete(ui_timer_);
        ui_timer_ = nullptr;
    }
    if (bg_buf_ != nullptr) {
        heap_caps_free(bg_buf_);
        bg_buf_ = nullptr;
    }
    if (face_buf_ != nullptr) {
        heap_caps_free(face_buf_);
        face_buf_ = nullptr;
    }
    if (time_buf_ != nullptr) {
        heap_caps_free(time_buf_);
        time_buf_ = nullptr;
    }
    if (battery_buf_ != nullptr) {
        heap_caps_free(battery_buf_);
        battery_buf_ = nullptr;
    }
}

void MojiPixelDisplay::SetupUI() {
    if (setup_ui_called_) {
        return;
    }
    Display::SetupUI();
    DisplayLockGuard lock(this);

    lv_obj_t* screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, Color(0x0b1026), 0);

    bg_buf_ = static_cast<lv_color_t*>(
        heap_caps_malloc(kScreenSize * kScreenSize * sizeof(lv_color_t),
                         MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (bg_buf_ == nullptr) {
        ESP_LOGE(TAG, "Failed to allocate background canvas buffer");
        return;
    }
    bg_canvas_ = lv_canvas_create(screen);
    lv_obj_set_pos(bg_canvas_, 0, 0);
    lv_obj_set_size(bg_canvas_, kScreenSize, kScreenSize);
    lv_canvas_set_buffer(bg_canvas_, bg_buf_, kScreenSize, kScreenSize, LV_COLOR_FORMAT_RGB565);

    face_buf_ = static_cast<lv_color_t*>(
        heap_caps_malloc(kFaceW * kFaceH * sizeof(lv_color_t),
                         MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (face_buf_ == nullptr) {
        ESP_LOGE(TAG, "Failed to allocate face canvas buffer");
        return;
    }
    face_canvas_ = lv_canvas_create(screen);
    lv_obj_set_pos(face_canvas_, kFaceX, kFaceY);
    lv_obj_set_size(face_canvas_, kFaceW, kFaceH);
    lv_canvas_set_buffer(face_canvas_, face_buf_, kFaceW, kFaceH, LV_COLOR_FORMAT_RGB565);

    time_buf_ = static_cast<lv_color_t*>(
        heap_caps_malloc(kTimeW * kTimeH * sizeof(lv_color_t),
                         MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (time_buf_ != nullptr) {
        time_canvas_ = lv_canvas_create(screen);
        lv_obj_set_pos(time_canvas_, 60, 16);
        lv_obj_set_size(time_canvas_, kTimeW, kTimeH);
        lv_canvas_set_buffer(time_canvas_, time_buf_, kTimeW, kTimeH, LV_COLOR_FORMAT_RGB565);
    }

    battery_buf_ = static_cast<lv_color_t*>(
        heap_caps_malloc(kBatteryW * kBatteryH * sizeof(lv_color_t),
                         MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
    if (battery_buf_ != nullptr) {
        battery_canvas_ = lv_canvas_create(screen);
        lv_obj_set_pos(battery_canvas_, 128, 16);
        lv_obj_set_size(battery_canvas_, kBatteryW, kBatteryH);
        lv_canvas_set_buffer(battery_canvas_, battery_buf_, kBatteryW, kBatteryH,
                             LV_COLOR_FORMAT_RGB565);
    }

    date_label_ = lv_label_create(screen);
    lv_obj_set_style_text_font(date_label_, &font_noto_sans_basic_14_1, 0);
    lv_obj_set_style_text_color(date_label_, Color(0x4a2f1d), 0);
    lv_obj_set_pos(date_label_, 42, 56);
    lv_label_set_text(date_label_, "");

    status_label_ = lv_label_create(screen);
    lv_obj_set_style_text_font(status_label_, &font_noto_sans_basic_14_1, 0);
    lv_obj_set_style_text_color(status_label_, Color(0x4a2f1d), 0);
    lv_obj_set_width(status_label_, 140);
    lv_label_set_long_mode(status_label_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_pos(status_label_, 50, 206);
    lv_label_set_text(status_label_, status_text_.c_str());

    chat_label_ = lv_label_create(screen);
    lv_obj_set_style_text_font(chat_label_, &font_noto_sans_basic_14_1, 0);
    lv_obj_set_style_text_color(chat_label_, Color(0x201510), 0);
    lv_obj_set_width(chat_label_, 170);
    lv_label_set_long_mode(chat_label_, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_pos(chat_label_, 35, 188);
    lv_label_set_text(chat_label_, "");
    lv_obj_add_flag(chat_label_, LV_OBJ_FLAG_HIDDEN);

    ui_timer_ = lv_timer_create(TimerCb, 100, this);
    if (ui_timer_ != nullptr) {
        lv_timer_set_repeat_count(ui_timer_, -1);
    }

    time_t now = time(nullptr);
    struct tm* tm = localtime(&now);
    last_period_ = PeriodOfHour(tm != nullptr ? tm->tm_hour : 14);

    RedrawBackground();
    RedrawFace();
    DrawClock();
    DrawBatteryIcon();
}

void MojiPixelDisplay::DrawRect(lv_obj_t* canvas, int x, int y, int w, int h, lv_color_t color) {
    if (canvas == nullptr || w <= 0 || h <= 0) {
        return;
    }
    lv_layer_t layer;
    lv_canvas_init_layer(canvas, &layer);
    lv_draw_rect_dsc_t dsc;
    lv_draw_rect_dsc_init(&dsc);
    dsc.bg_color = color;
    dsc.bg_opa = LV_OPA_COVER;
    dsc.radius = 0;
    lv_area_t area = {static_cast<lv_coord_t>(x), static_cast<lv_coord_t>(y),
                      static_cast<lv_coord_t>(x + w - 1), static_cast<lv_coord_t>(y + h - 1)};
    lv_draw_rect(&layer, &dsc, &area);
    lv_canvas_finish_layer(canvas, &layer);
}

void MojiPixelDisplay::DrawPixelChar(lv_obj_t* canvas, char ch, int x, int y, int scale,
                                     lv_color_t color) {
    static const char* digits[10] = {
        "111101101101111",  // 0
        "010110010010111",  // 1
        "111001111100111",  // 2
        "111001011001111",  // 3
        "101101111001001",  // 4
        "111100111001111",  // 5
        "111100111101111",  // 6
        "111001001001001",  // 7
        "111101111101111",  // 8
        "111101111001111",  // 9
    };
    if (ch == ':') {
        static const char* colon = "010010";
        for (int r = 0; r < 6; ++r) {
            if (colon[r] == '1') {
                lv_canvas_set_px(canvas, x, y + r * scale, color, LV_OPA_COVER);
                lv_canvas_set_px(canvas, x + scale - 1, y + r * scale, color, LV_OPA_COVER);
            }
        }
        return;
    }
    if (ch == '%') {
        static const char* pct = "110010010010011";
        for (int r = 0; r < 5; ++r) {
            for (int c = 0; c < 3; ++c) {
                if (pct[r * 3 + c] == '1') {
                    lv_canvas_set_px(canvas, x + c * scale, y + r * scale, color, LV_OPA_COVER);
                }
            }
        }
        return;
    }
    if (ch == '-') {
        for (int c = 0; c < 3; ++c) {
            lv_canvas_set_px(canvas, x + c * scale, y + 2 * scale, color, LV_OPA_COVER);
        }
        return;
    }
    if (ch == '?') {
        static const char* q = "111001010010000010";
        for (int r = 0; r < 6; ++r) {
            for (int c = 0; c < 3; ++c) {
                if (q[r * 3 + c] == '1') {
                    lv_canvas_set_px(canvas, x + c * scale, y + r * scale, color, LV_OPA_COVER);
                }
            }
        }
        return;
    }
    if (ch == 'Z') {
        static const char* z = "111001001001111";
        for (int r = 0; r < 5; ++r) {
            for (int c = 0; c < 3; ++c) {
                if (z[r * 3 + c] == '1') {
                    lv_canvas_set_px(canvas, x + c * scale, y + r * scale, color, LV_OPA_COVER);
                }
            }
        }
        return;
    }
    if (ch >= '0' && ch <= '9') {
        const char* g = digits[ch - '0'];
        for (int r = 0; r < 5; ++r) {
            for (int c = 0; c < 3; ++c) {
                if (g[r * 3 + c] == '1') {
                    lv_canvas_set_px(canvas, x + c * scale, y + r * scale, color, LV_OPA_COVER);
                }
            }
        }
    }
}

void MojiPixelDisplay::DrawPixelText(lv_obj_t* canvas, const std::string& text, int x, int y,
                                     int scale, lv_color_t color) {
    int cx = x;
    for (char ch : text) {
        if (ch == ' ') {
            cx += scale * 2;
            continue;
        }
        DrawPixelChar(canvas, ch, cx, y, scale, color);
        cx += scale * (ch == ':' ? 2 : 4);
    }
}

void MojiPixelDisplay::DrawClock() {
    if (time_canvas_ == nullptr) {
        return;
    }
    time_t now = time(nullptr);
    struct tm* tm = localtime(&now);
    int hour = tm != nullptr ? tm->tm_hour : 14;
    const char* period = PeriodOfHour(hour);
    lv_canvas_fill_bg(time_canvas_, SkyColor(period), LV_OPA_COVER);

    char hh[8] = "--", mm[8] = "--";
    if (tm != nullptr && tm->tm_year >= 2025 - 1900) {
        snprintf(hh, sizeof(hh), "%02d", tm->tm_hour);
        snprintf(mm, sizeof(mm), "%02d", tm->tm_min);
    }
    const lv_color_t fg = ForegroundColor(period);
    std::string clock = std::string(hh) + ":" + mm;
    DrawPixelText(time_canvas_, clock, 0, 8, 2, fg);

    if (date_label_ != nullptr && tm != nullptr && tm->tm_year >= 2025 - 1900) {
        char date[32];
        snprintf(date, sizeof(date), "%02d-%02d", tm->tm_mon + 1, tm->tm_mday);
        lv_label_set_text(date_label_, date);
    }
}

void MojiPixelDisplay::DrawBatteryIcon() {
    if (battery_canvas_ == nullptr) {
        return;
    }
    time_t now = time(nullptr);
    struct tm* tm = localtime(&now);
    int hour = tm != nullptr ? tm->tm_hour : 14;
    const char* period = PeriodOfHour(hour);
    lv_canvas_fill_bg(battery_canvas_, SkyColor(period), LV_OPA_COVER);

    if (!has_battery_) {
        // 无真实 ADC 数据：不画电池图标，避免误导
        return;
    }

    const lv_color_t outline = ForegroundColor(period);
    const int x = 14;
    const int y = 4;
    DrawRect(battery_canvas_, x - 1, y - 1, 26, 13, outline);
    DrawRect(battery_canvas_, x + 1, y + 1, 22, 9, Color(0x000000));
    DrawRect(battery_canvas_, x + 25, y + 3, 3, 5, outline);

    lv_color_t fill = charging_ ? Color(0x34e07a) : (battery_level_ < 20 ? Color(0xff4d5e)
                                                                       : Color(0x34e07a));
    if (charging_) {
        DrawRect(battery_canvas_, x + 3, y + 3, 18, 5, fill);
        DrawPixelText(battery_canvas_, "!", x + 10, y + 2, 1, Color(0x111111));
    } else {
        int pct = battery_level_ < 0 ? 0 : (battery_level_ > 100 ? 100 : battery_level_);
        int fill_w = 18 * pct / 100;
        if (fill_w > 0) {
            DrawRect(battery_canvas_, x + 3, y + 3, fill_w, 5, fill);
        }
    }
    char buf[16];
    snprintf(buf, sizeof(buf), "%d%%", battery_level_);
    DrawPixelText(battery_canvas_, buf, 0, 20, 2, fill);
}

void MojiPixelDisplay::RedrawBackground() {
    if (bg_canvas_ == nullptr) {
        return;
    }
    lv_canvas_fill_bg(bg_canvas_, Color(0x000000), LV_OPA_COVER);

    time_t now = time(nullptr);
    struct tm* tm = localtime(&now);
    int hour = tm != nullptr ? tm->tm_hour : 14;
    const char* period = PeriodOfHour(hour);

    static const uint32_t kNight[5] = {0x0b1026, 0x141b3d, 0x1b2348, 0x232b52, 0x2c315a};
    static const uint32_t kDawn[5] = {0xf6a1b5, 0xf9bca3, 0xf9d2a4, 0xe8d8ae, 0xcfe0ba};
    static const uint32_t kDay[5] = {0x8fd0f5, 0xa5dcf7, 0xbde7f5, 0xd8f0f0, 0xe8f6df};
    static const uint32_t kDusk[5] = {0xff9f68, 0xffb47b, 0xffca93, 0xd8a578, 0x9a8a70};
    const uint32_t* pal = kNight;
    if (strcmp(period, "dawn") == 0) pal = kDawn;
    else if (strcmp(period, "day") == 0) pal = kDay;
    else if (strcmp(period, "dusk") == 0) pal = kDusk;

    for (int i = 0; i < 5; ++i) {
        DrawRect(bg_canvas_, 0, i * 48, kScreenSize, 48, Color(pal[i]));
    }

    if (strcmp(period, "night") == 0) {
        DrawRect(bg_canvas_, 180, 46, 16, 16, Color(0xf3e9b8));
        DrawRect(bg_canvas_, 184, 50, 8, 8, Color(0xd8d3aa));
    } else {
        DrawRect(bg_canvas_, 178, 48, 18, 18, Color(0xfff0a8));
    }

    const uint32_t hill1 = strcmp(period, "night") == 0 ? 0x141b38
                         : strcmp(period, "dusk") == 0 ? 0x7a6a52
                         : strcmp(period, "dawn") == 0 ? 0x9cc09a
                                                       : 0x69a576;
    const uint32_t hill2 = strcmp(period, "night") == 0 ? 0x0d1229
                         : strcmp(period, "dusk") == 0 ? 0x5f5545
                         : strcmp(period, "dawn") == 0 ? 0x7ba47c
                                                       : 0x4f9060;
    for (int x = 0; x < kScreenSize; x += 8) {
        int y1 = 196 - static_cast<int>(std::abs(std::sin(x * 0.018) * 38 +
                                                 std::sin(x * 0.04) * 12));
        DrawRect(bg_canvas_, x, y1, 8, kScreenSize - y1, Color(hill1));
        int y2 = 218 - static_cast<int>(std::abs(std::cos(x * 0.024) * 22));
        DrawRect(bg_canvas_, x, y2, 8, kScreenSize - y2, Color(hill2));
    }

    const lv_color_t cloud = Color(0xf9f2dd);
    for (int i = 0; i < 3; ++i) {
        int cx = (i * 92 + static_cast<int>(anim_phase_ / 20)) % (kScreenSize + 80) - 40;
        int cy = 30 + i * 14;
        DrawRect(bg_canvas_, cx, cy, 34, 12, cloud);
        DrawRect(bg_canvas_, cx + 8, cy - 8, 20, 8, cloud);
    }

    const lv_color_t fg = ForegroundColor(period);
    if (date_label_ != nullptr) lv_obj_set_style_text_color(date_label_, fg, 0);
    if (status_label_ != nullptr) lv_obj_set_style_text_color(status_label_, fg, 0);
    if (chat_label_ != nullptr) lv_obj_set_style_text_color(chat_label_, fg, 0);

    DrawClock();
    DrawBatteryIcon();
}

void MojiPixelDisplay::RedrawFace() {
    if (face_canvas_ == nullptr) {
        return;
    }
    lv_canvas_fill_bg(face_canvas_, Color(0x000000), LV_OPA_COVER);
    DrawRect(face_canvas_, 0, 0, kFaceW, kFaceH, Color(0xffd9a0));
    DrawRect(face_canvas_, 0, 0, kFaceW, 3, Color(0x7a4a2b));
    DrawRect(face_canvas_, 0, kFaceH - 3, kFaceW, 3, Color(0x7a4a2b));
    DrawRect(face_canvas_, 0, 0, 3, kFaceH, Color(0x7a4a2b));
    DrawRect(face_canvas_, kFaceW - 3, 0, 3, kFaceH, Color(0x7a4a2b));

    const lv_color_t eye = Color(0x201510);
    const lv_color_t mouth = Color(0x7a3b2b);
    const int eye_y = 20;
    const int mouth_y = 52;

    if (emotion_ == "happy") {
        DrawRect(face_canvas_, 18, eye_y, 13, 13, eye);
        DrawRect(face_canvas_, 49, eye_y, 13, 13, eye);
        DrawRect(face_canvas_, 27, mouth_y + 4, 26, 3, mouth);
    } else if (emotion_ == "talking") {
        DrawRect(face_canvas_, 18, eye_y, 12, 12, eye);
        DrawRect(face_canvas_, 50, eye_y, 12, 12, eye);
        int mh = 7 + (anim_phase_ % 6);
        DrawRect(face_canvas_, 29, mouth_y + 9 - mh / 2, 22, mh, mouth);
    } else if (emotion_ == "thinking") {
        DrawRect(face_canvas_, 22, eye_y, 11, 11, eye);
        DrawRect(face_canvas_, 47, eye_y, 11, 11, eye);
        DrawRect(face_canvas_, 27, mouth_y, 26, 5, mouth);
        DrawPixelText(face_canvas_, "???", 56, 8, 2, Color(0xd95c2b));
    } else if (emotion_ == "listening") {
        DrawRect(face_canvas_, 20, eye_y, 12, 12, eye);
        DrawRect(face_canvas_, 48, eye_y, 12, 12, eye);
        int bars[3] = {8, 16, 11};
        int bx = 27;
        for (int i = 0; i < 3; ++i) {
            int bh = bars[i] + (anim_phase_ % 5);
            DrawRect(face_canvas_, bx, mouth_y + 12 - bh, 5, bh, mouth);
            bx += 8;
        }
    } else if (emotion_ == "sleeping") {
        DrawRect(face_canvas_, 16, eye_y + 4, 16, 4, eye);
        DrawRect(face_canvas_, 48, eye_y + 4, 16, 4, eye);
        DrawRect(face_canvas_, 27, mouth_y + 1, 22, 5, mouth);
        DrawPixelText(face_canvas_, "Z", 58, 6, 3, Color(0xd95c2b));
    } else {  // idle
        int eh = (anim_phase_ % 36 == 0 || anim_phase_ % 36 == 1) ? 3 : 12;
        DrawRect(face_canvas_, 20, eye_y + (12 - eh), 12, eh, eye);
        DrawRect(face_canvas_, 48, eye_y + (12 - eh), 12, eh, eye);
        DrawRect(face_canvas_, 27, mouth_y, 26, 6, mouth);
    }
}

void MojiPixelDisplay::TimerCb(lv_timer_t* timer) {
    auto* self = static_cast<MojiPixelDisplay*>(timer->user_data);
    if (self == nullptr) {
        return;
    }
    ++self->anim_phase_;

    // ?????????10fps
    self->RedrawFace();

    // ????????????????????
    if (self->anim_phase_ % 10 == 0) {
        time_t now = time(nullptr);
        struct tm* tm = localtime(&now);
        const char* period = PeriodOfHour(tm != nullptr ? tm->tm_hour : 14);
        if (self->last_period_ != period) {
            // ???????/??/??/??????????
            self->last_period_ = period;
            self->RedrawBackground();
        } else {
            self->DrawClock();
        }
    }

    if (self->battery_dirty_) {
        self->battery_dirty_ = false;
        self->DrawBatteryIcon();
    }
}

void MojiPixelDisplay::SetStatus(const char* status) {
    status_text_ = status != nullptr ? status : "";
    if (status_label_ != nullptr) {
        DisplayLockGuard lock(this);
        lv_label_set_text(status_label_, status_text_.c_str());
    }
}

void MojiPixelDisplay::SetTheme(Theme* theme) {
    // ?????????????? light/dark ?????????????????
    (void)theme;
}

void MojiPixelDisplay::ShowNotification(const char* notification, int duration_ms) {
    (void)duration_ms;
    SetStatus(notification != nullptr ? notification : "");
}

void MojiPixelDisplay::SetEmotion(const char* emotion) {
    const char* raw = emotion != nullptr ? emotion : "idle";
    if (strcmp(raw, "??") == 0 || strcmp(raw, "??") == 0 || strcmp(raw, "happy") == 0) {
        emotion_ = "happy";
    } else if (strcmp(raw, "??") == 0 || strcmp(raw, "talking") == 0) {
        emotion_ = "talking";
    } else if (strcmp(raw, "??") == 0 || strcmp(raw, "thinking") == 0) {
        emotion_ = "thinking";
    } else if (strcmp(raw, "??") == 0 || strcmp(raw, "listening") == 0) {
        emotion_ = "listening";
    } else if (strcmp(raw, "??") == 0 || strcmp(raw, "sleeping") == 0) {
        emotion_ = "sleeping";
    } else if (strcmp(raw, "neutral") == 0 || strcmp(raw, "idle") == 0) {
        emotion_ = "idle";
    } else {
        emotion_ = raw;
    }
    // ???????? 100ms ? LVGL ???????????/?? CPU
}

void MojiPixelDisplay::SetChatMessage(const char* role, const char* content) {
    (void)role;
    chat_text_ = content != nullptr ? content : "";
    if (chat_label_ == nullptr) {
        return;
    }
    DisplayLockGuard lock(this);
    lv_label_set_text(chat_label_, chat_text_.c_str());
    if (chat_text_.empty()) {
        lv_obj_add_flag(chat_label_, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_remove_flag(chat_label_, LV_OBJ_FLAG_HIDDEN);
    }
}

void MojiPixelDisplay::ClearChatMessages() {
    chat_text_.clear();
    if (chat_label_ != nullptr) {
        DisplayLockGuard lock(this);
        lv_label_set_text(chat_label_, "");
        lv_obj_add_flag(chat_label_, LV_OBJ_FLAG_HIDDEN);
    }
}

void MojiPixelDisplay::UpdateStatusBar(bool update_all) {
    (void)update_all;
    auto& board = Board::GetInstance();
    int level = 0;
    bool charging = false;
    bool discharging = false;
    has_battery_ = board.GetBatteryLevel(level, charging, discharging);
    battery_level_ = level;
    charging_ = charging;
    if (has_battery_ != (last_battery_level_ >= 0) || level != last_battery_level_ ||
        charging != last_charging_) {
        last_battery_level_ = level;
        last_charging_ = charging;
        battery_dirty_ = true;
    }
}

void MojiPixelDisplay::SetPowerSaveMode(bool on) {
    if (on) {
        SetEmotion("sleeping");
    } else {
        SetEmotion("idle");
    }
}
