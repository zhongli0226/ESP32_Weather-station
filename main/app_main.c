/*
 * @Description: lvgl_app
 * @Version: v0.0.1
 * @Autor: tangwc
 * @Date: 2022-09-24 22:49:57
 * @LastEditors: tangwc
 * @LastEditTime: 2022-12-24 11:13:57
 * @FilePath: \esp32_weather-station\main\app_main.c
 *
 *  Copyright (c) 2022 by tangwc, All Rights Reserved.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_vfs.h"
#include "driver/gpio.h"
#include "nvs_flash.h"

#include "task_define.h"
#include "smart_config.h"
#include "wifi_nvs.h"


char wifi_name[WIFI_LEN] = {0};
char wifi_password[WIFI_LEN] = {0};

void app_main(void)
{

	// 初始化nvs用于存放wifi或者其他需要掉电保存的东西
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
	if (Get_nvs_wifi(wifi_name, wifi_password) == 1) // 判断是否有连接标志
	{
		wifi_station_normal_init();
	}
	else
	{
		wifi_smart_config_init();
	}
	/*创建任务*/
	create_app_task();
	while (1)
	{
		vTaskDelay(1000);
	}
}
