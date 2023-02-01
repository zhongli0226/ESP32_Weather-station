/*
 * @Description:
 * @Version:
 * @Autor: tangwc
 * @Date: 2022-12-21 20:10:30
 * @LastEditors: tangwc tangwc@chipsea.com
 * @LastEditTime: 2023-02-01 14:24:12
 * @FilePath: \esp32_weather-station-develop\components\task_define\task_define.c
 *
 *  Copyright (c) 2022 by tangwc, All Rights Reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "esp_check.h"


#include "ui_task.h"
#include "network_task.h"
#include "task_define.h"

static const char *TAG = "task_define";

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

/***************QUEUE DEINE*******************/
// #define QUEUE_GUI_LEN 5
// #define QUEUE_GUI_SIZE (sizeof(message_define_t))

// static xQueueHandle g_gui_queue_handle;


// static queue_define_t g_gui_queue_info = {
//     .uxQueueLength = QUEUE_GUI_LEN,
//     .uxItemSize = QUEUE_GUI_SIZE,
//     .queue = &g_gui_queue_handle,
// };
/***************TASK DEINE********************/
#define GUI_TASK "gui_task"
#define NET_TASK "net_task"

#define TASK_GUI_STACK_SIZE (4096*2)
#define TASK_GUI_PRIORITY  (0) 

#define TASK_NET_STACK_SIZE (4096*2)
#define TASK_NET_PRIORITY  (1) 

static TaskHandle_t g_gui_task_handle;
static TaskHandle_t g_net_task_handle;

static task_define_t g_task_define_tab[] = {
    {
        .pvTaskCode = (void*)gui_task_handler,
        .pcName = GUI_TASK,
        .usStackDepth = TASK_GUI_STACK_SIZE,
        .pvParameters = NULL,
        .uxPriority = TASK_GUI_PRIORITY,
        .pxCreatedTask = &g_gui_task_handle,
        //.queue_define = &g_gui_queue_info,
    },
    {
        .pvTaskCode = (void*)network_task_handler,
        .pcName = NET_TASK,
        .usStackDepth = TASK_NET_STACK_SIZE,
        .pvParameters = NULL,
        .uxPriority = TASK_NET_PRIORITY,
        .pxCreatedTask = &g_net_task_handle,
        //.queue_define = &g_gui_queue_info,
    },
};


BaseType_t create_app_task(void)
{
    uint16_t index = 0;
    for (index = 0; index < ARRAY_SIZE(g_task_define_tab); index++)
    {
        /* 要先创建队列 */
        // if (g_task_define_tab[index].queue_define != NULL)
        // {
        //     queue_define_t* q_def = g_task_define_tab[index].queue_define;
        //     *q_def->queue = xQueueCreate(q_def->uxQueueLength, q_def->uxItemSize);
        //     if (*q_def->queue == NULL)
        //     {
        //         return ESP_FAIL;
        //     }
        // }
        /* 任务创建，再创建任务 */
        ESP_RETURN_ON_FALSE(!(*g_task_define_tab[index].pxCreatedTask),
            ESP_ERR_INVALID_STATE,
            TAG,
            "%s already started", g_task_define_tab[index].pcName);

        xTaskCreatePinnedToCore(g_task_define_tab[index].pvTaskCode,
            g_task_define_tab[index].pcName,
            g_task_define_tab[index].usStackDepth,
            g_task_define_tab[index].pvParameters,
            g_task_define_tab[index].uxPriority,
            g_task_define_tab[index].pxCreatedTask,
            tskNO_AFFINITY);

        ESP_RETURN_ON_FALSE((*g_task_define_tab[index].pxCreatedTask),
            ESP_FAIL,
            TAG,
            "%s failed", g_task_define_tab[index].pcName);
        vTaskDelay(10);
    }
    return ESP_OK;
}


/***********************获取任务句柄函数*******************************/

void* get_gui_task_handle(void)
{
    return (TaskHandle_t)g_gui_task_handle;
}

void* get_net_task_handle(void)
{
    return (TaskHandle_t)g_net_task_handle;
}

/***********************获取任务消息队列句柄函数*******************************/

// void* get_gui_task_queue_handle(void)
// {
//     return (xQueueHandle)g_gui_queue_handle;
// }
