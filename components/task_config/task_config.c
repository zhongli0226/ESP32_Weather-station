/*
 * @Description: 
 * @Version: 
 * @Autor: tangwc
 * @Date: 2022-12-21 20:10:30
 * @LastEditors: tangwc
 * @LastEditTime: 2022-12-21 20:29:30
 * @FilePath: \esp32_weather-station\components\task_config\task_config.c
 * 
 *  Copyright (c) 2022 by tangwc, All Rights Reserved. 
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "ui_main.h"
#include "task_config.h"

TaskHandle_t g_gui_task_create_handle;


#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))


task_define_t g_task_define_tab[]={
    { 
        gui_task,
        "gui task",
        4096*2,
        NULL,
        TASK_PRIORITY_AboveNormal,
        g_gui_task_create_handle,
    },
};

