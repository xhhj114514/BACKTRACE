#ifndef KEY_H
#define KEY_H

#include "bsp_gpio.h"
#include <stdint.h>

/* 按键事件类型枚举 */
typedef enum {
    KEY_NO_PRESS = 0,      // 无按键按下
    KEY_SHORT_PRESS,       // 短按
    KEY_LONG_PRESS         // 长按
} Key_Event_e;

/* FIFO队列元素结构体 */
typedef struct {
    Key_Event_e event;     // 按键事件类型
    uint32_t timestamp;    // 时间戳
} KeyEvent_FIFO_Item_s;

/* 按键FIFO队列配置 */
#define KEY_FIFO_SIZE 16   // FIFO队列大小

/* Led实例结构体定义 */
typedef struct
{
    // 按键的GPIO实例
    GPIOInstance *key_gpio;
    // 按键被按下为 1，未被按下为 0 
    uint8_t key_pressed;
    //按下按键的时间戳
    uint16_t key_press_time;   
    // 长按相关属性
    uint32_t key_long_press_start_time; // 长按开始时间
    uint32_t key_release_time;
    uint8_t long_press_detected;  // 已检测到长按事件
    
    // 新增FIFO队列相关字段
    KeyEvent_FIFO_Item_s key_event_fifo[KEY_FIFO_SIZE]; // FIFO队列
    uint8_t fifo_head;             // 队列头部索引
    uint8_t fifo_tail;             // 队列尾部索引
    uint8_t fifo_count;            // 队列中元素个数
    
    // 扩展字段：记录短按和长按次数
    uint16_t short_press_count;    // 短按次数
    uint16_t long_press_count;     // 长按次数
} KeyInstance;

/* Led初始化配置 */
typedef struct
{
    // 按键的GPIO初始化配置
    GPIO_Init_Config_s key_init_config;
    // 长按判定时间(毫秒)，默认1000ms
    uint16_t long_press_time_threshold;
} Key_Init_Config_s;

KeyInstance *KeyRegister(void);
void KeyTask(KeyInstance *_instance);
Key_Event_e KeyGetEvent(KeyInstance *_instance); // 获取按键事件
void KeyClearEvent(KeyInstance *_instance);      // 清空按键事件队列

#endif // !KEY_H