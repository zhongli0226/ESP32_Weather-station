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
    while (1)
	{
		vTaskDelay(1000);
	}
}