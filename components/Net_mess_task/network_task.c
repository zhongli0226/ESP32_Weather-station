/*
 * @Description:
 * @Version:
 * @Autor: tangwc
 * @Date: 2023-02-04 11:09:17
 * @LastEditors: tangwc
 * @LastEditTime: 2023-02-27 20:58:03
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
#include "esp_netif.h"
#include "esp_tls.h"
#include "esp_http_client.h"

#include "smart_config.h"
#include "wifi_nvs.h"
#include "weather_json.h"
#include "network_task.h"

#define MAX_HTTP_OUTPUT_BUFFER 2048

static const char *TAG = "network task";

// WiFi连接成功标志 信号量
SemaphoreHandle_t wifi_sem; 

//WiFi名称和密码
char wifi_name[WIFI_LEN] = {0};
char wifi_password[WIFI_LEN] = {0};

/**
 * @description: sntp 时间更新完成回调
 * @param {timeval} *tv
 * @return {*}
 */
static void time_sync_notification_cb(struct timeval *tv)
{
	ESP_LOGI(TAG, "Notification of a time synchronization event");
}
/**
 * @description: http事件回调
 * @param {esp_http_client_event_t} *evt:
 * @return {*}
 */
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
	static char *output_buffer; // 用于存储来自事件处理程序的http请求响应的缓冲区
	static int output_len;		// 存储读取的字节数
	switch (evt->event_id)
	{
	case HTTP_EVENT_ERROR:
		ESP_LOGD(TAG, "HTTP_EVENT_ERROR");
		break;
	case HTTP_EVENT_ON_CONNECTED:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_CONNECTED");
		break;
	case HTTP_EVENT_HEADER_SENT:
		ESP_LOGD(TAG, "HTTP_EVENT_HEADER_SENT");
		break;
	case HTTP_EVENT_ON_HEADER:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value);
		break;
	case HTTP_EVENT_ON_DATA:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len);
		/*
		 *  添加了检查分块编码，因为本例中使用的分块编码的URL返回二进制数据。
		 *  但是，在使用分块编码的情况下，也可以使用事件处理程序。
		 */
		if (!esp_http_client_is_chunked_response(evt->client))
		{
			// 如果配置了user_data buffer，请将响应复制到缓冲区中
			if (evt->user_data)
			{
				memcpy(evt->user_data + output_len, evt->data, evt->data_len);
			}
			else
			{
				if (output_buffer == NULL)
				{
					output_buffer = (char *)malloc(esp_http_client_get_content_length(evt->client));
					output_len = 0;
					if (output_buffer == NULL)
					{
						ESP_LOGE(TAG, "Failed to allocate memory for output buffer");
						return ESP_FAIL;
					}
				}
				memcpy(output_buffer + output_len, evt->data, evt->data_len);
			}
			output_len += evt->data_len;
		}

		break;
	case HTTP_EVENT_ON_FINISH:
		ESP_LOGD(TAG, "HTTP_EVENT_ON_FINISH");
		if (output_buffer != NULL)
		{
			// 响应在output_buffer中累积。取消对下一行的注释以打印累计响应
			// ESP_LOG_BUFFER_HEX(TAG, output_buffer, output_len);
			free(output_buffer);
			output_buffer = NULL;
		}
		output_len = 0;
		break;
	case HTTP_EVENT_DISCONNECTED:
		ESP_LOGI(TAG, "HTTP_EVENT_DISCONNECTED");
		int mbedtls_err = 0;
		esp_err_t err = esp_tls_get_and_clear_last_error(evt->data, &mbedtls_err, NULL);
		if (err != 0)
		{
			ESP_LOGI(TAG, "Last esp error code: 0x%x", err);
			ESP_LOGI(TAG, "Last mbedtls failure: 0x%x", mbedtls_err);
		}
		if (output_buffer != NULL)
		{
			free(output_buffer);
			output_buffer = NULL;
		}
		output_len = 0;
		break;
	}
	return ESP_OK;
}
/**
 * @description: 等待10s时间更新
 * @return {*}
 */
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
/**
 * @description: sntp时间初始化
 * @return {*}
 */
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
/**
 * @description: 获取心知天气实时天气
 * @return {*}
 */
static void http_with_url_weather_now(void)
{
	char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
	esp_http_client_config_t config = {
		.url = "https://api.seniverse.com/v3/weather/now.json?key=ScklFsoNV7aWkbSiU&location=ip&language=en&unit=c",
		.event_handler = _http_event_handler,
		.user_data = local_response_buffer, // 传递本地缓冲区的地址以获取响应
	};
	esp_http_client_handle_t client = esp_http_client_init(&config);
	// GET
	esp_err_t err = esp_http_client_perform(client);
	if (err == ESP_OK)
	{
		ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
				 esp_http_client_get_status_code(client),
				 esp_http_client_get_content_length(client));
	}
	else
	{
		ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
	}

	printf("local_response_buffer:%s ", local_response_buffer); /*打印心知天气json原始数据*/
	cjson_to_struct_now_info(local_response_buffer); //json 解析
	esp_http_client_cleanup(client);
}

/**
 * @description: 获取心知天气预报天气
 * @return {*}
 */
static void http_with_url_weather_daily(void)
{
	char local_response_buffer[MAX_HTTP_OUTPUT_BUFFER] = {0};
	esp_http_client_config_t config = {
		.url = "https://api.seniverse.com/v3/weather/daily.json?key=ScklFsoNV7aWkbSiU&location=ip&language=en&unit=c",
		.event_handler = _http_event_handler,
		.user_data = local_response_buffer, // 传递本地缓冲区的地址以获取响应
	};
	esp_http_client_handle_t client = esp_http_client_init(&config);
	// GET
	esp_err_t err = esp_http_client_perform(client);
	if (err == ESP_OK)
	{
		ESP_LOGI(TAG, "HTTP GET Status = %d, content_length = %d",
				 esp_http_client_get_status_code(client),
				 esp_http_client_get_content_length(client));
	}
	else
	{
		ESP_LOGE(TAG, "HTTP GET request failed: %s", esp_err_to_name(err));
	}

	printf("local_response_buffer:%s ", local_response_buffer); /*打印心知天气json原始数据*/
	cjson_to_struct_daily_info(local_response_buffer);//json 解析
	esp_http_client_cleanup(client);
}

void network_task_handler(void *pvParameter)
{
	(void)pvParameter;
	// // sntp 测试
	// static time_t now_time;
	// static struct tm time_info;
	// char strftime_buf[64];
	uint32_t url_times = 0;
	uint32_t result = 0;
	wifi_sem = xSemaphoreCreateBinary(); // 创建信号量

	result = xSemaphoreTake(wifi_sem, portMAX_DELAY);
	if (result == pdPASS)
	{
		ESP_LOGI(TAG, "wifi connent finish!");
		station_sntp_init(); // sntp,时间同步
		http_with_url_weather_daily();
	}
	while (1)
	{
		if (url_times % 1800 == 0)
		{
			http_with_url_weather_now();
			if(user_sen_flag == 0)
				user_sen_flag = 1;
			url_times = 0;
		}
		// time(&now_time);
		// localtime_r(&now_time, &time_info);
		// strftime(strftime_buf, sizeof(strftime_buf), "%c", &time_info);
		// ESP_LOGI(TAG, "The current date/time in Shanghai is: %s", strftime_buf);
		// ESP_LOGI(TAG,"This is network task while !");
		vTaskDelay(2000 / portTICK_PERIOD_MS);
		url_times++;
	}
}