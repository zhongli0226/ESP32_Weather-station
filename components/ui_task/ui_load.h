#ifndef __UI_LOAD_H__
#define __UI_LOAD_H__

#include "lvgl/lvgl.h"


typedef enum
{
    DEFAULT_STATUS,  //默认状态
    HISTORY_DATA_FLAG, //使用历史信息 连接
    SMART_CONFIG_FLAG, //smart_config 连接 
    WIFI_CONNECT_SUCCESS,//WiFi连接成功，跳转主界面
    WIFI_CONNECT_FAIL,//wifi 连接失败 重启
}ui_status_t;//ui 初始化时状态

extern lv_obj_t *scr_load; // 载入动画屏幕
extern lv_obj_t *scr_main; // 主显示屏幕
extern ui_status_t ui_status;

void ui_init_bg(void);
void bar_demo(void);

#endif