/*
 * @Description: 
 * @Version: 
 * @Autor: tangwc
 * @Date: 2022-12-21 20:08:12
 * @LastEditors: tangwc
 * @LastEditTime: 2023-02-27 20:51:47
 * @FilePath: \esp32_weather-station\components\ui_task\ui_task.c
 * 
 *  Copyright (c) 2022 by tangwc, All Rights Reserved. 
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"

#include "lvgl/lvgl.h"
#include "lvgl_helpers.h"
#include "ui_task.h"
#include "ui_load.h"
#include "ui_main.h"

#define TAG "gui_task"
#define LV_TICK_PERIOD_MS 1

/*用定时器给LVGL提供时钟*/
static void lv_tick_task(void *arg)
{
    (void)arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

static SemaphoreHandle_t xGuiSemaphore;

void gui_task_handler(void *pvParameter)
{
    (void) pvParameter;

    xGuiSemaphore = xSemaphoreCreateMutex();
    lv_init(); // lvgl内核初始化

    lvgl_driver_init(); // lvgl显示接口初始化

    /*内部DMA方式*/
    lv_color_t* buf1 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);
    lv_color_t* buf2 = heap_caps_malloc(DISP_BUF_SIZE * sizeof(lv_color_t), MALLOC_CAP_DMA);

    static lv_disp_buf_t disp_buf;
    uint32_t size_in_px = DISP_BUF_SIZE;
    lv_disp_buf_init(&disp_buf, buf1, buf2, size_in_px);

    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = disp_driver_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task,
        .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    //lv_demo_widgets();
    ui_init_bg();
    Loading_interface();

    Main_interface();

    while (1)
    {
        /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
        vTaskDelay(pdMS_TO_TICKS(10));

        /* Try to take the semaphore, call lvgl related function on success */
        if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
        {

            lv_task_handler();
            xSemaphoreGive(xGuiSemaphore);
        }
    }
}