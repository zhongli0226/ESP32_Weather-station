/*
 * @Description: lvgl_app
 * @Version: v0.0.1
 * @Autor: tangwc
 * @Date: 2022-09-24 22:49:57
 * @LastEditors: tangwc
 * @LastEditTime: 2023-02-25 20:49:48
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

#include "esp_log.h"
#include "esp_vfs.h"
#include "driver/gpio.h"
#include "nvs_flash.h"

#include "smart_config.h"
#include "wifi_nvs.h"
#include "task_define.h"
#include "ui_load.h"

static const char *TAG = "app_main";


static void nvs_init(void);
static void wifi_init(void);

void app_main(void)
{
	nvs_init();
	/*创建任务*/
	create_app_task();
	//开启WiFi
	wifi_init();
	while (1)
	{
		//ESP_LOGI(TAG,"This is app main while !");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

static void nvs_init(void)
{
	// 初始化nvs用于存放wifi或者其他需要掉电保存的东西
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
}

static void wifi_init(void)
{
	if (Get_nvs_wifi(wifi_name, wifi_password) == 1) // 判断是否有连接标志
	{
		ESP_LOGI(TAG, "wifi_station_normal_init");
		ui_status = HISTORY_DATA_FLAG;
		wifi_station_normal_init();
	}
	else
	{
		ESP_LOGI(TAG, "wifi_smart_config_init");
		ui_status = SMART_CONFIG_FLAG;
		wifi_smart_config_init();
	}
}