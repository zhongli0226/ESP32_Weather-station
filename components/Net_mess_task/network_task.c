/*
 * @Description:
 * @Version:
 * @Autor: tangwc
 * @Date: 2023-02-04 11:09:17
 * @LastEditors: tangwc
 * @LastEditTime: 2023-02-05 16:57:27
 * @FilePath: \esp32_weather-station\components\Net_mess_task\network_task.c
 *
 *  Copyright (c) 2023 by tangwc, All Rights Reserved.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <time.h>
#include <sys/time.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_sntp.h"

#include "smart_config.h"
#include "wifi_nvs.h"
#include "weather_json.h"
#include "network_task.h"

static const char *TAG = "network task";

SemaphoreHandle_t wifi_sem; // 信号量

char wifi_name[WIFI_LEN] = {0};
char wifi_password[WIFI_LEN] = {0};

/**
 * @description: sntp 时间更新完成回调
 * @param {timeval} *tv
 * @return {*}
 */
void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

static void obtain_time(void)
{
	// sntp init
	ESP_LOGI(TAG, "Initializing SNTP");
	sntp_setoperatingmode(SNTP_OPMODE_POLL);
	sntp_setservername(0, "pool.ntp.org");
	sntp_set_time_sync_notification_cb(time_sync_notification_cb);
	sntp_init();

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

static void station_sntp_init(void)
{
	time_t now;
	struct tm timeinfo;
	time(&now);
	localtime_r(&now, &timeinfo);
	if (timeinfo.tm_year < (2022 - 1900))
	{
		ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
		obtain_time();
		// update 'now' variable with current time
		time(&now);
	}
	setenv("TZ", "CST-8", 1);
	tzset();
}

void network_task_handler(void *pvParameter)
{
	(void)pvParameter;
	// sntp 测试
	static time_t now_time;
	static struct tm time_info;
  	char strftime_buf[64];

	uint32_t result = 0;
	wifi_sem = xSemaphoreCreateBinary(); // 创建信号量

	result = xSemaphoreTake(wifi_sem, portMAX_DELAY);
	if (result == pdPASS)
	{
		ESP_LOGI(TAG, "wifi connent finish!");
		station_sntp_init(); // sntp,时间同步
	}
	while (1)
	{
		time(&now_time);
		localtime_r(&now_time, &time_info);
		strftime(strftime_buf, sizeof(strftime_buf), "%c", &time_info);
        ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);
		// ESP_LOGI(TAG,"This is network task while !");
		vTaskDelay(2000 / portTICK_PERIOD_MS);
	}
}