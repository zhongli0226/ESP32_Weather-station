#ifndef  __NETWORK_TASK_H__
#define  __NETWORK_TASK_H__



extern uint32_t user_sen_flag;  //天气api连接成功标志
extern uint32_t user_time_flag;  //时间更新成功标记

void network_task_handler(void* pvParameter);

#endif