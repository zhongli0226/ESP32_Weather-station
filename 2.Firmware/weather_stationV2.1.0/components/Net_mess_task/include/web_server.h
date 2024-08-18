/*
 * @Author: tangwc
 * @Date: 2022-10-12 11:55:42
 * @LastEditors: tangwc
 * @LastEditTime: 2022-11-02 21:32:21
 * @Description: 
 * @FilePath: \esp32_wifi_link\components\web_server\web_server.h
 *  
 *  Copyright (c) 2022 by tangwc, All Rights Reserved.
 */
#ifndef __WEB_SERVER_H__
#define __WEB_SERVER_H__

#include "esp_http_server.h"


/**
 * @description: web 服务开启运行
 * @return {httpd_handle_t} HTTP服务器实例处理程序
 */
httpd_handle_t web_server_start(void);
/**
 * @description: 停止web服务
 * @param {httpd_handle_t} server  HTTP服务器实例处理程序
 * @return {void}
 */
void web_server_stop(httpd_handle_t server);

#endif

