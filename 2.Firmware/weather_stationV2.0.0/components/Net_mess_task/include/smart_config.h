/*
 * @Description: 
 * @Version: 
 * @Autor: tangwc
 * @Date: 2022-12-24 10:42:06
 * @LastEditors: tangwc
 * @LastEditTime: 2023-02-05 16:07:55
 * @FilePath: \esp32_weather-station\components\Net_mess_task\include\smart_config.h
 * 
 *  Copyright (c) 2022 by tangwc, All Rights Reserved. 
 */
#ifndef  __SMART_CONFIG_H__
#define  __SMART_CONFIG_H__


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

extern SemaphoreHandle_t wifi_sem; //信号量

void wifi_smart_config_init(void);
void wifi_station_normal_init(void);

#endif
