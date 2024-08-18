#include "ui_load.h"
#include "wifi_nvs.h"

lv_obj_t *scr_load; // 载入动画屏幕
lv_obj_t *scr_main; // 主显示屏幕

ui_status_t ui_status = DEFAULT_STATUS;

// bar任务
static void bar_anim(lv_task_t *t)
{
  static uint32_t x = 0; // 静态变量Bar可变值
  static uint32_t state = 0;
  static char buf[64];
  lv_obj_t *bar = t->user_data;          // 从任务参数中获取bar对象
  lv_bar_set_value(bar, x, LV_ANIM_OFF); // 设置值，关闭动画
  switch (state)
  {
  case 0: // 状态0
          // if (ui_status == DEFAULT_STATUS) // 默认事件 速度过快无法体现加载动画
    x++;
    lv_snprintf(buf, sizeof(buf), "check historical data!");
    lv_obj_set_style_local_value_str(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, buf); // 显示字符串
    if (x == 40)                                                                  // 保证加载动画流畅性
      state = 1;
    break;
  case 1:                               // 状态 1
    if (ui_status == HISTORY_DATA_FLAG) // 事件为找到了历史数据
    {
      if (x < 75)
        x++;
      lv_snprintf(buf, sizeof(buf), "Wait for connect!");
      lv_obj_set_style_local_value_str(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, buf); // 显示字符串
    }
    else if (ui_status == CAPTIVE_PORTAL_FLAG) // 事件没找到历史数据，请采用app连接
    {
      if (x < 75)
        x++;
      lv_snprintf(buf, sizeof(buf), "Connect Wifi (ESP32 station) !");
      lv_obj_set_style_local_value_str(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, buf); // 显示字符串
    }
    else // 新事件到来，改变状态,进度条设置为50
    {
      state = 2;
      x = 75;
    }
    break;
  case 2:
    if (ui_status == WIFI_CONNECT_SUCCESS) // 连接成功
    {
      x++;
      lv_snprintf(buf, sizeof(buf), "Success! Wait for Init!");
      lv_obj_set_style_local_value_str(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, buf); // 显示字符串
      if (x == 100)
      {
        lv_scr_load_anim(scr_main, LV_SCR_LOAD_ANIM_FADE_ON, 1000, 100, true);
        lv_task_del(t);
      }
    }
    else if (ui_status == WIFI_CONNECT_FAIL) // 连接失败
    {
      x = 100;
      lv_snprintf(buf, sizeof(buf), "FAIL! Restart....");
      lv_obj_set_style_local_value_str(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, buf); // 显示字符串
      lv_bar_set_value(bar, x, LV_ANIM_OFF);                                        // 设置值，关闭动画

      lv_obj_t *label = lv_label_create(lv_scr_act(), NULL); // 在主屏幕创建一个标签
      lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);    // 标签长内容框，保持控件宽度，内容过长就换行
      lv_label_set_recolor(label, true);                     // 使能字符命令重新对字符上色
      lv_obj_set_width(label, 150);
      lv_label_set_text(label, "#ff0000 " LV_SYMBOL_WARNING " Waring!# \n "
                               "Please check you WiFi!"); // 设置显示文本
      lv_obj_align(label, NULL, LV_ALIGN_CENTER, 0, -70); // 对齐到中心偏下
      lv_task_del(t);
    }
    break;

  default:
    break;
  }
}

void ui_init_bg(void)
{
  // 主屏幕背景样式
  static lv_style_t main_style_bg;
  lv_style_init(&main_style_bg);
  lv_style_set_bg_color(&main_style_bg, LV_STATE_DEFAULT, lv_color_hex(0x41485a));
  scr_main = lv_obj_create(NULL, NULL); // 创建主屏幕
  lv_obj_add_style(scr_main, LV_OBJ_PART_MAIN, &main_style_bg);
  // 加载屏幕背景风格
  static lv_style_t load_style_bg;
  lv_style_init(&load_style_bg);
  lv_style_set_bg_color(&load_style_bg, LV_STATE_DEFAULT, lv_color_hex(0xeaeff3));
  scr_load = lv_obj_create(NULL, NULL); // 创建加载屏幕
  lv_obj_add_style(scr_load, LV_OBJ_PART_MAIN, &load_style_bg);
  lv_scr_load(scr_load); // 设置加载屏幕为当前活动屏幕
}

void Loading_interface(void)
{
  lv_obj_t *bar = lv_bar_create(lv_scr_act(), NULL);
  lv_bar_set_type(bar, LV_BAR_TYPE_NORMAL);
  lv_obj_set_size(bar, 200, 20);                  // 控件尺寸
  lv_obj_align(bar, NULL, LV_ALIGN_CENTER, 0, 0); // 对齐到中心
  lv_bar_set_range(bar, 0, 100);                  // 设置范围，最小值，最大值
  // 设置本地属性->字体
  lv_obj_set_style_local_value_font(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, &lv_font_montserrat_16);
  // 设置本地属性->对齐
  lv_obj_set_style_local_value_align(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_ALIGN_OUT_BOTTOM_MID);
  // 设置本地属性->Y轴偏移
  lv_obj_set_style_local_value_ofs_y(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_DPI / 20);
  // 设置本地属性->底部边距
  lv_obj_set_style_local_margin_bottom(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, LV_DPI / 7);
  // 设置字体颜色
  lv_obj_set_style_local_value_color(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT,lv_color_hex(0xd7412e));
  //设置进度条背景颜色
  lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT,lv_color_hex(0xf5f5f5));
  // 设置进度条颜色
  lv_obj_set_style_local_bg_color(bar, LV_BAR_PART_INDIC, LV_STATE_DEFAULT,lv_color_hex(0x2196f3));
  // 设置进度条边框颜色和大小
  lv_obj_set_style_local_border_color(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, lv_color_hex(0x2196f3));
  lv_obj_set_style_local_border_width(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, 2);
  lv_obj_set_style_local_pad_all(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, 6);
  lv_obj_set_style_local_radius(bar, LV_BAR_PART_BG, LV_STATE_DEFAULT, 6);

  lv_bar_set_start_value(bar, 0, LV_ANIM_OFF); // 设置开始值，非动画方式
  lv_task_create(bar_anim, 100, LV_TASK_PRIO_LOWEST, bar);
}