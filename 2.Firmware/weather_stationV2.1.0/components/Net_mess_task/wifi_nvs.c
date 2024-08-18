/*
 * @Author: tangwc
 * @Date: 2022-10-12 11:35:21
 * @LastEditors: tangwc
 * @LastEditTime: 2022-11-20 16:57:24
 * @Description:
 * @FilePath: \esp32_wifi_link\components\wifi_nvs\wifi_nvs.c
 *
 *  Copyright (c) 2022 by tangwc, All Rights Reserved.
 */
#include <string.h>
#include <sys/param.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"

#include "nvs_flash.h"
#include "wifi_nvs.h"

static const char *TAG = "wifi nvs";

/**
 * @description: 获取nvs内存中wifi flag ssid pass等离线数据
 * @param {char} *wifi_ssid
 * @param {char} *wifi_passwd
 * @return {*} 0：无配置标记
 *             1：有配置标记
 */
uint32_t Get_nvs_wifi(char *wifi_ssid, char *wifi_passwd)
{
    esp_err_t err;
    nvs_handle_t wificonfig_handle;
    uint32_t flag = 0;
    size_t len;

    err = nvs_open("wificonfig", NVS_READWRITE, &wificonfig_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "opening NVS handle Done\n");
        ESP_LOGI(TAG, "Get ssid and pass from NVS ... \n");
        // 获取是否有wifi信息的标志位
        err = nvs_get_u32(wificonfig_handle, "flag", &flag);
        if (err == ESP_OK)
        {
            ESP_LOGI(TAG, "Get wifi config");
        }
        else
        {
            ESP_LOGI(TAG, "get err =0x%x\n", err);
        }
        if (!flag)
        {
            //没有返回0，没有获得连接标志
            ESP_LOGI(TAG, "NO Get wifi config flag!\n");
        }
        else
        {
            // 获取名称
            len = WIFI_LEN;
            err = nvs_get_str(wificonfig_handle, "ssid", wifi_ssid, &len);
            if (err == ESP_OK)
            {
                ESP_LOGI(TAG, "Get ssid success!\n");
                ESP_LOGI(TAG, "ssid=%s\n", wifi_ssid);
                ESP_LOGI(TAG, "ssid_len=%d\n", len);
            }
            else
            {
                ESP_LOGI(TAG, "get err =0x%x\n", err);
                ESP_LOGI(TAG, "Get ssid fail!\n");
            }
            // 获取密码
            len = WIFI_LEN;
            err = nvs_get_str(wificonfig_handle, "pass", wifi_passwd, &len);
            if (err == ESP_OK)
            {
                ESP_LOGI(TAG, "Get key success!\n");
                ESP_LOGI(TAG, "password=%s\n", wifi_passwd);
                ESP_LOGI(TAG, "password_len=%d\n", len);
            }
            else
            {
                ESP_LOGI(TAG, "get err =0x%x\n", err);
                ESP_LOGI(TAG, "Get ssid fail!\n");
            }
        }
        err = nvs_commit(wificonfig_handle);
        if (err == ESP_OK)
            ESP_LOGI(TAG, "nvs_commit Done\n");
        else
            ESP_LOGI(TAG, "nvs_commit Failed!\n");
    }
    // 关闭nvs
    nvs_close(wificonfig_handle);
    return flag;
}
/**
 * @description: 在nvs中写入 wifi ssid pass
 * @param {char} *wifi_ssid
 * @param {char} *wifi_passwd
 * @return {*}
 */
void Set_nvs_wifi(char *wifi_ssid, char *wifi_passwd)
{

    esp_err_t err;
    nvs_handle_t my_handle;

    err = nvs_open("wificonfig", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "opening NVS handle Done\n");
        ESP_LOGI(TAG, "Set ssid and pass from NVS ... \n");
        //写入ssid
        err = nvs_set_str(my_handle, "ssid", wifi_ssid);
        if (err == ESP_OK)
            ESP_LOGI(TAG, "set ssid success!\n");
        else
            ESP_LOGI(TAG, "set ssid fail!\n");
        //写入pass
        err = nvs_set_str(my_handle, "pass", wifi_passwd);
        if (err == ESP_OK)
            ESP_LOGI(TAG, "set password success!\n");
        else
            ESP_LOGI(TAG, "set password fail!\n");
        err = nvs_commit(my_handle);
        if (err == ESP_OK)
            ESP_LOGI(TAG, "nvs_commit Done\n");
        else
            ESP_LOGI(TAG, "nvs_commit Failed!\n");
    }
    // 关闭nvs
    nvs_close(my_handle);
    ESP_LOGI(TAG, "--------------------------------------\n");
}
/**
 * @description:  设置WiFi信息标记
 * @return {*}
 */
void Set_nvs_wifi_flag(void)
{
    esp_err_t err;
    nvs_handle_t my_handle;
    uint32_t flag = 1;
    err = nvs_open("wificonfig", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "opening NVS handle Done\n");
        ESP_LOGI(TAG, "Set wifi flag from NVS ... \n");

        err = nvs_set_u32(my_handle, "flag", flag);
        if (err == ESP_OK)
            ESP_LOGI(TAG, "set flag success!\n");
        else
            ESP_LOGI(TAG, "set flag fail!\n");
        err = nvs_commit(my_handle);
        if (err == ESP_OK)
            ESP_LOGI(TAG, "nvs_commit Done\n");
        else
            ESP_LOGI(TAG, "nvs_commit Failed!\n");
    }
    // 关闭nvs
    nvs_close(my_handle);
}
/**
 * @description:  清除WiFi信息标记
 * @return {*}
 */
void Clear_nvs_wifi_flag(void)
{
    esp_err_t err;
    nvs_handle_t my_handle;
    uint32_t flag = 0;
    err = nvs_open("wificonfig", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        ESP_LOGI(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
    }
    else
    {
        ESP_LOGI(TAG, "opening NVS handle Done\n");
        ESP_LOGI(TAG, "Set wifi flag from NVS ... \n");

        err = nvs_set_u32(my_handle, "flag", flag);
        if (err == ESP_OK)
            ESP_LOGI(TAG, "clear flag success!\n");
        else
            ESP_LOGI(TAG, "clear flag fail!\n");
        err = nvs_commit(my_handle);
        if (err == ESP_OK)
            ESP_LOGI(TAG, "nvs_commit Done\n");
        else
            ESP_LOGI(TAG, "nvs_commit Failed!\n");
    }
    // 关闭nvs
    nvs_close(my_handle);
}
