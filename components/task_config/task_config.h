/*
 * @Description: 
 * @Version: 
 * @Autor: tangwc
 * @Date: 2022-12-21 20:10:30
 * @LastEditors: tangwc
 * @LastEditTime: 2022-12-21 20:29:45
 * @FilePath: \esp32_weather-station\components\task_config\task_config.h
 * 
 *  Copyright (c) 2022 by tangwc, All Rights Reserved. 
 */
#ifndef  __TASK_CONFIG_H__
#define  __TASK_CONFIG_H__

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


typedef enum//任务优先级
{
    TASK_PRIORITY_IDLE = 7,
    TASK_PRIORITY_Low = 6,
    TASK_PRIORITY_BelowNormal = 5,
    TASK_PRIORITY_Normal = 4,
    TASK_PRIORITY_AboveNormal = 3,
    TASK_PRIORITY_High = 2,
    TASK_PRIORITY_Realtime = 1,
} task_priority_t;


typedef struct//任务结构体
{
    TaskFunction_t pvTaskCode;
    const char * const pcName;
    const uint32_t usStackDepth;
    void * const pvParameters;
    UBaseType_t uxPriority;
    TaskHandle_t * const pxCreatedTask;
    //queue_define_t *queue_define;
} task_define_t;


BaseType_t create_app_task(void);

#endif