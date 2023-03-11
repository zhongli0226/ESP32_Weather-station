/*
 * @Description:
 * @Version:
 * @Autor: tangwc
 * @Date: 2023-02-27 20:32:25
 * @LastEditors: tangwc
 * @LastEditTime: 2023-03-11 21:37:38
 * @FilePath: \esp32_weather-station\components\ui_task\ui_main.c
 *
 *  Copyright (c) 2023 by tangwc, All Rights Reserved.
 */
#include <time.h>
#include <sys/time.h>

#include "esp_sntp.h"
#include "esp_log.h"
#include "esp_system.h"

#include "ui_main.h"
#include "ui_load.h"
#include "weather_json.h"
#include "network_task.h"

typedef struct _lv_clock
{
    lv_obj_t *time_label;              // 时间标签
    lv_obj_t *data_label;              // 日期标签
    lv_obj_t *weekday_label;           // 星期标签
    lv_obj_t *temperature_daily_label; // 当日天气范围
    lv_obj_t *temperature_now_label;   // 实时温度
    lv_obj_t *ip_label;                // 地点标签
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

    lv_clock.temperature_now_label = lv_label_create(obj_top, NULL);
    lv_obj_add_style(lv_clock.temperature_now_label, LV_LABEL_PART_MAIN, &style_temperature_now); // 应用效果风格

    lv_clock.temperature_daily_label = lv_label_create(obj_top, NULL);
    lv_obj_add_style(lv_clock.temperature_daily_label, LV_LABEL_PART_MAIN, &style_temperature_daily); // 应用效果风格

    lv_task_t *task_timer = lv_task_create(clock_date_task_callback, 200, LV_TASK_PRIO_MID, &lv_clock); // 创建定时任务，200ms刷新一次
    lv_task_ready(task_timer);
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

        // ESP_LOGI(TAG, "wind_direction:%s", user_sen_config.wind_direction);
        // ESP_LOGI(TAG, "wind_scale:%s", user_sen_config.wind_scale);
        // ESP_LOGI(TAG, "humidity:%s", user_sen_config.humidity);
        ESP_LOGI(TAG, "name:%s", user_sen_config.name);
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
    }
}