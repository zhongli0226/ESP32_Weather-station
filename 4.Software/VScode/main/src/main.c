
/**
 * @file main
 *
 */

 /*********************
  *      INCLUDES
  *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" \
                            issue*/
#include "SDL2/SDL.h"
#include "lvgl/lvgl.h"
#include "lv_drivers/display/monitor.h"
#include "lv_drivers/indev/mouse.h"
#include "lv_drivers/indev/keyboard.h"
#include "lv_drivers/indev/mousewheel.h"
#include "lv_examples/lv_examples.h"
  /*********************
   *      DEFINES
   *********************/
LV_IMG_DECLARE(Sunny);
#define WEATHER_1  "\xEE\x98\x85"  //11
#define WEATHER_2  "\xEE\x98\x86"  //15
#define WEATHER_3  "\xEE\x98\x87"  //17
#define WEATHER_4  "\xEE\x98\x88"  //16
#define WEATHER_5  "\xEE\x98\x89"  //12
#define WEATHER_6  "\xEE\x98\x8A"  //0 2 38
#define WEATHER_7  "\xEE\x98\x8B"  //13
#define WEATHER_8  "\xEE\x98\x8C"  //18
#define WEATHER_9  "\xEE\x98\x8D"  //6 8
#define WEATHER_10  "\xEE\x98\x8E"  //1 3
#define WEATHER_11  "\xEE\x98\x8F"  //9
#define WEATHER_12  "\xEE\x98\x90"  //14
#define WEATHER_13  "\xEE\x98\x91"  //20
#define WEATHER_14  "\xEE\x98\x92"  //10
#define WEATHER_15  "\xEE\x98\x93"  //4 5 7
#define WEATHER_16  "\xEE\x98\x94"  //25 37
#define WEATHER_17  "\xEE\x98\x95"  //24
#define WEATHER_18  "\xEE\x98\x96"  //19
#define WEATHER_19  "\xEE\x98\x97"  //28
#define WEATHER_20  "\xEE\x98\x98"  //26
#define WEATHER_21  "\xEE\x98\x99"  //30
#define WEATHER_22  "\xEE\x98\x9A"  //29
#define WEATHER_23  "\xEE\x98\x9B"  //22
#define WEATHER_24  "\xEE\x98\x9C"  //31
#define WEATHER_25  "\xEE\x98\x9D"  //27
#define WEATHER_26  "\xEE\x98\x9E"  //23
#define WEATHER_27  "\xEE\x98\x9F"  //32 33 34 35 36 
#define WEATHER_28  "\xEE\x98\xA0"  //21
#define WEATHER_29  "\xEE\x99\xB6"  //99

#define HUMIDITU  "\xEE\x9C\x9D"
#define WIND_MESS "\xEE\x9A\x96"
/**********************
*      TYPEDEFS
**********************/
typedef struct _lv_clock
{
  lv_obj_t* time_label; // 时间标签
  lv_obj_t* data_label; // 日期标签
  lv_obj_t* weekday_label; //星期标签
  lv_obj_t* temperature_daily_label;//当日天气范围
  lv_obj_t* temperature_now_label;//实时温度
  lv_obj_t* weather_code_label;//天气代码
  lv_obj_t* ip_label;//地点标签
  lv_obj_t* humi_label;//湿度
  lv_obj_t* wind_label;//风力
}lv_clock_t;
/**********************
 *  STATIC PROTOTYPES
 **********************/
static void hal_init(void);
static int tick_thread(void* data);
static void memory_monitor(lv_task_t* param);

/**********************
 *  STATIC VARIABLES
 **********************/
lv_disp_buf_t disp_buf1;
lv_color_t buf1_1[LV_HOR_RES_MAX * 120];
lv_disp_drv_t disp_drv;
lv_indev_drv_t mouse_drv;
lv_indev_drv_t keyb_drv;
lv_indev_drv_t enc_drv;


static int flag_wifi = 0;
static int flag_history = 0;
static int flag_connect_succ = 0;

/**********************
 *      MACROS
 **********************/
 // bar任务
static void bar_anim(lv_task_t* t)
{
  static uint32_t x = 0;				// 静态变量Bar可变值
  static uint32_t state = 0;
  static char buf[64];
  lv_obj_t* bar = t->user_data;		// 从任务参数中获取bar对象
  lv_bar_set_value(bar, x, LV_ANIM_OFF);				// 设置值，关闭动画
  switch (state)
  {
  case 0:
    if (!flag_history)
    {
      if (x < 20)
      {
        x++;
      }
      lv_snprintf(buf, sizeof(buf), "Find historical data!");
      lv_obj_set_style_local_value_str(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, buf);// 显示字符串
      //printf("查找历史数据\n");
    }
    else           //到来事件状态改变，进度条设置为20
    {
      state = 1;
      x = 20;
    }
    break;
  case 1:
    if (!flag_wifi)
    {
      if (x < 50)
      {
        x++;
      }
      lv_snprintf(buf, sizeof(buf), "WiFi Connect!");
      lv_obj_set_style_local_value_str(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, buf);// 显示字符串
      //printf("wifi 连接\n");
    }
    else            //到来事件状态改变，进度条设置为50
    {
      state = 2;
      x = 50;
    }
    break;
  case 2:
    if (!flag_connect_succ)
    {
      if (x < 100)
      {
        x++;
      }
      lv_snprintf(buf, sizeof(buf), "Wait for connect! ");
      lv_obj_set_style_local_value_str(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, buf);// 显示字符串
      //printf("等待连接成功\n");
    }
    else            //到来事件状态改变，进度条设置为100
    {
      state = 3;
      x = 100;
      lv_snprintf(buf, sizeof(buf), "Connect success! ");
      lv_obj_set_style_local_value_str(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, buf);// 显示字符串
    }
    break;

  default:
    break;
  }
}
static void Events_Simulation(lv_task_t* t)
{
  static uint32_t time = 0;
  time++;
  if (time == 4)
  {
    flag_history = 1;
  }
  if (time == 7)
  {
    flag_wifi = 1;
  }
  if (time == 9)
  {
    flag_connect_succ = 1;
  }
  if (time >= 10)
  {
    time = 10;
  }
}
void bar_demo(void)
{
  lv_obj_t* bar = lv_bar_create(lv_scr_act(), NULL);
  lv_bar_set_type(bar, LV_BAR_TYPE_NORMAL);
  lv_obj_set_size(bar, 200, 20);                    // 控件尺寸
  lv_obj_align(bar, NULL, LV_ALIGN_CENTER, 0, 0);   // 对齐到中心
  lv_bar_set_range(bar, 0, 100);                    // 设置范围，最小值，最大值
  // 设置本地属性->字体
  lv_obj_set_style_local_value_font(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, &lv_font_montserrat_16);
  // 设置本地属性->对齐
  lv_obj_set_style_local_value_align(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
  // 设置本地属性->Y轴偏移
  lv_obj_set_style_local_value_ofs_y(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_DPI / 20);
  // 设置本地属性->底部边距
  lv_obj_set_style_local_margin_bottom(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_DPI / 7);
  // 设置字体颜色
  lv_obj_set_style_local_value_color(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x2196f3));
  //设置进度条背景颜色
  lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0xf5f5f5));
  // 设置进度条颜色
  lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT, lv_color_hex(0x2196f3));
  // 设置进度条边框颜色和大小
  lv_obj_set_style_local_border_color(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x2196f3));
  lv_obj_set_style_local_border_width(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_pad_all(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, 6);
  lv_obj_set_style_local_radius(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, 6);

  lv_bar_set_start_value(bar, 0, LV_ANIM_OFF);      // 设置开始值，非动画方式
  lv_task_create(bar_anim, 50, LV_TASK_PRIO_LOWEST, bar);
  lv_task_create(Events_Simulation, 700, LV_TASK_PRIO_LOWEST, NULL);
}
static void clock_date_task_callback(lv_task_t* task)
{
  static const char* week_day[7] = { "Sunday","Monday","Tuesday","Wednesday","Thursday","Friday","Saturday" };
  static time_t unix_time;
  static struct tm* time_info;
  unix_time = time(NULL);
  time_info = localtime(&unix_time);

  int year = time_info->tm_year + 1900;
  int month = time_info->tm_mon + 1;
  int day = time_info->tm_mday;
  int weekday = time_info->tm_wday;
  int hour = time_info->tm_hour;
  int minutes = time_info->tm_min;
  int second = time_info->tm_sec;

  lv_clock_t* clock = (lv_clock_t*)(task->user_data);

  lv_label_set_text_fmt(clock->time_label, "%02d:%02d:%02d", hour, minutes, second);
  lv_obj_align(clock->time_label, lv_obj_get_parent(clock->time_label), LV_ALIGN_CENTER, 0, -10);

  lv_label_set_text_fmt(clock->data_label, "%d-%02d-%02d", year, month, day);
  lv_obj_align(clock->data_label, lv_obj_get_parent(clock->data_label), LV_ALIGN_IN_BOTTOM_LEFT, 5, -5);

  lv_label_set_text_fmt(clock->weekday_label, "%s", week_day[weekday]);
  lv_obj_align(clock->weekday_label, lv_obj_get_parent(clock->weekday_label), LV_ALIGN_IN_BOTTOM_RIGHT, -5, -5);
}
void lv_main_ui(void)
{
  int temperature = 30;
  char* ip = "shenzhen";
  char* humi = "70";
  char* wind = "2";
  static lv_style_t style_bg;
  lv_style_init(&style_bg);
  lv_style_set_bg_color(&style_bg, LV_STATE_DEFAULT, lv_color_hex(0x41485a));
  lv_obj_add_style(lv_scr_act(), LV_OBJ_PART_MAIN, &style_bg);

  static lv_clock_t lv_clock = { 0 };
  ///////////////////////////////////////中间时钟部分设计
  static lv_style_t style_clock;
  lv_style_reset(&style_clock);
  lv_style_init(&style_clock);
  lv_style_set_bg_color(&style_clock, LV_STATE_DEFAULT, lv_color_hex(0x41485a));
  lv_style_set_border_color(&style_clock, LV_STATE_DEFAULT, lv_color_hex(0x41485a));

  static lv_style_t style_lab_time;
  lv_style_reset(&style_lab_time);
  lv_style_init(&style_lab_time);
  lv_style_set_text_color(&style_lab_time, LV_STATE_DEFAULT, LV_COLOR_ORANGE); //字体颜色
  lv_style_set_text_font(&style_lab_time, LV_STATE_DEFAULT, &lv_font_montserrat_48);//字体大小

  static lv_style_t style_lab_data_weekday;
  lv_style_reset(&style_lab_data_weekday);
  lv_style_init(&style_lab_data_weekday);
  lv_style_set_text_color(&style_lab_data_weekday, LV_STATE_DEFAULT, LV_COLOR_WHITE); //字体颜色
  lv_style_set_text_font(&style_lab_data_weekday, LV_STATE_DEFAULT, &lv_font_montserrat_18);//字体大小

  lv_obj_t* obj_mid = lv_obj_create(lv_scr_act(), NULL);
  lv_obj_set_size(obj_mid, 210, 80); // 设置对象大小
  lv_obj_align(obj_mid, lv_scr_act(), LV_ALIGN_CENTER, 0, 0);
  lv_obj_add_style(obj_mid, LV_OBJ_PART_MAIN, &style_clock);

  lv_clock.time_label = lv_label_create(obj_mid, NULL);//创建文本
  lv_obj_add_style(lv_clock.time_label, LV_LABEL_PART_MAIN, &style_lab_time);

  lv_clock.data_label = lv_label_create(obj_mid, NULL);
  lv_obj_add_style(lv_clock.data_label, LV_LABEL_PART_MAIN, &style_lab_data_weekday);

  lv_clock.weekday_label = lv_label_create(obj_mid, NULL);
  lv_obj_add_style(lv_clock.weekday_label, LV_LABEL_PART_MAIN, &style_lab_data_weekday);
  ////////////////////////////顶部天气部分设计
  static lv_style_t style_weather;
  lv_style_reset(&style_weather);
  lv_style_init(&style_weather);
  lv_style_set_bg_color(&style_weather, LV_STATE_DEFAULT, lv_color_hex(0x41485a));
  lv_style_set_border_color(&style_weather, LV_STATE_DEFAULT, lv_color_hex(0x41485a));

  lv_obj_t* obj_top = lv_obj_create(lv_scr_act(), NULL);
  lv_obj_set_size(obj_top, 240, 80); // 设置对象大小
  lv_obj_align(obj_top, lv_scr_act(), LV_ALIGN_IN_TOP_MID, 0, 0);
  lv_obj_add_style(obj_top, LV_OBJ_PART_MAIN, &style_weather);

  static lv_style_t style_weather_text;											// 创建一个风格
  lv_style_init(&style_weather_text);// 初始化风格
  lv_style_set_text_color(&style_weather_text, LV_STATE_DEFAULT, lv_color_hex(0xff00fa)); //字体颜色
  lv_style_set_text_font(&style_weather_text, LV_STATE_DEFAULT, &lv_font_montserrat_48);

  static lv_style_t style_weather_text2;											// 创建一个风格
  lv_style_init(&style_weather_text2);// 初始化风格
  lv_style_set_text_color(&style_weather_text2, LV_STATE_DEFAULT, lv_color_hex(0xff00fa)); //字体颜色
  lv_style_set_text_font(&style_weather_text2, LV_STATE_DEFAULT, &lv_font_montserrat_18);

  static lv_style_t weather_code_style;
  lv_style_init(&weather_code_style);
  lv_style_set_text_color(&weather_code_style, LV_STATE_DEFAULT, lv_color_hex(0x79b8ff)); //字体颜色
  lv_style_set_text_font(&weather_code_style, LV_STATE_DEFAULT, &my_font_70);

  lv_clock.weather_code_label = lv_label_create(obj_top, NULL);
  lv_obj_add_style(lv_clock.weather_code_label, LV_LABEL_PART_MAIN, &weather_code_style);		// 应用效果风格
  lv_label_set_text(lv_clock.weather_code_label, WEATHER_22);// 设置显示文本
  lv_obj_align(lv_clock.weather_code_label, obj_top, LV_ALIGN_IN_TOP_LEFT, 20, 5); // 重新设置对齐

  lv_clock.temperature_now_label = lv_label_create(obj_top, NULL);
  lv_obj_add_style(lv_clock.temperature_now_label, LV_LABEL_PART_MAIN, &style_weather_text);		// 应用效果风格
  lv_label_set_text_fmt(lv_clock.temperature_now_label, "%d°", temperature);
  lv_obj_align(lv_clock.temperature_now_label, obj_top, LV_ALIGN_IN_TOP_MID, 55, 0);

  lv_clock.temperature_daily_label = lv_label_create(obj_top, NULL);
  lv_obj_add_style(lv_clock.temperature_daily_label, LV_LABEL_PART_MAIN, &style_weather_text2);		// 应用效果风格
  lv_label_set_text_fmt(lv_clock.temperature_daily_label, LV_SYMBOL_UP " %d°    " LV_SYMBOL_DOWN " %d°", temperature, temperature);
  lv_obj_align(lv_clock.temperature_daily_label, obj_top, LV_ALIGN_IN_BOTTOM_RIGHT, -15, -5);
  ////////////////////////////////////底部天气地点设定
  lv_obj_t* obj_bottom = lv_obj_create(lv_scr_act(), NULL);
  lv_obj_set_size(obj_bottom, 240, 80); // 设置对象大小
  lv_obj_align(obj_bottom, lv_scr_act(), LV_ALIGN_IN_BOTTOM_MID, 0, 0);
  lv_obj_add_style(obj_bottom, LV_OBJ_PART_MAIN, &style_weather);

  lv_clock.ip_label = lv_label_create(obj_bottom, NULL);
  lv_obj_add_style(lv_clock.ip_label, LV_LABEL_PART_MAIN, &style_weather_text2);		// 应用效果风格
  lv_label_set_text_fmt(lv_clock.ip_label, LV_SYMBOL_GPS " %s", ip);
  lv_obj_align(lv_clock.ip_label, obj_bottom, LV_ALIGN_IN_BOTTOM_LEFT, 0, 0);

  static lv_style_t icon_style;
  lv_style_init(&icon_style);
  lv_style_set_text_color(&icon_style, LV_STATE_DEFAULT, lv_color_hex(0x79b8ff)); //字体颜色
  lv_style_set_text_font(&icon_style, LV_STATE_DEFAULT, &my_font_30);

  static lv_style_t humi_style;
  lv_style_init(&humi_style);
  lv_style_set_text_color(&humi_style, LV_STATE_DEFAULT, lv_color_hex(0x79b8ff)); //字体颜色
  lv_style_set_text_font(&humi_style, LV_STATE_DEFAULT, &lv_font_montserrat_32);

  lv_obj_t* humi_icon_label = lv_label_create(obj_bottom, NULL);
  lv_obj_add_style(humi_icon_label, LV_LABEL_PART_MAIN, &icon_style);		// 应用效果风格
  lv_label_set_text_fmt(humi_icon_label, HUMIDITU);
  lv_obj_align(humi_icon_label, obj_bottom, LV_ALIGN_IN_TOP_LEFT, 5, 10);

  lv_clock.humi_label = lv_label_create(obj_bottom, NULL);
  lv_obj_add_style(lv_clock.humi_label, LV_LABEL_PART_MAIN, &humi_style);		// 应用效果风格
  lv_label_set_text_fmt(lv_clock.humi_label, "%s", humi);
  lv_obj_align(lv_clock.humi_label, obj_bottom, LV_ALIGN_IN_TOP_LEFT, 40, 5);

  lv_obj_t* wind_icon_label = lv_label_create(obj_bottom, NULL);
  lv_obj_add_style(wind_icon_label, LV_LABEL_PART_MAIN, &icon_style);		// 应用效果风格
  lv_label_set_text_fmt(wind_icon_label, WIND_MESS);
  lv_obj_align(wind_icon_label, obj_bottom, LV_ALIGN_IN_TOP_MID, 35, 10);

  lv_clock.wind_label = lv_label_create(obj_bottom, NULL);
  lv_obj_add_style(lv_clock.wind_label, LV_LABEL_PART_MAIN, &humi_style);		// 应用效果风格
  lv_label_set_text_fmt(lv_clock.wind_label, "%s", wind);
  lv_obj_align(lv_clock.wind_label, obj_bottom, LV_ALIGN_IN_TOP_MID, 70, 5);

  lv_task_t* task_timer = lv_task_create(clock_date_task_callback, 200, LV_TASK_PRIO_MID, &lv_clock);  // 创建定时任务，200ms刷新一次
  lv_task_ready(task_timer);
}
/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int main(int argc, char** argv)
{
  (void)argc; /*Unused*/
  (void)argv; /*Unused*/

  /*Initialize LVGL*/
  lv_init();

  /*Initialize the HAL (display, input devices, tick) for LVGL*/
  hal_init();

  //  lv_demo_widgets();
    /* For printer demo set resolution to 800x480 */
  //  lv_demo_printer();
  //  lv_demo_keypad_encoder();
  //  lv_demo_benchmark();
  //  lv_demo_stress();
  //  lv_demo_music();
  //bar_demo();
  lv_main_ui();
  while (1) {
    /* Periodically call the lv_task handler.
     * It could be done in a timer interrupt or an OS task too.*/
    lv_task_handler();
    usleep(5 * 1000);
  }

  return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

 /**
  * Initialize the Hardware Abstraction Layer (HAL) for the Littlev graphics
  * library
  */
static void hal_init(void) {
  /* Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
  monitor_init();

  /*Create a display buffer*/
  lv_disp_buf_init(&disp_buf1, buf1_1, NULL, LV_HOR_RES_MAX * 120);

  /*Create a display*/
  lv_disp_drv_init(&disp_drv); /*Basic initialization*/
  disp_drv.buffer = &disp_buf1;
  disp_drv.flush_cb = monitor_flush;
  lv_disp_drv_register(&disp_drv);

  /* Add the mouse as input device
   * Use the 'mouse' driver which reads the PC's mouse*/
  mouse_init();
  lv_indev_drv_init(&mouse_drv); /*Basic initialization*/
  mouse_drv.type = LV_INDEV_TYPE_POINTER;

  /*This function will be called periodically (by the library) to get the mouse position and state*/
  mouse_drv.read_cb = mouse_read;
  lv_indev_t* mouse_indev = lv_indev_drv_register(&mouse_drv);

  /*Set a cursor for the mouse*/
  LV_IMG_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
  lv_obj_t* cursor_obj = lv_img_create(lv_scr_act(), NULL); /*Create an image object for the cursor */
  lv_img_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
  lv_indev_set_cursor(mouse_indev, cursor_obj);             /*Connect the image  object to the driver*/

  /* Add the keyboard as input device
   * Use the 'keyboard' driver which reads the PC's keyboard*/
  keyboard_init();
  lv_indev_drv_init(&keyb_drv);
  keyb_drv.type = LV_INDEV_TYPE_KEYPAD;
  keyb_drv.read_cb = keyboard_read;
  lv_indev_drv_register(&keyb_drv);

  /* Add the mouse wheel as input device (encoder type)
   * Use the 'mousewheel' driver which reads the PC's mouse wheel*/
  mousewheel_init();
  lv_indev_drv_init(&enc_drv);
  enc_drv.type = LV_INDEV_TYPE_ENCODER;
  enc_drv.read_cb = mousewheel_read;
  lv_indev_drv_register(&enc_drv);

  /* Tick init.
   * You have to call 'lv_tick_inc()' in periodically to inform LittelvGL about
   * how much time were elapsed Create an SDL thread to do this*/
  SDL_CreateThread(tick_thread, "tick", NULL);

  /* Optional:
   * Create a memory monitor task which prints the memory usage in
   * periodically.*/
  lv_task_create(memory_monitor, 5000, LV_TASK_PRIO_MID, NULL);
}

/**
 * A task to measure the elapsed time for LVGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void* data) {
  (void)data;

  while (1) {
    SDL_Delay(5);   /*Sleep for 5 millisecond*/
    lv_tick_inc(5); /*Tell LittelvGL that 5 milliseconds were elapsed*/
  }

  return 0;
}

/**
 * Print the memory usage periodically
 * @param param
 */
static void memory_monitor(lv_task_t* param) {
  (void)param; /*Unused*/

  lv_mem_monitor_t mon;
  lv_mem_monitor(&mon);
  printf("used: %6d (%3d %%), frag: %3d %%, biggest free: %6d\n",
    (int)mon.total_size - mon.free_size, mon.used_pct, mon.frag_pct,
    (int)mon.free_biggest_size);
}
