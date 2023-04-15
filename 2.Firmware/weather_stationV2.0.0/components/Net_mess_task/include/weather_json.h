#ifndef __WEATHER_JSON_H__
#define __WEATHER_JSON_H__





/**
 * @brief 心知天气（seniverse） 数据结构体
 */
typedef struct
{
	char name[30];//地点
    char text[20];//天气
    char code[10];//城市代码
    char temperature[10];//温度
    char wind_direction[10];//风向
    char wind_scale[10];//风力等级
	char humidity[10];//相对湿度
    char high[10]; //最高气温
    char low[10]; //最低气温
} user_seniverse_config_t;

extern user_seniverse_config_t user_sen_config;

/**
 * @description: 解析daily api接口 心知天气json数据
 * @param {char} *json_data
 * @return {*}
 */
void cjson_to_struct_daily_info(char *json_data);

/**
 * @description: 解析 now api接口 心知天气json数据
 * @param {char} *json_data
 * @return {*}
 */
void cjson_to_struct_now_info(char *json_data);



#endif
