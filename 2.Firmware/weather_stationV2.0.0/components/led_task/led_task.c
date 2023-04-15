#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/rmt.h"
#include "led_strip.h"


#include "led_task.h"


static const char* TAG = "LED TASK";

#define LED_RMT_TX_CHANNEL RMT_CHANNEL_0 //选择 rmt 通道
#define LED_RMT_TX_GPIO GPIO_NUM_4      //设置RMT输出引脚
#define STRIP_LED_NUMBER 1              //需要控制的灯的数量

#define EXAMPLE_CHASE_SPEED_MS (10)


/**
 * @brief sin()函数从0到2π的样本值，一共255个点，最大值为255，最小值为0
 *
 * 离散信号函数：SinValue(k)=(255*sin(2*k*π/255)+255)/2 （四舍五入取整数）
 *
 */
const uint32_t SinValue[256] = {
                                128,   131,   134,   137,   140,   143,   147,   150,   153,   156,
                                159,   162,   165,   168,   171,   174,   177,   180,   182,   185,
                                188,   191,   194,   196,   199,   201,   204,   206,   209,   211,
                                214,   216,   218,   220,   223,   225,   227,   229,   230,   232,
                                234,   236,   237,   239,   240,   242,   243,   245,   246,   247,
                                248,   249,   250,   251,   252,   252,   253,   253,   254,   254,
                                255,   255,   255,   255,   255,   255,   255,   255,   255,   254,
                                254,   253,   253,   252,   251,   250,   249,   249,   247,   246,
                                245,   244,   243,   241,   240,   238,   237,   235,   233,   231,
                                229,   228,   226,   224,   221,   219,   217,   215,   212,   210,
                                208,   205,   203,   200,   198,   195,   192,   189,   187,   184,
                                181,   178,   175,   172,   169,   166,   163,   160,   157,   154,
                                151,   148,   145,   142,   139,   136,   132,   129,   126,   123,
                                120,   117,   114,   111,   107,   104,   101,    98,    95,    92,
                                89,    86,    83,    80,    77,    74,    72,    69,    66,    63,
                                61,    58,    55,    53,    50,    48,    45,    43,    41,    38,
                                36,    34,    32,    30,    28,    26,    24,    22,    21,    19,
                                17,    16,    14,    13,    12,    10,     9,     8,     7,     6,
                                5,     4,     4,     3,     2,     2,     1,     1,     1,     0,
                                0,     0,     0,     0,     1,     1,     1,     2,     2,     3,
                                3,     4,     5,     6,     6,     7,     9,    10,    11,    12,
                                14,    15,    17,    18,    20,    21,    23,    25,    27,    29,
                                31,    33,    35,    37,    40,    42,    44,    47,    49,    52,
                                54,    57,    59,    62,    65,    67,    70,    73,    76,    79,
                                82,    85,    88,    91,    94,    97,   100,   103,   106,   109,
                                112,   115,   118,   121,   125,   128
};


static void ws2812_rainbow_change(led_strip_t* strip, uint32_t led_num, uint32_t change_rate)
{
    uint32_t green = 0, red = 0, blue = 0;
    uint8_t ig, ir, ib;//这里使用8位数据,超过255会自动从0位开始,可以方便不需要多余处理
    for (ig = 0;ig < 255;ig++)
    {
        ir = ig + 85;
        ib = ig + 170;// 正弦角度错开120°
        green = SinValue[ig];
        red = SinValue[ir];
        blue = SinValue[ib];
        for (uint32_t i = 0;i < led_num;i++) // 设置ws2812的RGB的值 
            ESP_ERROR_CHECK(strip->set_pixel(strip, i, red, green, blue));


        // 给WS2812发送RGB的值
        ESP_ERROR_CHECK(strip->refresh(strip, 100));
        vTaskDelay(pdMS_TO_TICKS(change_rate));
    }
}

void led_task_handler(void* pvParameter)
{
    (void)pvParameter;

    // rmt 初始化
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(LED_RMT_TX_GPIO, LED_RMT_TX_CHANNEL);
    // set counter clock to 40MHz
    config.clk_div = 2;

    ESP_ERROR_CHECK(rmt_config(&config));
    ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));

    // install ws2812 driver
    led_strip_config_t strip_config = LED_STRIP_DEFAULT_CONFIG(STRIP_LED_NUMBER, (led_strip_dev_t)config.channel);
    led_strip_t* strip = led_strip_new_rmt_ws2812(&strip_config);
    if (!strip) {
        ESP_LOGE(TAG, "install WS2812 driver failed");
    }
    // Clear LED strip (turn off all LEDs)
    ESP_ERROR_CHECK(strip->clear(strip, 100));

    // Show simple rainbow chasing pattern
    ESP_LOGI(TAG, "LED Rainbow Chase Start");

    while (1)
        ws2812_rainbow_change(strip, STRIP_LED_NUMBER, 100);
}