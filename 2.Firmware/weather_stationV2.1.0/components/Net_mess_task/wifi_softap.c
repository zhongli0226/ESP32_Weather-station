/*
 * @Author: tangwc
 * @Date: 2022-10-12 11:35:21
 * @LastEditors: tangwc
 * @LastEditTime: 2022-11-06 14:46:41
 * @Description:
 * @FilePath: \esp32_wifi_link\components\wifi_softap\wifi_softap.c
 *
 *  Copyright (c) 2022 by tangwc, All Rights Reserved.
 */
#include <string.h>
#include <sys/param.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "lwip/inet.h"
#include "esp_http_server.h"

#include "wifi_softap.h"
#include "web_server.h"
#include "dns_server.h"

#define ENABLE_PASSWORD 0 //是否开启密码 1:开启 0：关闭

#define EXAMPLE_ESP_WIFI_SSID "ESP32 station" //待连接的WiFi名称
#if ENABLE_PASSWORD
#define EXAMPLE_ESP_WIFI_PASS "123456" //密码不能为空

#endif
#define EXAMPLE_MAX_STA_CONN 3 //最大接入数

static const char *TAG = "WIFI_softap";

esp_netif_t *ap_netif; //可以认为是一个soft-ap接口用于后续关闭
extern httpd_handle_t server;
SemaphoreHandle_t ap_sem; // softap 关闭信号量

void wifi_softap_event_handler(void *arg, esp_event_base_t event_base,
                        int32_t event_id, void *event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED)
    {
        wifi_event_ap_staconnected_t *event = (wifi_event_ap_staconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
    else if (event_id == WIFI_EVENT_AP_STADISCONNECTED)
    {
        wifi_event_ap_stadisconnected_t *event = (wifi_event_ap_stadisconnected_t *)event_data;
        ESP_LOGI(TAG, "station " MACSTR " leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void init_wifi_softap(void)
{
    /*
        转向HTTP服务器的警告，因为重定向流量将产生大量的无效请求
    */
    esp_log_level_set("httpd_uri", ESP_LOG_ERROR);
    esp_log_level_set("httpd_txrx", ESP_LOG_ERROR);
    esp_log_level_set("httpd_parse", ESP_LOG_ERROR);

    // 初始化网络栈
    ESP_ERROR_CHECK(esp_netif_init());

    // 创建主应用程序所需的默认事件循环
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    //前面初始化了就不需要再次初始化了
    // // 初始化Wi-Fi所需的NVS
    // ESP_ERROR_CHECK(nvs_flash_init());

    // 用默认配置初始化Wi-Fi，包括netif
    ap_netif = esp_netif_create_default_wifi_ap();

    //初始化wifi功能
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    //注册事件回调函数
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_softap_event_handler, NULL));

    wifi_config_t wifi_ap_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
#ifdef EXAMPLE_ESP_WIFI_PASS
            .password = EXAMPLE_ESP_WIFI_PASS,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK,
#else
            .authmode = WIFI_AUTH_OPEN,
#endif
            .max_connection = EXAMPLE_MAX_STA_CONN,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_AP, &wifi_ap_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    //获取wifi相关信息打印
    esp_netif_ip_info_t ip_info;
    esp_netif_get_ip_info(esp_netif_get_handle_from_ifkey("WIFI_AP_DEF"), &ip_info);

    char ip_addr[16];
    inet_ntoa_r(ip_info.ip.addr, ip_addr, 16);
    ESP_LOGI(TAG, "Set up softAP with IP: %s", ip_addr);

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:'%s' password:'%s'",
             wifi_ap_config.ap.ssid, wifi_ap_config.ap.password);
}

void captive_portal_init(void)
{
    ap_sem = xSemaphoreCreateBinary(); //创建信号量

    init_wifi_softap(); //初始化wifi-ap模式

    server = web_server_start(); //开启http服务
    dns_server_start(); //开启DNS服务
}