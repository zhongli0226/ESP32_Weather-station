#ifndef __WEATHER_JSON_H__
#define __WEATHER_JSON_H__





/**
 * @brief 心知天气（seniverse） 数据结构体
 */
typedef struct
{
	char *name;//地点
    char *text;//天气
    char *code;//城市代码
    char *temperature;//温度
    char *wind_direction;//风向
    char *wind_scale;//风力等级
	char *humidity;//相对湿度
} user_seniverse_config_t;

extern user_seniverse_config_t user_sen_config;
extern uint32_t user_sen_flag;
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
