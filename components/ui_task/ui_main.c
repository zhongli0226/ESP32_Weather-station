/*
 * @Description:
 * @Version:
 * @Autor: tangwc
 * @Date: 2023-02-27 20:32:25
 * @LastEditors: tangwc
 * @LastEditTime: 2023-03-16 21:12:09
 * @FilePath: \esp32_weather-station\components\ui_task\ui_main.c
 *
 *  Copyright (c) 2023 by tangwc, All Rights Reserved.
 */
#include <time.h>
#include <sys/time.h>
#include <string.h>

#include "esp_sntp.h"
#include "esp_log.h"
#include "esp_system.h"

#include "ui_main.h"
#include "ui_load.h"
#include "weather_json.h"
#include "network_task.h"

#define WEATHER_1 "\xEE\x98\x85"  // 11
#define WEATHER_2 "\xEE\x98\x86"  // 15
#define WEATHER_3 "\xEE\x98\x87"  // 17
#define WEATHER_4 "\xEE\x98\x88"  // 16
#define WEATHER_5 "\xEE\x98\x89"  // 12
#define WEATHER_6 "\xEE\x98\x8A"  // 0 2 38
#define WEATHER_7 "\xEE\x98\x8B"  // 13
#define WEATHER_8 "\xEE\x98\x8C"  // 18
#define WEATHER_9 "\xEE\x98\x8D"  // 6 8
#define WEATHER_10 "\xEE\x98\x8E" // 1 3
#define WEATHER_11 "\xEE\x98\x8F" // 9
#define WEATHER_12 "\xEE\x98\x90" // 14
#define WEATHER_13 "\xEE\x98\x91" // 20
#define WEATHER_14 "\xEE\x98\x92" // 10
#define WEATHER_15 "\xEE\x98\x93" // 4 5 7
#define WEATHER_16 "\xEE\x98\x94" // 25 37
#define WEATHER_17 "\xEE\x98\x95" // 24
#define WEATHER_18 "\xEE\x98\x96" // 19
#define WEATHER_19 "\xEE\x98\x97" // 28
#define WEATHER_20 "\xEE\x98\x98" // 26
#define WEATHER_21 "\xEE\x98\x99" // 30
#define WEATHER_22 "\xEE\x98\x9A" // 29
#define WEATHER_23 "\xEE\x98\x9B" // 22
#define WEATHER_24 "\xEE\x98\x9C" // 31
#define WEATHER_25 "\xEE\x98\x9D" // 27
#define WEATHER_26 "\xEE\x98\x9E" // 23
#define WEATHER_27 "\xEE\x98\x9F" // 32 33 34 35 36
#define WEATHER_28 "\xEE\x98\xA0" // 21
#define WEATHER_29 "\xEE\x99\xB6" // 99

#define HUMIDITU "\xEE\x9C\x9D"
#define WIND_MESS "\xEE\x9A\x96"

typedef struct _lv_clock
{
    lv_obj_t *time_label;              // 时间标签
    lv_obj_t *data_label;              // 日期标签
    lv_obj_t *weekday_label;           // 星期标签
    lv_obj_t *temperature_daily_label; // 当日天气温度范围
    lv_obj_t *temperature_now_label;   // 实时温度
    lv_obj_t *weather_code_label;      // 天气代码
    lv_obj_t *ip_label;                // 地点标签
    lv_obj_t *humi_label;              // 湿度
    lv_obj_t *wind_label;              // 风力
} lv_clock_t;

static const char *TAG = "ui_main";
static void clock_date_task_callback(lv_task_t *task);

void Main_interface(void)
{
    static lv_clock_t lv_clock = {0};

    static lv_style_t style_obj_bg;
    lv_style_reset(&style_obj_bg);
    lv_style_init(&style_obj_bg);
    lv_style_set_bg_color(&style_obj_bg, LV_STATE_DEFAULT, lv_color_hex(0x41485a));
    lv_style_set_border_color(&style_obj_bg, LV_STATE_DEFAULT, lv_color_hex(0x41485a));

    // 中间时钟部分设计
    static lv_style_t style_lab_time;
    lv_style_reset(&style_lab_time);
    lv_style_init(&style_lab_time);
    lv_style_set_text_color(&style_lab_time, LV_STATE_DEFAULT, LV_COLOR_ORANGE);       // 字体颜色
    lv_style_set_text_font(&style_lab_time, LV_STATE_DEFAULT, &lv_font_montserrat_48); // 字体大小

    static lv_style_t style_lab_data_weekday;
    lv_style_reset(&style_lab_data_weekday);
    lv_style_init(&style_lab_data_weekday);
    lv_style_set_text_color(&style_lab_data_weekday, LV_STATE_DEFAULT, LV_COLOR_WHITE);        // 字体颜色
    lv_style_set_text_font(&style_lab_data_weekday, LV_STATE_DEFAULT, &lv_font_montserrat_18); // 字体大小

    lv_obj_t *obj_mid = lv_obj_create(scr_main, NULL);
    lv_obj_set_size(obj_mid, 210, 80); // 设置对象大小
    lv_obj_align(obj_mid, scr_main, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(obj_mid, LV_OBJ_PART_MAIN, &style_obj_bg);

    lv_clock.time_label = lv_label_create(obj_mid, NULL); // 创建文本
    lv_obj_add_style(lv_clock.time_label, LV_LABEL_PART_MAIN, &style_lab_time);

    lv_clock.data_label = lv_label_create(obj_mid, NULL);
    lv_obj_add_style(lv_clock.data_label, LV_LABEL_PART_MAIN, &style_lab_data_weekday);

    lv_clock.weekday_label = lv_label_create(obj_mid, NULL);
    lv_obj_add_style(lv_clock.weekday_label, LV_LABEL_PART_MAIN, &style_lab_data_weekday);

    // 顶部天气设置
    lv_obj_t *obj_top = lv_obj_create(scr_main, NULL);
    lv_obj_set_size(obj_top, 240, 80); // 设置对象大小
    lv_obj_align(obj_top, scr_main, LV_ALIGN_IN_TOP_MID, 0, 0);
    lv_obj_add_style(obj_top, LV_OBJ_PART_MAIN, &style_obj_bg);

    static lv_style_t style_temperature_now;                                                   // 创建一个风格
    lv_style_init(&style_temperature_now);                                                     // 初始化风格
    lv_style_set_text_color(&style_temperature_now, LV_STATE_DEFAULT, lv_color_hex(0xe1e4dc)); // 字体颜色
    lv_style_set_text_font(&style_temperature_now, LV_STATE_DEFAULT, &lv_font_montserrat_48);

    static lv_style_t style_temperature_daily;                                                   // 创建一个风格
    lv_style_init(&style_temperature_daily);                                                     // 初始化风格
    lv_style_set_text_color(&style_temperature_daily, LV_STATE_DEFAULT, lv_color_hex(0xe1e4dc)); // 字体颜色
    lv_style_set_text_font(&style_temperature_daily, LV_STATE_DEFAULT, &lv_font_montserrat_18);

    static lv_style_t weather_code_style;
    lv_style_init(&weather_code_style);
    lv_style_set_text_color(&weather_code_style, LV_STATE_DEFAULT, lv_color_hex(0x79b8ff)); // 字体颜色
    lv_style_set_text_font(&weather_code_style, LV_STATE_DEFAULT, &my_font_70);

    lv_clock.weather_code_label = lv_label_create(obj_top, NULL);
    lv_obj_add_style(lv_clock.weather_code_label, LV_LABEL_PART_MAIN, &weather_code_style); // 应用效果风格

    lv_clock.temperature_now_label = lv_label_create(obj_top, NULL);
    lv_obj_add_style(lv_clock.temperature_now_label, LV_LABEL_PART_MAIN, &style_temperature_now); // 应用效果风格

    lv_clock.temperature_daily_label = lv_label_create(obj_top, NULL);
    lv_obj_add_style(lv_clock.temperature_daily_label, LV_LABEL_PART_MAIN, &style_temperature_daily); // 应用效果风格

    // 底部ui设计
    lv_obj_t *obj_bottom = lv_obj_create(scr_main, NULL);
    lv_obj_set_size(obj_bottom, 240, 80); // 设置对象大小
    lv_obj_align(obj_bottom, scr_main, LV_ALIGN_IN_BOTTOM_MID, 0, 0);
    lv_obj_add_style(obj_bottom, LV_OBJ_PART_MAIN, &style_obj_bg);

    static lv_style_t style_ip_name;                                                   // 创建一个风格
    lv_style_init(&style_ip_name);                                                     // 初始化风格
    lv_style_set_text_color(&style_ip_name, LV_STATE_DEFAULT, lv_color_hex(0x07c615)); // 字体颜色
    lv_style_set_text_font(&style_ip_name, LV_STATE_DEFAULT, &lv_font_montserrat_18);

    lv_clock.ip_label = lv_label_create(obj_bottom, NULL);
    lv_obj_add_style(lv_clock.ip_label, LV_LABEL_PART_MAIN, &style_ip_name); // 应用效果风格

    static lv_style_t icon_style;
    lv_style_init(&icon_style);
    lv_style_set_text_color(&icon_style, LV_STATE_DEFAULT, lv_color_hex(0x79b8ff)); // 字体颜色
    lv_style_set_text_font(&icon_style, LV_STATE_DEFAULT, &my_font_30);

    static lv_style_t figures_style;
    lv_style_init(&figures_style);
    lv_style_set_text_color(&figures_style, LV_STATE_DEFAULT, lv_color_hex(0x79b8ff)); // 字体颜色
    lv_style_set_text_font(&figures_style, LV_STATE_DEFAULT, &lv_font_montserrat_32);

    lv_obj_t *humi_icon_label = lv_label_create(obj_bottom, NULL);
    lv_obj_add_style(humi_icon_label, LV_LABEL_PART_MAIN, &icon_style); // 应用效果风格
    lv_label_set_text_fmt(humi_icon_label, HUMIDITU);
    lv_obj_align(humi_icon_label, obj_bottom, LV_ALIGN_IN_TOP_LEFT, 5, 10);

    lv_obj_t *wind_icon_label = lv_label_create(obj_bottom, NULL);
    lv_obj_add_style(wind_icon_label, LV_LABEL_PART_MAIN, &icon_style); // 应用效果风格
    lv_label_set_text_fmt(wind_icon_label, WIND_MESS);
    lv_obj_align(wind_icon_label, obj_bottom, LV_ALIGN_IN_TOP_MID, 35, 10);

    lv_clock.humi_label = lv_label_create(obj_bottom, NULL);
    lv_obj_add_style(lv_clock.humi_label, LV_LABEL_PART_MAIN, &figures_style); // 应用效果风格

    lv_clock.wind_label = lv_label_create(obj_bottom, NULL);
    lv_obj_add_style(lv_clock.wind_label, LV_LABEL_PART_MAIN, &figures_style); // 应用效果风格

    lv_task_t *task_timer = lv_task_create(clock_date_task_callback, 200, LV_TASK_PRIO_MID, &lv_clock); // 创建定时任务，200ms刷新一次
    lv_task_ready(task_timer);
    ESP_LOGI(TAG, "start time call back!");
}

static void clock_date_task_callback(lv_task_t *task)
{
    static const char *week_day[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    time_t now_time;
    struct tm time_info;

    time(&now_time);
    localtime_r(&now_time, &time_info);

    int year = time_info.tm_year + 1900;
    int month = time_info.tm_mon + 1;
    int day = time_info.tm_mday;
    int weekday = time_info.tm_wday;
    int hour = time_info.tm_hour;
    int minutes = time_info.tm_min;
    int second = time_info.tm_sec;

    lv_clock_t *clock = (lv_clock_t *)(task->user_data);

    if (user_time_flag)
    {
        lv_label_set_text_fmt(clock->time_label, "%02d:%02d:%02d", hour, minutes, second);
        lv_obj_align(clock->time_label, lv_obj_get_parent(clock->time_label), LV_ALIGN_CENTER, 0, -10);
        // ESP_LOGI(TAG,"time : %d:%d:%d",time_info.tm_hour,time_info.tm_min,time_info.tm_sec);
        lv_label_set_text_fmt(clock->data_label, "%d-%02d-%02d", year, month, day);
        lv_obj_align(clock->data_label, lv_obj_get_parent(clock->data_label), LV_ALIGN_IN_BOTTOM_LEFT, 0, -5);

        lv_label_set_text_fmt(clock->weekday_label, "%s", week_day[weekday]);
        lv_obj_align(clock->weekday_label, lv_obj_get_parent(clock->weekday_label), LV_ALIGN_IN_BOTTOM_RIGHT, -5, -5);
    }
    else
    {
        lv_label_set_text_fmt(clock->time_label, "--:--:--");
        lv_obj_align(clock->time_label, lv_obj_get_parent(clock->time_label), LV_ALIGN_CENTER, 0, -10);
        // ESP_LOGI(TAG,"time : %d:%d:%d",time_info.tm_hour,time_info.tm_min,time_info.tm_sec);
        lv_label_set_text_fmt(clock->data_label, "-- -- --");
        lv_obj_align(clock->data_label, lv_obj_get_parent(clock->data_label), LV_ALIGN_IN_BOTTOM_LEFT, 0, -5);

        lv_label_set_text_fmt(clock->weekday_label, "----");
        lv_obj_align(clock->weekday_label, lv_obj_get_parent(clock->weekday_label), LV_ALIGN_IN_BOTTOM_RIGHT, -5, -5);
    }

    if (user_sen_flag)
    {
        lv_label_set_text_fmt(clock->temperature_now_label, "%s°", user_sen_config.temperature);
        lv_obj_align(clock->temperature_now_label, lv_obj_get_parent(clock->temperature_now_label), LV_ALIGN_IN_TOP_MID, 55, 0);

        lv_label_set_text_fmt(clock->temperature_daily_label, LV_SYMBOL_UP " %s°    " LV_SYMBOL_DOWN " %s°", user_sen_config.high, user_sen_config.low);
        lv_obj_align(clock->temperature_daily_label, lv_obj_get_parent(clock->temperature_daily_label), LV_ALIGN_IN_BOTTOM_RIGHT, -15, -5);
        int weather_code = atoi(user_sen_config.code);
        switch (weather_code)
        {
        case 0:
        case 2:
        case 38:
            lv_label_set_text(clock->weather_code_label, WEATHER_6); // 设置显示文本
            break;
        case 1:
        case 3:
            lv_label_set_text(clock->weather_code_label, WEATHER_10); // 设置显示文本
            break;
        case 4:
        case 5:
        case 7:
            lv_label_set_text(clock->weather_code_label, WEATHER_15); // 设置显示文本
            break;
        case 6:
        case 8:
            lv_label_set_text(clock->weather_code_label, WEATHER_9); // 设置显示文本
            break;
        case 9:
            lv_label_set_text(clock->weather_code_label, WEATHER_11); // 设置显示文本
            break;
        case 10:
            lv_label_set_text(clock->weather_code_label, WEATHER_14); // 设置显示文本
            break;
        case 11:
            lv_label_set_text(clock->weather_code_label, WEATHER_1); // 设置显示文本
            break;
        case 12:
            lv_label_set_text(clock->weather_code_label, WEATHER_5); // 设置显示文本
            break;
        case 13:
            lv_label_set_text(clock->weather_code_label, WEATHER_7); // 设置显示文本
            break;
        case 14:
            lv_label_set_text(clock->weather_code_label, WEATHER_12); // 设置显示文本
            break;
        case 15:
            lv_label_set_text(clock->weather_code_label, WEATHER_2); // 设置显示文本
            break;
        case 16:
            lv_label_set_text(clock->weather_code_label, WEATHER_4); // 设置显示文本
            break;
        case 17:
            lv_label_set_text(clock->weather_code_label, WEATHER_3); // 设置显示文本
            break;
        case 18:
            lv_label_set_text(clock->weather_code_label, WEATHER_8); // 设置显示文本
            break;
        case 19:
            lv_label_set_text(clock->weather_code_label, WEATHER_18); // 设置显示文本
            break;
        case 20:
            lv_label_set_text(clock->weather_code_label, WEATHER_13); // 设置显示文本
            break;
        case 21:
            lv_label_set_text(clock->weather_code_label, WEATHER_28); // 设置显示文本
            break;
        case 22:
            lv_label_set_text(clock->weather_code_label, WEATHER_23); // 设置显示文本
            break;
        case 23:
            lv_label_set_text(clock->weather_code_label, WEATHER_26); // 设置显示文本
            break;
        case 24:
            lv_label_set_text(clock->weather_code_label, WEATHER_17); // 设置显示文本
            break;
        case 25:
        case 37:
            lv_label_set_text(clock->weather_code_label, WEATHER_16); // 设置显示文本
            break;
        case 26:
            lv_label_set_text(clock->weather_code_label, WEATHER_20); // 设置显示文本
            break;
        case 27:
            lv_label_set_text(clock->weather_code_label, WEATHER_25); // 设置显示文本
            break;
        case 28:
            lv_label_set_text(clock->weather_code_label, WEATHER_19); // 设置显示文本
            break;
        case 29:
            lv_label_set_text(clock->weather_code_label, WEATHER_22); // 设置显示文本
            break;
        case 30:
            lv_label_set_text(clock->weather_code_label, WEATHER_21); // 设置显示文本
            break;
        case 31:
            lv_label_set_text(clock->weather_code_label, WEATHER_24); // 设置显示文本
            break;
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
            lv_label_set_text(clock->weather_code_label, WEATHER_27); // 设置显示文本
            break;
        default:
            lv_label_set_text(clock->weather_code_label, WEATHER_29); // 设置显示文本
            break;
        }
        lv_obj_align(clock->weather_code_label, lv_obj_get_parent(clock->weather_code_label), LV_ALIGN_IN_TOP_LEFT, 20, 5); // 重新设置对齐

        lv_label_set_text_fmt(clock->ip_label, LV_SYMBOL_GPS " %s", user_sen_config.name);
        lv_obj_align(clock->ip_label, lv_obj_get_parent(clock->ip_label), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

        lv_label_set_text_fmt(clock->humi_label, "%s", user_sen_config.humidity);
        lv_obj_align(clock->humi_label, lv_obj_get_parent(clock->humi_label), LV_ALIGN_IN_TOP_LEFT, 40, 5);

        lv_label_set_text_fmt(clock->wind_label, "%s", user_sen_config.wind_scale);
        lv_obj_align(clock->wind_label, lv_obj_get_parent(clock->wind_label), LV_ALIGN_IN_TOP_MID, 70, 5);

        // ESP_LOGI(TAG, "wind_direction:%s", user_sen_config.wind_direction);
        // ESP_LOGI(TAG, "wind_scale:%s", user_sen_config.wind_scale);
        // ESP_LOGI(TAG, "humidity:%s", user_sen_config.humidity);
        // ESP_LOGI(TAG, "name:%s", user_sen_config.name);
        // ESP_LOGI(TAG, "text_day:%s", user_sen_config.text);
        // ESP_LOGI(TAG, "code_day:%s", user_sen_config.code);
        // ESP_LOGI(TAG, "temperature:%s", user_sen_config.temperature);
    }
    else
    {
        lv_label_set_text_fmt(clock->temperature_now_label, "--°");
        lv_obj_align(clock->temperature_now_label, lv_obj_get_parent(clock->temperature_now_label), LV_ALIGN_IN_TOP_MID, 55, 0);

        lv_label_set_text_fmt(clock->temperature_daily_label, LV_SYMBOL_UP " --°    " LV_SYMBOL_DOWN " --°");
        lv_obj_align(clock->temperature_daily_label, lv_obj_get_parent(clock->temperature_daily_label), LV_ALIGN_IN_BOTTOM_RIGHT, -15, -5);

        lv_label_set_text(clock->weather_code_label, WEATHER_29);                                                           // 设置显示文本
        lv_obj_align(clock->weather_code_label, lv_obj_get_parent(clock->weather_code_label), LV_ALIGN_IN_TOP_LEFT, 20, 5); // 重新设置对齐

        lv_label_set_text_fmt(clock->ip_label, LV_SYMBOL_GPS " --");
        lv_obj_align(clock->ip_label, lv_obj_get_parent(clock->ip_label), LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

        lv_label_set_text_fmt(clock->humi_label, "--");
        lv_obj_align(clock->humi_label, lv_obj_get_parent(clock->humi_label), LV_ALIGN_IN_TOP_LEFT, 40, 5);

        lv_label_set_text_fmt(clock->wind_label, "--");
        lv_obj_align(clock->wind_label, lv_obj_get_parent(clock->wind_label), LV_ALIGN_IN_TOP_MID, 70, 5);
    }
}