/*
 * @Author: tangwc
 * @Date: 2022-10-12 11:58:36
 * @LastEditors: tangwc
 * @LastEditTime: 2022-12-24 11:12:10
 * @Description: 
 * @FilePath: \esp32_weather-station\components\wifi_nvs\wifi_nvs.h
 *  
 *  Copyright (c) 2022 by tangwc, All Rights Reserved.
 */
#ifndef __WIFI_NVS_H__
#define __WIFI_NVS_H__


#define WIFI_LEN 32

extern char wifi_name[WIFI_LEN];
extern char wifi_password[WIFI_LEN];

uint32_t Get_nvs_wifi(char *wifi_ssid, char *wifi_passwd);
void Set_nvs_wifi(char *wifi_ssid, char *wifi_passwd);
void Set_nvs_wifi_flag(void);
void Clear_nvs_wifi_flag(void);
#endif

