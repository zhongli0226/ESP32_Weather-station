#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"

#include "smart_config.h"
#include "wifi_nvs.h"
#include "network_task.h"


char wifi_name[WIFI_LEN] = { 0 };
char wifi_password[WIFI_LEN] = { 0 };

void network_task_handler(void* pvParameter)
{
    (void)pvParameter;
    
    if (Get_nvs_wifi(wifi_name, wifi_password) == 1) // 判断是否有连接标志
    {
        wifi_station_normal_init();
    }
    else
    {
        wifi_smart_config_init();
    }
    while (1)
	{
		vTaskDelay(1000);
	}
}