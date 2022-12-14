/*
 * @Description: lvgl_app
 * @Version: v0.0.1
 * @Autor: tangwc
 * @Date: 2022-09-24 22:49:57
 * @LastEditors: tangwc
 * @LastEditTime: 2022-10-05 20:01:33
 * @FilePath: \esp32_lvgl\main\app_main.c
 *
 *  Copyright (c) 2022 by tangwc, All Rights Reserved.
 */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "lvgl/lvgl.h"
#include "lvgl_helpers.h"
#include "lv_examples/src/lv_demo_widgets/lv_demo_widgets.h"

#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "driver/gpio.h"
#include "nvs_flash.h"

#define TAG "lvgl_demo"
#define LV_TICK_PERIOD_MS 1

/*用定时器给LVGL提供时钟*/
static void lv_tick_task(void *arg)
{
    (void)arg;
    lv_tick_inc(LV_TICK_PERIOD_MS);
}

SemaphoreHandle_t xGuiSemaphore;

static void gui_task(void *pvParameter)
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

    lv_demo_widgets();

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

void app_main(void)
{

	// 初始化nvs用于存放wifi或者其他需要掉电保存的东西
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);
    
	/*创建lvgl任务显示*/
	xTaskCreatePinnedToCore(gui_task, "gui task", 4096*2, NULL, 0, NULL, 1);
}
