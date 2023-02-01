/*
 * @Description: 
 * @Version: 
 * @Autor: tangwc
 * @Date: 2022-12-21 20:10:30
 * @LastEditors: tangwc
 * @LastEditTime: 2022-12-22 21:57:02
 * @FilePath: \esp32_weather-station\components\task_define\task_define.h
 * 
 *  Copyright (c) 2022 by tangwc, All Rights Reserved. 
 */
#ifndef  __TASK_DEFINE_H__
#define  __TASK_DEFINE_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// typedef struct
// {
//     uint8_t service_id;
//     uint8_t cmd_id;
//     uint32_t msg_val;
//     uint32_t msg_len;
//     void *msg_buf;
// } message_define_t;

// typedef struct
// {
//     UBaseType_t uxQueueLength;
//     UBaseType_t uxItemSize;
//     xQueueHandle *queue;
// } queue_define_t;


typedef struct//任务结构体
{
    TaskFunction_t pvTaskCode;
    const char * const pcName;
    const uint32_t usStackDepth;
    void * const pvParameters;
    UBaseType_t uxPriority;
    TaskHandle_t * const pxCreatedTask;
    //const BaseType_t xCoreID;
    //queue_define_t *queue_define;
}task_define_t;


BaseType_t create_app_task(void);

void* get_gui_task_handle(void);
void* get_net_task_handle(void);

// void* get_gui_task_queue_handle(void);


#endif