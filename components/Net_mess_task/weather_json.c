#include <stdio.h>
#include <string.h>
#include "esp_log.h"

#include "cJSON.h"
#include "cJSON_Utils.h"

#include "weather_json.h"

static const char *TAG = "weather json";


//全局变量 用户天气信息
user_seniverse_config_t user_sen_config;


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
    //截取有效json
    char *index = strchr(json_data, '{');
    strcpy(json_data, index);
    root = cJSON_Parse(json_data); /*json_data 为心知天气的原始数据*/
    if (!root)
    {
        ESP_LOGI(TAG,"Error before: [%s]\n", cJSON_GetErrorPtr());
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
                ESP_LOGI(TAG,"Error before: [%s]\n", cJSON_GetErrorPtr());
            }
            else
            {
                cJSON *Plocation = cJSON_GetObjectItem(results_root, "location");

                item = cJSON_GetObjectItem(Plocation, "name");
                user_sen_config.name = item->valuestring;
                ESP_LOGI(TAG,"name:%s\n", cJSON_Print(item));
                //strcpy(name, user_sen_config.name);
                /*-------------------------------------------------------------------*/
                cJSON *Pdaily = cJSON_GetObjectItem(results_root, "daily");

                cJSON *item_daily = cJSON_GetArrayItem(Pdaily, 0);

                char *sdaily = cJSON_PrintUnformatted(item_daily);

                daily_root = cJSON_Parse(sdaily);

                if (!daily_root)
                {
                    ESP_LOGI(TAG,"Error before: [%s]\n", cJSON_GetErrorPtr());
                }
                else
                {

                    item = cJSON_GetObjectItem(daily_root, "wind_direction");
                    user_sen_config.wind_direction = item->valuestring;
                    ESP_LOGI(TAG,"wind_direction:%s\n", cJSON_Print(item));
                    //strcpy(wind_direction, user_sen_config.wind_direction);

                    item = cJSON_GetObjectItem(daily_root, "wind_scale");
                    user_sen_config.wind_scale = item->valuestring;
                    ESP_LOGI(TAG,"wind_scale:%s\n", cJSON_Print(item));
                    //wind_scale = atoi(user_sen_config.wind_scale);

                    item = cJSON_GetObjectItem(daily_root, "humidity");
                    user_sen_config.humidity = item->valuestring;
                    ESP_LOGI(TAG,"humidity:%s\n", cJSON_Print(item));
                    //humi = atoi(user_sen_config.humidity);

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
    //截取有效json
    char *index = strchr(json_data, '{');
    strcpy(json_data, index);
    root = cJSON_Parse(json_data); /*json_data 为心知天气的原始数据*/
    if (!root)
    {
        ESP_LOGI(TAG,"Error before: [%s]\n", cJSON_GetErrorPtr());
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
                ESP_LOGI(TAG,"Error before: [%s]\n", cJSON_GetErrorPtr());
            }
            else
            {
                cJSON *Plocation = cJSON_GetObjectItem(results_root, "location");

                item = cJSON_GetObjectItem(Plocation, "name");
                user_sen_config.name = cJSON_Print(item);
                ESP_LOGI(TAG,"name:%s\n", cJSON_Print(item));

                cJSON *Pdaily = cJSON_GetObjectItem(results_root, "now");

                item = cJSON_GetObjectItem(Pdaily, "text");
                user_sen_config.text = item->valuestring;
                ESP_LOGI(TAG,"text_day:%s\n", cJSON_Print(item));

                item = cJSON_GetObjectItem(Pdaily, "code");
                user_sen_config.code = item->valuestring;
                //code_day = atoi(user_sen_config.code);
                ESP_LOGI(TAG,"code_day:%s\n", cJSON_Print(item));

                item = cJSON_GetObjectItem(Pdaily, "temperature");
                user_sen_config.temperature = item->valuestring;
                //temperature = atoi(user_sen_config.temperature);
                ESP_LOGI(TAG,"temperature:%s\n", cJSON_Print(item));

                cJSON_Delete(results_root); /*每次调用cJSON_Parse函数后，都要释放内存*/
            }
        }
        cJSON_Delete(root);
    }
}