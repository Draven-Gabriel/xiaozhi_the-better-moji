#ifndef _BOARD_CONFIG_H_
#define _BOARD_CONFIG_H_

// Movecall Moji configuration

#include <driver/gpio.h>

#define AUDIO_INPUT_SAMPLE_RATE  24000
#define AUDIO_OUTPUT_SAMPLE_RATE 24000

#define AUDIO_I2S_GPIO_MCLK GPIO_NUM_6
#define AUDIO_I2S_GPIO_WS GPIO_NUM_12
#define AUDIO_I2S_GPIO_BCLK GPIO_NUM_14
#define AUDIO_I2S_GPIO_DIN  GPIO_NUM_13
#define AUDIO_I2S_GPIO_DOUT GPIO_NUM_11

#define AUDIO_CODEC_PA_PIN       GPIO_NUM_9
#define AUDIO_CODEC_I2C_SDA_PIN  GPIO_NUM_5
#define AUDIO_CODEC_I2C_SCL_PIN  GPIO_NUM_4
#define AUDIO_CODEC_ES8311_ADDR  ES8311_CODEC_DEFAULT_ADDR

#define BUILTIN_LED_GPIO        GPIO_NUM_21
#define BOOT_BUTTON_GPIO        GPIO_NUM_0

#define DISPLAY_WIDTH   240
#define DISPLAY_HEIGHT  240
#define DISPLAY_MIRROR_X true
#define DISPLAY_MIRROR_Y false
#define DISPLAY_SWAP_XY false

#define DISPLAY_OFFSET_X  0
#define DISPLAY_OFFSET_Y  0

#define DISPLAY_BACKLIGHT_PIN GPIO_NUM_3
#define DISPLAY_BACKLIGHT_OUTPUT_INVERT false

#define DISPLAY_SPI_SCLK_PIN    GPIO_NUM_16
#define DISPLAY_SPI_MOSI_PIN    GPIO_NUM_17
#define DISPLAY_SPI_CS_PIN      GPIO_NUM_15
#define DISPLAY_SPI_DC_PIN      GPIO_NUM_7
#define DISPLAY_SPI_RESET_PIN   GPIO_NUM_18

#define DISPLAY_SPI_SCLK_HZ     (40 * 1000 * 1000)

// 电池 ADC 采样（真实硬件配置）
// 官方 Moji 原理图未把电池电压分压接到任何 GPIO（无 BAT/ADC 网络），
// 所以默认保持 GPIO_NUM_NC：GetBatteryLevel() 返回 false，界面显示“未知”，不造假数据。
// 如果后续硬件飞线或新版本 PCB 提供了真实分压引脚，把下面两个引脚改成实际 GPIO/通道，
// 并按实际分压电阻填入 upper/lower，电量就会走 ESP32-S3 内部 ADC 真实读取。
#define BATTERY_ADC_GPIO         GPIO_NUM_NC
#define BATTERY_ADC_CHANNEL      0
#define BATTERY_ADC_UPPER_RESISTOR 100000.0f
#define BATTERY_ADC_LOWER_RESISTOR 100000.0f
#define BATTERY_CHARGING_GPIO    GPIO_NUM_NC

#endif // _BOARD_CONFIG_H_
