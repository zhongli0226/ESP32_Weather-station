/*
 * @Description: 
 * @Version: 
 * @Autor: tangwc
 * @Date: 2022-11-02 21:36:22
 * @LastEditors: tangwc
 * @LastEditTime: 2022-11-20 21:18:11
 * @FilePath: \esp32_wifi_link\components\wifi_station\wifi_station.h
 * 
 *  Copyright (c) 2022 by tangwc, All Rights Reserved. 
 */
#ifndef __WIFI_STATION_H__
#define __WIFI_STATION_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"


extern SemaphoreHandle_t wifi_sem; //信号量

void wifi_captive_portal_connect(void);
void wifi_normal_connect(void);

#endif
