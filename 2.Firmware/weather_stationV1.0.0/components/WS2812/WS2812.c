#include <stdio.h>
#include "WS2812.h"
#include "driver/rmt.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LED_RMT_TX_CHANNEL RMT_CHANNEL_0 //RMT使用的通道
#define LED_RMT_TX_GPIO GPIO_NUM_4      //设置RMT输出引脚

#define BITS_PER_LED_CMD 24 //一个灯需要写入24位

//设置数据存放的缓存区，一般为 灯数量*一个灯的字节
#define LED_BUFFER_ITEMS ((NUM_LEDS * BITS_PER_LED_CMD))
rmt_item32_t led_data_buffer[LED_BUFFER_ITEMS];

//这些值在不同灯的情况下有些许不同。
#define T0H 14 // 0 bit high time
#define T1H 28 // 1 bit high time
#define T0L 32 // low time for either bit
#define T1L 24

//此处rgb不一定代表红绿蓝，根据不同情况进行调整，蓝，红 ，绿
rgb_color set_LedRGB(uint8_t r, uint8_t g, uint8_t b)
{
  rgb_color v;

  v.r = r;
  v.g = g;
  v.b = b;
  return v;
}

void ws2812_control_init(void)
{
  rmt_config_t config = RMT_DEFAULT_CONFIG_TX(LED_RMT_TX_GPIO, LED_RMT_TX_CHANNEL); //配置驱动程序

  //将计数器时钟设置为40MHz
  config.clk_div = 2;

  ESP_ERROR_CHECK(rmt_config(&config));
  ESP_ERROR_CHECK(rmt_driver_install(config.channel, 0, 0));
}

void setup_rmt_data_buffer(rgb_color *led_color)
{
  for (uint32_t led = 0; led < NUM_LEDS; led++)
  {
    uint32_t bits_to_send = led_color[led].rgb;
    uint32_t mask = 1 << (BITS_PER_LED_CMD - 1);
    for (uint32_t bit = 0; bit < BITS_PER_LED_CMD; bit++)
    {
      uint32_t bit_is_set = bits_to_send & mask;
      led_data_buffer[led * BITS_PER_LED_CMD + bit] = bit_is_set ? (rmt_item32_t){{{T1H, 1, T0L, 0}}} : (rmt_item32_t){{{T0H, 1, T1L, 0}}};
      mask >>= 1;
    }
  }
}

void ws2812_write_leds(rgb_color *led_color)
{
  setup_rmt_data_buffer(led_color);
  ESP_ERROR_CHECK(rmt_write_items(LED_RMT_TX_CHANNEL, led_data_buffer, LED_BUFFER_ITEMS, false));
  ESP_ERROR_CHECK(rmt_wait_tx_done(LED_RMT_TX_CHANNEL, portMAX_DELAY));
}

//测试rgb三原色
void ws2812_test_rgb(void *pvParameters)
{
  rgb_color red = set_LedRGB(0, 0xff, 0);
  rgb_color blue = set_LedRGB(0xff, 0, 0);
  rgb_color green = set_LedRGB(0, 0, 0xff);
  rgb_color *pixels;

  pixels = malloc(sizeof(rgb_color) * NUM_LEDS);
  printf("rgb test begin......\n");
  while (1)
  {
    for (uint8_t i = 0; i < NUM_LEDS; i++)
    {
      pixels[i] = red;
    }
    ws2812_write_leds(pixels);
    printf("rgb red test......\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
    for (uint8_t i = 0; i < NUM_LEDS; i++)
    {
      pixels[i] = green;
    }
    ws2812_write_leds(pixels);
    printf("rgb green test......\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
   
    for (uint8_t i = 0; i < NUM_LEDS; i++)
    {
      pixels[i] = blue;
    }
    ws2812_write_leds(pixels);
    printf("rgb blue test......\n");
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    
  }
}
//渐变色变化 //此程序渐变色时一定要多个rgb灯才能实现
void rainbow(void *pvParameters)
{
  const uint8_t anim_step = 10;
  const uint8_t anim_max = 240;

  rgb_color color = set_LedRGB(anim_max, 0, 0);
  uint8_t step = 0;
  rgb_color color2 = set_LedRGB(anim_max, 0, 0);
  uint8_t step2 = 0;
  rgb_color *pixels;

  pixels = malloc(sizeof(rgb_color) * NUM_LEDS);

  while (1)
  {
    color = color2;
    step = step2;

    for (uint8_t i = 0; i < NUM_LEDS; i++)
    {
      pixels[i] = color;

      if (i == 1)
      {
        color2 = color;
        step2 = step;
      }

      switch (step)
      {
      case 0:
        color.g += anim_step;
        if (color.g >= anim_max)
          step++;
        break;
      case 1:
        color.r -= anim_step;
        if (color.r == 0)
          step++;
        break;
      case 2:
        color.b += anim_step;
        if (color.b >= anim_max)
          step++;
        break;
      case 3:
        color.g -= anim_step;
        if (color.g == 0)
          step++;
        break;
      case 4:
        color.r += anim_step;
        if (color.r >= anim_max)
          step++;
        break;
      case 5:
        color.b -= anim_step;
        if (color.b == 0)
          step = 0;
        break;
      }
    }
    ws2812_write_leds(pixels);
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}