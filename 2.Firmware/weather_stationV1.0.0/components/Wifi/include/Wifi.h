#ifndef __WIFI_H
#define __WIFI_H

#include "esp_wifi.h"
#include "esp_wpa2.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "cJSON.h"
#include "cJSON_Utils.h"




extern char wifi_ssid[32];
extern char wifi_password[64];
extern char loaction[10];
extern int code_day;
extern int temperature;
extern char wind_direction[8];
extern char name[20];
extern int wind_scale;
extern int humi;


/**
 * @brief 心知天气（seniverse） 数据结构体
 */
typedef struct
{
	char *name;//地点
    char *text;//天气
    char *code;
    char *temperature;//温度
    char *wind_direction;//风向
    char *wind_scale;//风力等级
	char *humidity;//相对湿度

} user_seniverse_config_t;


extern void wifi_init(void);

#endif
