#include <stdio.h>
#include "Wifi.h"
#include "SD_card.h"
#include "string.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

static const char *TAG = "WiFi";

// http组包宏，获取天气的http接口参数
#define WEB_SERVER "api.seniverse.com"
#define WEB_PORT "80"

static char *SiYao = "ScklFsoNV7aWkbSiU";
static char *host = "api.seniverse.com";
static const int CONNECTED_BIT = BIT0;

user_seniverse_config_t user_sen_config;
// wifi链接成功事件
static EventGroupHandle_t wifi_event_group;

// HTTP 请求任务
void http_get_task(void *pvParameters);

// wifi事件处理
static void event_handler(void *arg, esp_event_base_t event_base,
                          int32_t event_id, void *event_data)
{
    if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_START))
    {
        esp_wifi_connect();
    }
    else if ((event_base == WIFI_EVENT) && (event_id == WIFI_EVENT_STA_DISCONNECTED))
    {
        esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
    }
    else if ((event_base == IP_EVENT) && (event_id == IP_EVENT_STA_GOT_IP))
    {
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        xTaskCreate(http_get_task, "http_get_task", 8192, NULL, 3, NULL);
    }
}

// wifi初始化
void wifi_init(void)
{
    esp_netif_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // 初始化事件循环
    esp_netif_create_default_wifi_sta();
    // ESP_ERROR_CHECK(esp_event_loop_init(wifi_event_handler, NULL));
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL));
    // ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };
    strcpy((char *)wifi_config.sta.ssid, wifi_ssid);
    strcpy((char *)wifi_config.sta.password, wifi_password);
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_LOGI(TAG, "start the WIFI SSID:[%s]", wifi_config.sta.ssid);
    ESP_LOGI(TAG, "start the WIFI PASSWORD:[%s]", wifi_config.sta.password);
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_LOGI(TAG, "Waiting for wifi");
    // 等待wifi连上
    xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT, false, true, portMAX_DELAY);
}

// 解析json数据 只处理 解析 城市 天气 天气代码  温度  其他的自行扩展
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
        printf("Error before: [%s]\n", cJSON_GetErrorPtr());
    }
    else
    {
        // printf("%s\n\n", cJSON_Print(root));				   /*将完整的数据以JSON格式打印出来*/
        cJSON *Presult = cJSON_GetObjectItem(root, "results"); /*results 的键值对为数组，*/
        result_array_size = cJSON_GetArraySize(Presult);       /*求results键值对数组中有多少个元素*/
        for (i = 0; i < result_array_size; i++)
        {
            cJSON *item_results = cJSON_GetArrayItem(Presult, i);

            char *sresults = cJSON_PrintUnformatted(item_results);
            results_root = cJSON_Parse(sresults);
            if (!results_root)
            {
                printf("Error before: [%s]\n", cJSON_GetErrorPtr());
            }
            else
            {
                cJSON *Plocation = cJSON_GetObjectItem(results_root, "location");

                item = cJSON_GetObjectItem(Plocation, "name");
                user_sen_config.name = item->valuestring;
                printf("name:%s\n", cJSON_Print(item));
                strcpy(name, user_sen_config.name);
                /*-------------------------------------------------------------------*/
                cJSON *Pdaily = cJSON_GetObjectItem(results_root, "daily");

                cJSON *item_daily = cJSON_GetArrayItem(Pdaily, 0);

                char *sdaily = cJSON_PrintUnformatted(item_daily);

                daily_root = cJSON_Parse(sdaily);

                if (!daily_root)
                {
                    printf("Error before: [%s]\n", cJSON_GetErrorPtr());
                }
                else
                {

                    item = cJSON_GetObjectItem(daily_root, "wind_direction");
                    user_sen_config.wind_direction = item->valuestring;
                    printf("wind_direction:%s\n", cJSON_Print(item));
                    strcpy(wind_direction, user_sen_config.wind_direction);

                    item = cJSON_GetObjectItem(daily_root, "wind_scale");
                    user_sen_config.wind_scale = item->valuestring;
                    printf("wind_scale:%s\n", cJSON_Print(item));
                    wind_scale = atoi(user_sen_config.wind_scale);

                    item = cJSON_GetObjectItem(daily_root, "humidity");
                    user_sen_config.humidity = item->valuestring;
                    printf("humidity:%s\n", cJSON_Print(item));
                    humi = atoi(user_sen_config.humidity);

                    cJSON_Delete(daily_root); /*每次调用cJSON_Parse函数后，都要释放内存*/
                }
                cJSON_Delete(results_root); /*每次调用cJSON_Parse函数后，都要释放内存*/
            }
        }
    }
    cJSON_Delete(root);
}

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
        printf("Error before: [%s]\n", cJSON_GetErrorPtr());
    }
    else
    {
        // printf("%s\n\n", cJSON_Print(root));				   /*将完整的数据以JSON格式打印出来*/
        cJSON *Presult = cJSON_GetObjectItem(root, "results"); /*results 的键值对为数组，*/
        result_array_size = cJSON_GetArraySize(Presult);       /*求results键值对数组中有多少个元素*/
        for (i = 0; i < result_array_size; i++)
        {
            cJSON *item_results = cJSON_GetArrayItem(Presult, i);

            char *sresults = cJSON_PrintUnformatted(item_results);
            results_root = cJSON_Parse(sresults);
            if (!results_root)
            {
                printf("Error before: [%s]\n", cJSON_GetErrorPtr());
            }
            else
            {
                cJSON *Plocation = cJSON_GetObjectItem(results_root, "location");

                item = cJSON_GetObjectItem(Plocation, "name");
                user_sen_config.name = cJSON_Print(item);
                printf("name:%s\n", cJSON_Print(item));

                cJSON *Pdaily = cJSON_GetObjectItem(results_root, "now");

                item = cJSON_GetObjectItem(Pdaily, "text");
                user_sen_config.text = item->valuestring;
                printf("text_day:%s\n", cJSON_Print(item));

                item = cJSON_GetObjectItem(Pdaily, "code");
                user_sen_config.code = item->valuestring;
                code_day = atoi(user_sen_config.code);
                printf("code_day:%s\n", cJSON_Print(item));

                item = cJSON_GetObjectItem(Pdaily, "temperature");
                user_sen_config.temperature = item->valuestring;
                temperature = atoi(user_sen_config.temperature);
                printf("temperature:%s\n", cJSON_Print(item));

                cJSON_Delete(results_root); /*每次调用cJSON_Parse函数后，都要释放内存*/
            }
        }
        cJSON_Delete(root);
    }
}
// http任务
void http_get_task(void *pvParameters)
{
    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };
    struct addrinfo *res;
    struct in_addr *addr;
    int s, r;
    char recv_buf[1024];
    char mid_buf[1024];
    int index;
    int flag = 0;
    while (flag == 0)
    {
        // DNS域名解析
        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);
        if (err != 0 || res == NULL)
        {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p\r\n", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        //打印获取的IP
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        // ESP_LOGI(HTTP_TAG, "DNS lookup succeeded. IP=%s\r\n", inet_ntoa(*addr));

        //新建socket
        s = socket(res->ai_family, res->ai_socktype, 0);
        if (s < 0)
        {
            ESP_LOGE(TAG, "... Failed to allocate socket.\r\n");
            close(s);
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        //连接ip
        if (connect(s, res->ai_addr, res->ai_addrlen) != 0)
        {
            ESP_LOGE(TAG, "... socket connect failed errno=%d\r\n", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        freeaddrinfo(res);
        //发送http包
        char url[150];
        char REQUEST[512];
        sprintf(url, "/v3/weather/daily.json?key=%s&location=%s&language=zh-Hans&unit=c&start=0&days=1", SiYao, loaction);
        // http请求包
        sprintf(REQUEST, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", url, host);
        if (write(s, REQUEST, strlen(REQUEST)) < 0)
        {
            ESP_LOGE(TAG, "... socket send failed\r\n");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        //清缓存
        memset(mid_buf, 0, sizeof(mid_buf));
        //获取http应答包
        do
        {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf) - 1);
            strcat(mid_buf, recv_buf);
        } while (r > 0);
        ESP_LOGE(TAG, "Rev:%s\n", mid_buf);
        // json解析
        cjson_to_struct_daily_info(mid_buf);
        //关闭socket，http是短连接
        close(s);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        flag = 1;
    }
    while (1)
    {
        // DNS域名解析
        int err = getaddrinfo(WEB_SERVER, WEB_PORT, &hints, &res);
        if (err != 0 || res == NULL)
        {
            ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p\r\n", err, res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        //打印获取的IP
        addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
        // ESP_LOGI(HTTP_TAG, "DNS lookup succeeded. IP=%s\r\n", inet_ntoa(*addr));

        //新建socket
        s = socket(res->ai_family, res->ai_socktype, 0);
        if (s < 0)
        {
            ESP_LOGE(TAG, "... Failed to allocate socket.\r\n");
            close(s);
            freeaddrinfo(res);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }
        //连接ip
        if (connect(s, res->ai_addr, res->ai_addrlen) != 0)
        {
            ESP_LOGE(TAG, "... socket connect failed errno=%d\r\n", errno);
            close(s);
            freeaddrinfo(res);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        freeaddrinfo(res);
        //发送http包
        char url[150];
        char REQUEST[512];
        sprintf(url, "/v3/weather/now.json?key=%s&location=%s&language=zh-Hans&unit=c", SiYao, loaction);
        // http请求包
        sprintf(REQUEST, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", url, host);
        if (write(s, REQUEST, strlen(REQUEST)) < 0)
        {
            ESP_LOGE(TAG, "... socket send failed\r\n");
            close(s);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            continue;
        }
        //清缓存
        memset(mid_buf, 0, sizeof(mid_buf));
        //获取http应答包
        do
        {
            bzero(recv_buf, sizeof(recv_buf));
            r = read(s, recv_buf, sizeof(recv_buf) - 1);
            strcat(mid_buf, recv_buf);
        } while (r > 0);
        ESP_LOGE(TAG, "Rev:%s\n", mid_buf);
        // json解析
        cjson_to_struct_now_info(mid_buf);
        //关闭socket，http是短连接
        close(s);

        //延时一小时，数据1小时刷新一次
        for (int hour = 60; hour >= 0; hour--)
        {
            for (int countdown = 60; countdown >= 0; countdown--)
            {
                vTaskDelay(1000 / portTICK_PERIOD_MS);
            }
        }
    }
}
