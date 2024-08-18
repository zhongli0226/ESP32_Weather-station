/*
 * @Author: tangwc
 * @Date: 2022-10-12 11:55:42
 * @LastEditors: tangwc
 * @LastEditTime: 2022-11-20 16:24:59
 * @Description:
 * @FilePath: \esp32_wifi_link\components\web_server\web_server.c
 *
 *  Copyright (c) 2022 by tangwc, All Rights Reserved.
 */
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>

#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"

#include "esp_wifi.h"
#include "esp_netif.h"
#include "lwip/inet.h"

#include "web_server.h"
#include "wifi_station.h"
#include "wifi_nvs.h"

#include "url.h"

static const char *TAG = "WEB_SERVER";

#define BUFF_SIZE 1024

httpd_handle_t server = NULL;
extern SemaphoreHandle_t ap_sem;
extern SemaphoreHandle_t dns_sem;
extern const char index_root_start[] asm("_binary_index_html_start");
extern const char index_root_end[] asm("_binary_index_html_end");

/**
 * @description: 一个HTTP GET处理程序 用来发送网页到服务器
 * @param {httpd_req_t} *req
 * @return {*}
 */
static esp_err_t root_get_handler(httpd_req_t *req)
{
    const uint32_t root_len = index_root_end - index_root_start;

    ESP_LOGI(TAG, "Serve html");
    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, index_root_start, root_len);

    return ESP_OK;
}

static const httpd_uri_t root = {
    .uri = "/", //根地址默认 192.168.4.1
    .method = HTTP_GET,
    .handler = root_get_handler,
    .user_ctx = NULL};

/**
 * @description: HTTP错误（404）处理程序 - 将所有请求重定向到根页面
 * @param {httpd_req_t} *req
 * @param {httpd_err_code_t} err
 * @return {*}
 */
static esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    // 设置状态
    httpd_resp_set_status(req, "302 Temporary Redirect");
    // 重定向到"/"根目录
    httpd_resp_set_hdr(req, "Location", "/");
    // iOS需要响应中的内容来检测俘虏式门户，仅仅重定向是不够的。
    httpd_resp_send(req, "Redirect to the captive portal", HTTPD_RESP_USE_STRLEN);

    ESP_LOGI(TAG, "Redirecting to root");
    return ESP_OK;
}

/**
 * @description: 一个HTTP POST处理程序 用于接受网页返回信息
 * @param {httpd_req_t} *req
 * @return {*}
 */
static esp_err_t echo_post_handler(httpd_req_t *req)
{
    char buf[100];
    // char ssid[10];
    // char pswd[10];
    int ret, remaining = req->content_len;

    while (remaining > 0)
    {
        /* 读取该请求的数据 */
        if ((ret = httpd_req_recv(req, buf,
                                  MIN(remaining, sizeof(buf)))) <= 0)
        {
            if (ret == HTTPD_SOCK_ERR_TIMEOUT)
            {
                /* 如果发生超时，重试接收 */
                continue;
            }
            return ESP_FAIL;
        }

        /* 发回相同的数据 */
        httpd_resp_send_chunk(req, buf, ret);
        remaining -= ret;

        esp_err_t ret = httpd_query_key_value(buf, "ssid", wifi_name, sizeof(wifi_name));
        if (ret == ESP_OK)
        {
            char str[32];
            memcpy(str, wifi_name, 32);
            url_decode(str, wifi_name);// 将url解码
            ESP_LOGI(TAG, "ssid = %s", wifi_name);
        }
        else
        {
            ESP_LOGI(TAG, "error = %d\r\n", ret);
        }

        ret = httpd_query_key_value(buf, "password", wifi_password, sizeof(wifi_password));
        if (ret == ESP_OK)
        {
            char str[32];
            memcpy(str, wifi_password, 32);
            url_decode(str, wifi_password);// 将url解码
            ESP_LOGI(TAG, "pswd = %s", wifi_password);
        }
        else
        {
            ESP_LOGI(TAG, "error = %d\r\n", ret);
        }
        /* 收到的日志数据 */
        ESP_LOGI(TAG, "=========== RECEIVED DATA ==========");
        ESP_LOGI(TAG, "%.*s", ret, buf);
        ESP_LOGI(TAG, "====================================");
    }

    // 结束回应
    httpd_resp_send_chunk(req, NULL, 0);
    if (strcmp(wifi_name, "\0") != 0)//密码可能为空
    {
        xSemaphoreGive(ap_sem);
        ESP_LOGI(TAG, "set wifi name and password successfully! goto station mode");
        //        xSemaphoreGive(dns_sem);
    }
    return ESP_OK;
}

//!若弹出页面出现 Header fields are too long for server to interpret问题，修改sdkconfig文件中的CONFIG_HTTPD_MAX_REQ_HDR_LEN，将其设置为更大的数
static const httpd_uri_t echo = {
    .uri = "/wifi_data",
    .method = HTTP_POST,
    .handler = echo_post_handler,
    .user_ctx = NULL};

/**
 * @description: web 服务开启运行
 * @return {httpd_handle_t} HTTP服务器实例处理程序
 */
httpd_handle_t web_server_start(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.max_open_sockets = 4; // todo 使用默认产生警告无法接入页面 待找出原因
    config.lru_purge_enable = true;

    // 启动httpd服务器
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // 设置URI处理程序
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &echo);
        httpd_register_uri_handler(server, &root);
        httpd_register_err_handler(server, HTTPD_404_NOT_FOUND, http_404_error_handler);
        return server;
    }
    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}
/**
 * @description: 停止web服务
 * @param {httpd_handle_t} server  HTTP服务器实例处理程序
 * @return {void}
 */
void web_server_stop(httpd_handle_t server)
{
    // 停止httpd服务器
    if (server)
    {
        httpd_stop(server);
    }
}
