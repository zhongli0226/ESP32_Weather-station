/*
 * @Description:
 * @Version:
 * @Autor: tangwc
 * @Date: 2023-03-04 14:35:41
 * @LastEditors: tangwc
 * @LastEditTime: 2023-03-30 21:02:41
 * @FilePath: \esp32_weather-station\components\Net_mess_task\weather_json.c
 *
 *  Copyright (c) 2023 by tangwc, All Rights Reserved.
 */
#include <stdio.h>
#include <string.h>
#include "esp_log.h"

#include "cJSON.h"
#include "cJSON_Utils.h"

#include "weather_json.h"

static const char *TAG = "weather json";

// 全局变量 用户天气信息
user_seniverse_config_t user_sen_config = {0};

/**
 * @description: 去除cjson打印出来的双引号
 * @param {char} *format_data
 * @param {char} *data
 * @return {*}
 */
static void format_json_data(char *format_data,char *data)
{
    uint32_t len = strlen(data);
    uint32_t len_f = strlen(format_data); 
    memcpy(format_data,"",len_f);
    memcpy(format_data, data + 1, len - 2);
    ESP_LOGI(TAG, "%s", format_data);
}

/**
 * @description: 解析daily api接口 心知天气json数据
 * @param {char} *json_data
 * @return {*}
 */
void cjson_to_struct_daily_info(char *json_data)
{
    uint8_t i;
    uint8_t result_array_size = 0;

    cJSON *item = NULL;
    cJSON *root = NULL;
    cJSON *results_root = NULL;
    cJSON *daily_root = NULL;
    // 截取有效json
    char *index = strchr(json_data, '{');
    strcpy(json_data, index);
    root = cJSON_Parse(json_data); /*json_data 为心知天气的原始数据*/
    if (!root)
    {
        ESP_LOGI(TAG, "Error before: [%s]\n", cJSON_GetErrorPtr());
    }
    else
    {
        // ESP_LOGI(TAG,"%s\n\n", cJSON_Print(root));				   /*将完整的数据以JSON格式打印出来*/
        cJSON *Presult = cJSON_GetObjectItem(root, "results"); /*results 的键值对为数组，*/
        result_array_size = cJSON_GetArraySize(Presult);       /*求results键值对数组中有多少个元素*/
        for (i = 0; i < result_array_size; i++)
        {
            cJSON *item_results = cJSON_GetArrayItem(Presult, i);

            char *sresults = cJSON_PrintUnformatted(item_results);
            results_root = cJSON_Parse(sresults);
            if (!results_root)
            {
                ESP_LOGI(TAG, "Error before: [%s]\n", cJSON_GetErrorPtr());
            }
            else
            {
                cJSON *Plocation = cJSON_GetObjectItem(results_root, "location");

                item = cJSON_GetObjectItem(Plocation, "name");
                format_json_data(user_sen_config.name,cJSON_Print(item));
                ESP_LOGI(TAG, "name:%s\n", cJSON_Print(item));
                // strcpy(name, user_sen_config.name);
                /*-------------------------------------------------------------------*/
                cJSON *Pdaily = cJSON_GetObjectItem(results_root, "daily");

                cJSON *item_daily = cJSON_GetArrayItem(Pdaily, 0);

                char *sdaily = cJSON_PrintUnformatted(item_daily);

                daily_root = cJSON_Parse(sdaily);

                if (!daily_root)
                {
                    ESP_LOGI(TAG, "Error before: [%s]\n", cJSON_GetErrorPtr());
                }
                else
                {
                    item = cJSON_GetObjectItem(daily_root, "high");
                    format_json_data(user_sen_config.high,cJSON_Print(item));
                    ESP_LOGI(TAG, "high:%s\n", cJSON_Print(item));

                    item = cJSON_GetObjectItem(daily_root, "low");
                    format_json_data(user_sen_config.low,cJSON_Print(item));
                    ESP_LOGI(TAG, "low:%s\n", cJSON_Print(item));

                    item = cJSON_GetObjectItem(daily_root, "wind_direction");
                    format_json_data(user_sen_config.wind_direction,cJSON_Print(item));
                    ESP_LOGI(TAG, "wind_direction:%s\n", cJSON_Print(item));
                    // strcpy(wind_direction, user_sen_config.wind_direction);

                    item = cJSON_GetObjectItem(daily_root, "wind_scale");
                    format_json_data(user_sen_config.wind_scale,cJSON_Print(item));
                    ESP_LOGI(TAG, "wind_scale:%s\n", cJSON_Print(item));
                    // wind_scale = atoi(user_sen_config.wind_scale);

                    item = cJSON_GetObjectItem(daily_root, "humidity");
                    format_json_data(user_sen_config.humidity,cJSON_Print(item));
                    ESP_LOGI(TAG, "humidity:%s\n", cJSON_Print(item));
                    // humi = atoi(user_sen_config.humidity);

                    cJSON_Delete(daily_root); /*每次调用cJSON_Parse函数后，都要释放内存*/
                }
                cJSON_Delete(results_root); /*每次调用cJSON_Parse函数后，都要释放内存*/
            }
        }
    }
    cJSON_Delete(root);
}

/**
 * @description: 解析 now api接口 心知天气json数据
 * @param {char} *json_data
 * @return {*}
 */
void cjson_to_struct_now_info(char *json_data)
{
    uint8_t i;
    uint8_t result_array_size = 0;

    cJSON *item = NULL;
    cJSON *root = NULL;
    cJSON *results_root = NULL;
    // 截取有效json
    char *index = strchr(json_data, '{');
    strcpy(json_data, index);
    root = cJSON_Parse(json_data); /*json_data 为心知天气的原始数据*/
    if (!root)
    {
        ESP_LOGI(TAG, "Error before: [%s]\n", cJSON_GetErrorPtr());
    }
    else
    {
        // ESP_LOGI(TAG,"%s\n\n", cJSON_Print(root));				   /*将完整的数据以JSON格式打印出来*/
        cJSON *Presult = cJSON_GetObjectItem(root, "results"); /*results 的键值对为数组，*/
        result_array_size = cJSON_GetArraySize(Presult);       /*求results键值对数组中有多少个元素*/
        for (i = 0; i < result_array_size; i++)
        {
            cJSON *item_results = cJSON_GetArrayItem(Presult, i);

            char *sresults = cJSON_PrintUnformatted(item_results);
            results_root = cJSON_Parse(sresults);
            if (!results_root)
            {
                ESP_LOGI(TAG, "Error before: [%s]\n", cJSON_GetErrorPtr());
            }
            else
            {
                cJSON *Plocation = cJSON_GetObjectItem(results_root, "location");

                item = cJSON_GetObjectItem(Plocation, "name");
                format_json_data(user_sen_config.name,cJSON_Print(item));
                ESP_LOGI(TAG, "name(%d):%s\n", strlen(user_sen_config.name), cJSON_Print(item));

                cJSON *Pdaily = cJSON_GetObjectItem(results_root, "now");

                item = cJSON_GetObjectItem(Pdaily, "text");
                format_json_data(user_sen_config.text,cJSON_Print(item));
                ESP_LOGI(TAG, "text_day:%s\n", cJSON_Print(item));

                item = cJSON_GetObjectItem(Pdaily, "code");
                format_json_data(user_sen_config.code,cJSON_Print(item));
                // code_day = atoi(user_sen_config.code);
                ESP_LOGI(TAG, "code_day:%s\n", cJSON_Print(item));

                item = cJSON_GetObjectItem(Pdaily, "temperature");
                format_json_data(user_sen_config.temperature,cJSON_Print(item));
                // temperature = atoi(user_sen_config.temperature);
                ESP_LOGI(TAG, "temperature:%s\n", cJSON_Print(item));

                cJSON_Delete(results_root); /*每次调用cJSON_Parse函数后，都要释放内存*/
            }
        }
        cJSON_Delete(root);
    }
}