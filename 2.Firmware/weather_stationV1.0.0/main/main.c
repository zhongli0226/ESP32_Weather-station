/* LVGL Example project
 *
 * Basic project to test LVGL on ESP32 based projects.
 *
 * This example code_day is in the Public Domain (or CC0 licensed, at your option.)
 *
 * Unless required by applicable law or agreed to in writing, this
 * software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, either express or implied.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_sntp.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"
#include "freertos/semphr.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "driver/rmt.h"
#include "driver/touch_pad.h"

#include "SD_card.h"
#include "Wifi.h"
#include "WS2812.h"
/* Littlevgl specific */

#include "lvgl.h"
#include "lvgl_helpers.h"
#include "lv_fs_if.h"
#include "lv_sjpg.h"

/*********************
 *      DEFINES
 *********************/
#define LV_TICK_PERIOD_MS 1
#define TAG "Albums"
#define TOUCH_THRESH_NO_USE (0)
#define TOUCHPAD_FILTER_TOUCH_PERIOD (10)
#define Touch_pad_1 TOUCH_PAD_NUM9
#define Touch_pad_2 TOUCH_PAD_NUM8

LV_FONT_DECLARE(weather_font40);  //申明字体
LV_FONT_DECLARE(weather_text_20); //申明字体
LV_FONT_DECLARE(myFont);          // 字体声明
LV_FONT_DECLARE(address_font);    // 字体声明
LV_FONT_DECLARE(other_font);

LV_IMG_DECLARE(leisheng);
LV_IMG_DECLARE(linghua);
LV_IMG_DECLARE(shengzi);
LV_IMG_DECLARE(tuoma);
LV_IMG_DECLARE(wanye);
LV_IMG_DECLARE(wulang);
LV_IMG_DECLARE(xiaogong);
LV_IMG_DECLARE(xinghai);
LV_IMG_DECLARE(zhaoyou);
//定义图标字体1
#define weather_1 "\xEE\x98\x90"  //晴天
#define weather_2 "\xEE\x9A\x8C"  //阴天
#define weather_3 "\xEE\x98\x8C"  //多云
#define weather_4 "\xEE\x98\x95"  //雨
#define weather_5 "\xEE\x98\x93"  //大雨
#define weather_6 "\xEE\x98\x8D"  //暴雨
#define weather_7 "\xEE\x98\x85"  //阵雨
#define weather_8 "\xEE\x98\x97"  //雪
#define weather_9 "\xEE\x98\x8F"  //大雪
#define weather_10 "\xEE\x98\x9F" //雾
#define weather_11 "\xEE\x9B\xB2" //无

#define image_WY 1
#define image_XG 2
#define image_LS 3
#define image_ZY 4
#define image_LH 5
#define image_TM 6
#define image_SZ 7
#define image_WL 8
#define image_XH 9
int image_num = 1;
/**********************
 *  STATIC PROTOTYPES
 **********************/
//从sd卡获取WiFi信息
char wifi_ssid[32] = "";
char wifi_password[64] = "";
char loaction[10] = "";
char wind_direction[8] = "";
char name[20] = "";
int wind_scale;
int humi;
int code_day;
int temperature;
static const char *Waitwifi = "Wait Everything Init";

static bool Right_flag, Left_flag, Touch_flag;

typedef struct _lv_clock
{
    lv_obj_t *time_label;         // 时间标签
    lv_obj_t *data_label;         // 日期标签
    lv_obj_t *weekday_label;      //星期标签
    lv_obj_t *weather_label;      //天气标签
    lv_obj_t *weather_text_label; //天气标签
    lv_obj_t *temperature_label;  //温度标签
} lv_clock_t;

static void lv_tick_task(void *arg);
static void guiTask(void *pvParameter);
static void touchTask(void *pvParameter);
static void obtain_time(void);
static void initialize_sntp(void);

lv_obj_t *scr_ui1; //创建一个屏幕
void lv_ex_init(void);
void lv_main_time(void);

static void clock_date_task_callback(lv_task_t *task);

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

// 触摸中断处理函数。触摸过的端口保存在s_pad_activated数组中
static void tp_example_rtc_intr(void *arg)
{
    uint32_t pad_intr = touch_pad_get_status(); // 读取触摸状态
    touch_pad_clear_status();                   // 清除中断
    if ((pad_intr >> Touch_pad_1) & 0x01)
    {
        Right_flag = true;
    }
    if ((pad_intr >> Touch_pad_2) & 0x01)
    {
        Left_flag = true;
    }
}

/**********************
 *   APPLICATION MAIN
 **********************/
void app_main()
{
    nvs_flash_init();
    ws2812_control_init();
    // lvgl 创建
    lv_init();

    /*初始化驱动程序使用的SPI或I2C总线*/
    lvgl_driver_init();

    lv_fs_if_init();      // LVGL 文件系统初始化
    lv_split_jpeg_init(); // LVGL PNG解码库初始化

    lv_color_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1 != NULL);

    static lv_color_t *buf2 = NULL;
    static lv_disp_buf_t disp_buf;

    uint32_t size_in_px = DISP_BUF_SIZE;

    /*根据所选显示初始化工作缓冲区。
     *注：使用单色显示器时，buf2==NULL。*/
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;

    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    /*创建并启动一个周期性计时器中断，以调用lv_tick_inc*/
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    /*创建演示应用程序*/
    static lv_style_t style_bg;
    lv_style_init(&style_bg);
    lv_style_set_bg_color(&style_bg, LV_STATE_DEFAULT, lv_color_hex(0x41485a));
    scr_ui1 = lv_obj_create(NULL, NULL); //创建主屏幕
    lv_obj_add_style(scr_ui1, LV_OBJ_PART_MAIN, &style_bg);

    lv_ex_init(); //加载界面

    xTaskCreatePinnedToCore(guiTask, "gui", 4096, NULL, 0, NULL, 1);
    xTaskCreate(rainbow, "ws2812 rainbow demo", 4096, NULL, 10, NULL);
    time_t now;
    struct tm timeinfo;
    // char strftime_buf[64];
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    wifi_init();
    if (timeinfo.tm_year < (2022 - 1900))
    {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
        // update 'now' variable with current time
        time(&now);
    }
    setenv("TZ", "CST-8", 1);
    tzset();
    lv_main_time(); // 时钟界面
    lv_scr_load_anim(scr_ui1, LV_SCR_LOAD_ANIM_FADE_ON, 1000, 100, true);
    xTaskCreate(touchTask, "touch chack demo", 4096, NULL, 5, NULL);
    //////////////////////////////////////////////////////////
    /////////创建一个IMG对象并加载SD卡中的sjpg图片解码显示//////////
    //////////////////////////////////////////////////////////
    lv_obj_t *obsjpg = lv_img_create(scr_ui1, NULL); // 创建一个IMG对象

    Touch_flag = true;
    while (1)
    {
        if (Touch_flag == true)
        {
            switch (image_num)
            {
            case image_WY:
                lv_img_set_src(obsjpg, &wanye);                              // 加载SD卡中的SJPG图片
                lv_obj_align(obsjpg, scr_ui1, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0); // 重新设置对齐
                break;
            case image_XG:
                lv_img_set_src(obsjpg, &xiaogong);
                lv_obj_align(obsjpg, scr_ui1, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0); // 重新设置对齐
                break;
            case image_LS:
                lv_img_set_src(obsjpg, &leisheng);
                lv_obj_align(obsjpg, scr_ui1, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0); // 重新设置对齐
                break;
            case image_ZY:
                lv_img_set_src(obsjpg, &zhaoyou);
                lv_obj_align(obsjpg, scr_ui1, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0); // 重新设置对齐
                break;
            case image_LH:
                lv_img_set_src(obsjpg, &linghua);
                lv_obj_align(obsjpg, scr_ui1, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0); // 重新设置对齐
                break;
            case image_TM:
                lv_img_set_src(obsjpg, &tuoma);
                lv_obj_align(obsjpg, scr_ui1, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0); // 重新设置对齐
                break;
            case image_SZ:
                lv_img_set_src(obsjpg, &shengzi);
                lv_obj_align(obsjpg, scr_ui1, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0); // 重新设置对齐
                break;
            case image_WL:
                lv_img_set_src(obsjpg, &wulang);
                lv_obj_align(obsjpg, scr_ui1, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0); // 重新设置对齐
                break;
            case image_XH:
                lv_img_set_src(obsjpg, &xinghai);
                lv_obj_align(obsjpg, scr_ui1, LV_ALIGN_IN_BOTTOM_RIGHT, 0, 0); // 重新设置对齐
                break;
            default:
                break;
                Touch_flag = false;
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

/*创建一个信号量来处理对lvgl的并发调用
 *如果希望从其他线程/任务调用*any*lvgl函数
 *你应该锁定同一个信号灯！*/
SemaphoreHandle_t xGuiSemaphore;

static void guiTask(void *pvParameter)
{

    (void)pvParameter;
    xGuiSemaphore = xSemaphoreCreateMutex();

    while (1)
    {
        /*延迟1滴答声（假设FreeRTOS滴答声为10毫秒*/
        vTaskDelay(1);
        // 尝试锁定信号量，如果成功，请调用lvgl的东西
        if (xSemaphoreTake(xGuiSemaphore, (TickType_t)10) == pdTRUE)
        {
            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore); // 释放信号量
        }
    }
    vTaskDelete(NULL); // 删除任务
}

static void touchTask(void *pvParameter)
{
    uint16_t touch_value;
    printf("go on........\n");
    //初始化触摸板外围设备，它将启动计时器运行过滤器
    ESP_ERROR_CHECK(touch_pad_init());
    // 如果使用中断触发模式，应将触摸传感器FSM模式设置为“ TOUCH_FSM_MODE_TIMER”(通过定时器启动FSM)
    touch_pad_set_fsm_mode(TOUCH_FSM_MODE_TIMER);
    //设置充电/放电的参考电压
    //对于大多数使用场景，我们建议使用以下组合：
    //高参考电压为2.7V-1V=1.7V，低参考电压为0.5V。
    touch_pad_set_voltage(TOUCH_HVOLT_2V7, TOUCH_LVOLT_0V5, TOUCH_HVOLT_ATTEN_1V);
    //初始化触摸板IO
    touch_pad_config(Touch_pad_1, TOUCH_THRESH_NO_USE);
    touch_pad_config(Touch_pad_2, TOUCH_THRESH_NO_USE);
    //初始化并启动软件过滤器，以检测电容的微小变化。
    touch_pad_filter_start(TOUCHPAD_FILTER_TOUCH_PERIOD);
    // 设定中断限值，此时不要触摸，2/3的读取值做为限值
    touch_pad_read_filtered(Touch_pad_1, &touch_value); // 读取过滤值
    printf("test init : touch pad 1 val is %d\n", touch_value);
    ESP_ERROR_CHECK(touch_pad_set_thresh(Touch_pad_1, touch_value * 2 / 3)); // 设置中断限值

    touch_pad_read_filtered(Touch_pad_2, &touch_value); // 读取过滤值
    printf("test init : touch pad 2 val is %d\n", touch_value);
    ESP_ERROR_CHECK(touch_pad_set_thresh(Touch_pad_2, touch_value * 2 / 3)); // 设置中断限值
    // 注册触摸中断ISR
    touch_pad_isr_register(tp_example_rtc_intr, NULL);

    touch_pad_intr_enable();  // 启用中断模式
    touch_pad_clear_status(); // 清除触摸状态
    while (1)
    {
        //触摸按键监测
        if (Right_flag == true)
        {
            printf("touch right activated!\n");
            vTaskDelay(200 / portTICK_PERIOD_MS);
            Right_flag = false;
            Touch_flag = true;
            image_num = image_num - 1;
            if (image_num == 0)
            {
                image_num = 9;
            }
            touch_pad_read_filtered(Touch_pad_1, &touch_value);
            printf("test init :touch pad val is %d\n", touch_value);
        }
        if (Left_flag == true)
        {
            printf("touch left activated!\n");
            vTaskDelay(200 / portTICK_PERIOD_MS);
            Left_flag = false;
            Touch_flag = true;
            image_num = image_num + 1;
            if (image_num == 10)
            {
                image_num = 1;
            }
            touch_pad_read_filtered(Touch_pad_2, &touch_value);
            printf("test init :touch pad val is %d\n", touch_value);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void lv_ex_init(void)
{
    /*创建一个预加载对象*/
    lv_obj_t *preload = lv_spinner_create(lv_scr_act(), NULL); //创建加载圆圈
    lv_obj_set_size(preload, 110, 110);                        //设置尺寸
    lv_spinner_set_arc_length(preload, 80);                    //设置动态圆弧长度
    lv_obj_align(preload, NULL, LV_ALIGN_CENTER, 0, 15);       //相对父对象居中

    static lv_style_t style;
    lv_style_init(&style);

    /*Set a background color and a radius*/
    lv_style_set_radius(&style, LV_STATE_DEFAULT, 10);
    lv_style_set_bg_opa(&style, LV_STATE_DEFAULT, LV_OPA_COVER);
    lv_style_set_bg_color(&style, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_border_color(&style, LV_STATE_DEFAULT, lv_color_hex(0x01a2b1));
    /*Create an object with the new style*/
    lv_obj_t *obj = lv_obj_create(lv_scr_act(), NULL);
    lv_obj_add_style(obj, LV_OBJ_PART_MAIN, &style);
    lv_obj_set_size(obj, 180, 30);
    lv_obj_align_origo(obj, preload, LV_ALIGN_OUT_TOP_MID, 0, -20); //设置位置

    static lv_style_t style_lab;
    lv_style_init(&style_lab);

    lv_style_set_text_color(&style_lab, LV_STATE_DEFAULT, LV_COLOR_BLACK); //字体颜色
    lv_obj_t *label = lv_label_create(obj, NULL);                          //创建文本
    lv_obj_add_style(label, LV_LABEL_PART_MAIN, &style_lab);
    lv_label_set_static_text(label, Waitwifi);
    lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, 0);
}

void lv_main_time(void)
{

    static lv_clock_t lv_clock = {0};

    static lv_style_t style_clock;
    lv_style_reset(&style_clock);
    lv_style_init(&style_clock);
    lv_style_set_bg_color(&style_clock, LV_STATE_DEFAULT, lv_color_hex(0x41485a));
    lv_style_set_border_color(&style_clock, LV_STATE_DEFAULT, lv_color_hex(0x41485a));

    static lv_style_t style_lab_time;
    lv_style_reset(&style_lab_time);
    lv_style_init(&style_lab_time);
    lv_style_set_text_color(&style_lab_time, LV_STATE_DEFAULT, LV_COLOR_ORANGE);       //字体颜色
    lv_style_set_text_font(&style_lab_time, LV_STATE_DEFAULT, &lv_font_montserrat_48); //字体大小

    static lv_style_t style_lab_data_weekday;
    lv_style_reset(&style_lab_data_weekday);
    lv_style_init(&style_lab_data_weekday);
    lv_style_set_text_color(&style_lab_data_weekday, LV_STATE_DEFAULT, LV_COLOR_WHITE);        //字体颜色
    lv_style_set_text_font(&style_lab_data_weekday, LV_STATE_DEFAULT, &lv_font_montserrat_18); //字体大小

    lv_obj_t *obj_time = lv_obj_create(scr_ui1, NULL);
    lv_obj_set_size(obj_time, 220, 75); // 设置对象大小
    lv_obj_align(obj_time, scr_ui1, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_style(obj_time, LV_OBJ_PART_MAIN, &style_clock);

    lv_clock.time_label = lv_label_create(obj_time, NULL); //创建文本
    lv_obj_add_style(lv_clock.time_label, LV_LABEL_PART_MAIN, &style_lab_time);

    lv_clock.data_label = lv_label_create(obj_time, NULL);
    lv_obj_add_style(lv_clock.data_label, LV_LABEL_PART_MAIN, &style_lab_data_weekday);

    lv_clock.weekday_label = lv_label_create(obj_time, NULL);
    lv_obj_add_style(lv_clock.weekday_label, LV_LABEL_PART_MAIN, &style_lab_data_weekday);

    static lv_style_t style_weather;                                                   // 创建一个风格
    lv_style_init(&style_weather);                                                     // 初始化风格
    lv_style_set_text_color(&style_weather, LV_STATE_DEFAULT, lv_color_hex(0x00fefe)); //字体颜色
    lv_style_set_text_font(&style_weather, LV_STATE_DEFAULT, &weather_font40);

    static lv_style_t style_weather_text;                                                   // 创建一个风格
    lv_style_init(&style_weather_text);                                                     // 初始化风格
    lv_style_set_text_color(&style_weather_text, LV_STATE_DEFAULT, lv_color_hex(0xff00fa)); //字体颜色
    lv_style_set_text_font(&style_weather_text, LV_STATE_DEFAULT, &weather_text_20);

    lv_obj_t *obj_weather = lv_obj_create(scr_ui1, NULL);
    lv_obj_set_size(obj_weather, 60, 70); // 设置对象大小
    lv_obj_align(obj_weather, scr_ui1, LV_ALIGN_IN_TOP_LEFT, 0, 5);
    lv_obj_add_style(obj_weather, LV_OBJ_PART_MAIN, &style_clock);

    lv_clock.weather_label = lv_label_create(obj_weather, NULL);
    lv_obj_add_style(lv_clock.weather_label, LV_LABEL_PART_MAIN, &style_weather); // 应用效果风格

    lv_clock.weather_text_label = lv_label_create(obj_weather, NULL);
    lv_obj_add_style(lv_clock.weather_text_label, LV_LABEL_PART_MAIN, &style_weather_text); // 应用效果风格

    ////////////////地点部分设计
    static lv_style_t style_address;
    lv_style_reset(&style_address);
    lv_style_init(&style_address);
    lv_style_set_bg_color(&style_address, LV_STATE_DEFAULT, lv_color_hex(0xff6634));
    lv_style_set_border_color(&style_address, LV_STATE_DEFAULT, lv_color_hex(0xff6634));

    lv_obj_t *obj_address = lv_obj_create(scr_ui1, NULL);
    lv_obj_set_size(obj_address, 60, 20); // 设置对象大小
    lv_obj_align(obj_address, scr_ui1, LV_ALIGN_IN_TOP_MID, -30, 15);
    lv_obj_add_style(obj_address, LV_OBJ_PART_MAIN, &style_address);
    if ((strcmp(loaction, (char *)"hefei") == 0) || (strcmp(loaction, (char *)"beijing") == 0) || (strcmp(loaction, (char *)"dalian") == 0) || (strcmp(loaction, (char *)"shanghai") == 0) || (strcmp(loaction, (char *)"shenzhen") == 0))
    {
        static lv_style_t style_address_text;                                           // 创建一个风格
        lv_style_init(&style_address_text);                                             // 初始化风格
        lv_style_set_text_color(&style_address_text, LV_STATE_DEFAULT, LV_COLOR_WHITE); //字体颜色
        lv_style_set_text_font(&style_address_text, LV_STATE_DEFAULT, &address_font);

        lv_obj_t *label_address = lv_label_create(obj_address, NULL);
        lv_obj_add_style(label_address, LV_LABEL_PART_MAIN, &style_address_text); // 应用效果风格
        lv_label_set_text(label_address, name);                                   // 设置显示文本
        lv_obj_align(label_address, lv_obj_get_parent(label_address), LV_ALIGN_CENTER, 0, 0);
    }
    else
    {
        static lv_style_t style_address_text;                                           // 创建一个风格
        lv_style_init(&style_address_text);                                             // 初始化风格
        lv_style_set_text_color(&style_address_text, LV_STATE_DEFAULT, LV_COLOR_WHITE); //字体颜色
        lv_style_set_text_font(&style_address_text, LV_STATE_DEFAULT, &lv_font_montserrat_16);

        lv_obj_t *label_address = lv_label_create(obj_address, NULL);
        lv_obj_add_style(label_address, LV_LABEL_PART_MAIN, &style_address_text); // 应用效果风格
        lv_label_set_text_fmt(label_address, "%s", loaction);                     // 设置显示文本
        lv_obj_align(label_address, lv_obj_get_parent(label_address), LV_ALIGN_CENTER, 0, 0);
    }
    //------------------温度temperature界面

    lv_obj_t *obj_temperature = lv_obj_create(scr_ui1, NULL);
    lv_obj_set_size(obj_temperature, 60, 40); // 设置对象大小
    lv_obj_align(obj_temperature, obj_address, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    lv_obj_add_style(obj_temperature, LV_OBJ_PART_MAIN, &style_clock);

    static lv_style_t style_temperature;                                                   // 创建一个风格
    lv_style_init(&style_temperature);                                                     // 初始化风格
    lv_style_set_text_color(&style_temperature, LV_STATE_DEFAULT, lv_color_hex(0x22a8f1)); //字体颜色#22a8f1
    lv_style_set_text_font(&style_temperature, LV_STATE_DEFAULT, &lv_font_montserrat_30);

    lv_clock.temperature_label = lv_label_create(obj_temperature, NULL);
    lv_obj_add_style(lv_clock.temperature_label, LV_LABEL_PART_MAIN, &style_temperature); // 应用效果风格
                                                                                          //------------------ 其他信息图案
    lv_obj_t *obj_other = lv_obj_create(scr_ui1, NULL);
    lv_obj_set_size(obj_other, 120, 60); // 设置对象大小
    lv_obj_align(obj_other, scr_ui1, LV_ALIGN_IN_TOP_RIGHT, 0, 15);
    lv_obj_add_style(obj_other, LV_OBJ_PART_MAIN, &style_clock);

    static lv_style_t style_other;
    lv_style_reset(&style_other);
    lv_style_init(&style_other);
    lv_style_set_text_color(&style_other, LV_STATE_DEFAULT, lv_color_hex(0xe5cab9)); //字体颜色
    lv_style_set_text_font(&style_other, LV_STATE_DEFAULT, &other_font);             //字体大小

    lv_obj_t *other_label_1 = lv_label_create(obj_other, NULL);
    lv_obj_add_style(other_label_1, LV_LABEL_PART_MAIN, &style_other); // 应用效果风格
    char wind[20];
    sprintf(wind, "%s风%d级", wind_direction, wind_scale);
    lv_label_set_text(other_label_1, wind);
    lv_obj_align(other_label_1, lv_obj_get_parent(other_label_1), LV_ALIGN_IN_TOP_LEFT, 0, 0);

    lv_obj_t *other_label_2 = lv_label_create(obj_other, NULL);
    lv_obj_add_style(other_label_2, LV_LABEL_PART_MAIN, &style_other); // 应用效果风格
    char humidity[20];
    sprintf(humidity, "湿度:%d%%", humi);
    lv_label_set_text(other_label_2, humidity);
    lv_obj_align(other_label_2, lv_obj_get_parent(other_label_2), LV_ALIGN_IN_BOTTOM_LEFT, 0, -5);

    //------------------左下脚WiFi图案
    lv_obj_t *obj_wifi = lv_obj_create(scr_ui1, NULL);
    lv_obj_set_size(obj_wifi, 150, 30); // 设置对象大小
    lv_obj_align(obj_wifi, scr_ui1, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);
    lv_obj_add_style(obj_wifi, LV_OBJ_PART_MAIN, &style_clock);

    static lv_style_t style_wifi;
    lv_style_reset(&style_wifi);
    lv_style_init(&style_wifi);
    lv_style_set_text_color(&style_wifi, LV_STATE_DEFAULT, LV_COLOR_WHITE);        //字体颜色
    lv_style_set_text_font(&style_wifi, LV_STATE_DEFAULT, &lv_font_montserrat_18); //字体大小

    lv_obj_t *wifi_label = lv_label_create(obj_wifi, NULL);
    lv_obj_add_style(wifi_label, LV_LABEL_PART_MAIN, &style_wifi); // 应用效果风格
    lv_label_set_text_fmt(wifi_label, LV_SYMBOL_WIFI " %s", wifi_ssid);
    lv_obj_align(wifi_label, lv_obj_get_parent(wifi_label), LV_ALIGN_IN_LEFT_MID, 5, 0);

    lv_task_t *task_timer = lv_task_create(clock_date_task_callback, 200, LV_TASK_PRIO_MID, &lv_clock); // 创建定时任务，200ms刷新一次
    lv_task_ready(task_timer);
}

static void lv_tick_task(void *arg)
{
    (void)arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}

static void obtain_time(void)
{

    initialize_sntp();

    // wait for time to be set
    time_t now = 0;
    struct tm timeinfo = {0};
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    time(&now);
    localtime_r(&now, &timeinfo);
}

static void initialize_sntp(void)
{
    ESP_LOGI(TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    sntp_init();
}

static void clock_date_task_callback(lv_task_t *task)
{
    static const char *week_day[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    static time_t now_time;
    static struct tm time_info;

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

    lv_label_set_text_fmt(clock->time_label, "%02d:%02d:%02d", hour, minutes, second);
    lv_obj_align(clock->time_label, lv_obj_get_parent(clock->time_label), LV_ALIGN_CENTER, 0, -10);
    // ESP_LOGI(TAG,"time : %d:%d:%d",time_info.tm_hour,time_info.tm_min,time_info.tm_sec);
    lv_label_set_text_fmt(clock->data_label, "%d-%02d-%02d", year, month, day);
    lv_obj_align(clock->data_label, lv_obj_get_parent(clock->data_label), LV_ALIGN_IN_BOTTOM_LEFT, 0, -5);

    lv_label_set_text_fmt(clock->weekday_label, "%s", week_day[weekday]);
    lv_obj_align(clock->weekday_label, lv_obj_get_parent(clock->weekday_label), LV_ALIGN_IN_BOTTOM_RIGHT, -5, -5);

    if ((code_day >= 0) && (code_day <= 3))
    {
        lv_label_set_text(clock->weather_label, weather_1); // 设置显示文本
        lv_obj_align(clock->weather_label, lv_obj_get_parent(clock->weather_label), LV_ALIGN_CENTER, 0, -10);
        lv_label_set_text(clock->weather_text_label, "晴天"); // 设置显示文本
        lv_obj_align(clock->weather_text_label, lv_obj_get_parent(clock->weather_text_label), LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    }
    else if ((code_day >= 4) && (code_day <= 8))
    {
        lv_label_set_text(clock->weather_label, weather_3); // 设置显示文本
        lv_obj_align(clock->weather_label, lv_obj_get_parent(clock->weather_label), LV_ALIGN_CENTER, 0, -10);
        lv_label_set_text(clock->weather_text_label, "多云"); // 设置显示文本
        lv_obj_align(clock->weather_text_label, lv_obj_get_parent(clock->weather_text_label), LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    }
    else if (code_day == 9)
    {
        lv_label_set_text(clock->weather_label, weather_2); // 设置显示文本
        lv_obj_align(clock->weather_label, lv_obj_get_parent(clock->weather_label), LV_ALIGN_CENTER, 0, -10);
        lv_label_set_text(clock->weather_text_label, "阴天"); // 设置显示文本
        lv_obj_align(clock->weather_text_label, lv_obj_get_parent(clock->weather_text_label), LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    }
    else if ((code_day >= 10) && (code_day <= 12))
    {
        lv_label_set_text(clock->weather_label, weather_7); // 设置显示文本
        lv_obj_align(clock->weather_label, lv_obj_get_parent(clock->weather_label), LV_ALIGN_CENTER, 0, -10);
        lv_label_set_text(clock->weather_text_label, "阵雨"); // 设置显示文本
        lv_obj_align(clock->weather_text_label, lv_obj_get_parent(clock->weather_text_label), LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    }
    else if ((code_day >= 13) && (code_day <= 14))
    {
        lv_label_set_text(clock->weather_label, weather_4); // 设置显示文本
        lv_obj_align(clock->weather_label, lv_obj_get_parent(clock->weather_label), LV_ALIGN_CENTER, 0, -10);
        lv_label_set_text(clock->weather_text_label, "雨天"); // 设置显示文本
        lv_obj_align(clock->weather_text_label, lv_obj_get_parent(clock->weather_text_label), LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    }
    else if (code_day == 15)
    {
        lv_label_set_text(clock->weather_label, weather_5); // 设置显示文本
        lv_obj_align(clock->weather_label, lv_obj_get_parent(clock->weather_label), LV_ALIGN_CENTER, 0, -10);
        lv_label_set_text(clock->weather_text_label, "大雨"); // 设置显示文本
        lv_obj_align(clock->weather_text_label, lv_obj_get_parent(clock->weather_text_label), LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    }
    else if ((code_day >= 16) && (code_day <= 18))
    {
        lv_label_set_text(clock->weather_label, weather_6); // 设置显示文本
        lv_obj_align(clock->weather_label, lv_obj_get_parent(clock->weather_label), LV_ALIGN_CENTER, 0, -10);
        lv_label_set_text(clock->weather_text_label, "暴雨"); // 设置显示文本
        lv_obj_align(clock->weather_text_label, lv_obj_get_parent(clock->weather_text_label), LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    }
    else if ((code_day >= 19) && (code_day <= 22))
    {
        lv_label_set_text(clock->weather_label, weather_8); // 设置显示文本
        lv_obj_align(clock->weather_label, lv_obj_get_parent(clock->weather_label), LV_ALIGN_CENTER, 0, -10);
        lv_label_set_text(clock->weather_text_label, "雪天"); // 设置显示文本
        lv_obj_align(clock->weather_text_label, lv_obj_get_parent(clock->weather_text_label), LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    }
    else if ((code_day >= 23) && (code_day <= 25))
    {
        lv_label_set_text(clock->weather_label, weather_9); // 设置显示文本
        lv_obj_align(clock->weather_label, lv_obj_get_parent(clock->weather_label), LV_ALIGN_CENTER, 0, -10);
        lv_label_set_text(clock->weather_text_label, "暴雪"); // 设置显示文本
        lv_obj_align(clock->weather_text_label, lv_obj_get_parent(clock->weather_text_label), LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    }
    else if ((code_day >= 30) && (code_day <= 31))
    {
        lv_label_set_text(clock->weather_label, weather_10); // 设置显示文本
        lv_obj_align(clock->weather_label, lv_obj_get_parent(clock->weather_label), LV_ALIGN_CENTER, 0, -10);
        lv_label_set_text(clock->weather_text_label, "雾天"); // 设置显示文本
        lv_obj_align(clock->weather_text_label, lv_obj_get_parent(clock->weather_text_label), LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    }
    else
    {
        lv_label_set_text(clock->weather_label, weather_11); // 设置显示文本
        lv_obj_align(clock->weather_label, lv_obj_get_parent(clock->weather_label), LV_ALIGN_CENTER, 0, -10);
        lv_label_set_text(clock->weather_text_label, "未知"); // 设置显示文本
        lv_obj_align(clock->weather_text_label, lv_obj_get_parent(clock->weather_text_label), LV_ALIGN_IN_BOTTOM_MID, 0, -5);
    }
    lv_label_set_text_fmt(clock->temperature_label, "%d°", temperature);
    lv_obj_align(clock->temperature_label, lv_obj_get_parent(clock->temperature_label), LV_ALIGN_CENTER, 0, 0);
}