#ifndef __WS2812_H
#define __WS2812_H
#include <stdint.h>
#include "sdkconfig.h"

#define NUM_LEDS 24 //控制灯的数量

//此结构用于指示每个LED的颜色应设置什么。
//每个LED都有一个32位的值。仅使用较低的3个字节即后24位，它们保存
//红色（字节2）、绿色（字节1）和蓝色（字节0）值。
typedef union
{

  struct __attribute__((packed))
  {
    uint8_t r, g, b;
  };
  uint32_t rgb;
} rgb_color;

//设置一个rgb颜色
rgb_color set_LedRGB(uint8_t r, uint8_t g, uint8_t b);


void ws2812_control_init(void);

//将需要的颜色写入
void ws2812_write_leds(rgb_color *led_color);
//测试三原色
void ws2812_test_rgb(void *pvParameters);
//渐变色变化
void rainbow(void *pvParameters);

#endif