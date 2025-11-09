#include "key.h"
#include <stdlib.h>
#include "bsp_gpio.h"
#include "main.h"
#include "string.h"

// 按键的GPIO初始化配置
Key_Init_Config_s config={
    .key_init_config={
        .GPIOx = GPIOA,
        .GPIO_Pin = GPIO_PIN_4,
        .exti_mode = GPIO_EXTI_MODE_FALLING,
    },
    .long_press_time_threshold = 1000, // 默认长按时间为1000ms
};

// 回调函数传入的是GPIOInstance *gpio，为了能在调用函数时改变key实例数据
// 我们定义KeyInstance *key;
static KeyInstance *key;


/*
    编写我们需要用到的回调函数，按键按下时，key->key_pressed =1；
    并记录我们此刻按下按键的时间
*/
static void RecordKeyPress(GPIOInstance *gpio)
{
    if (key != NULL) {
        key->key_pressed = 1;
        key->key_press_time = HAL_GetTick();
        key->key_long_press_start_time = HAL_GetTick(); // 记录长按开始时间
        key->long_press_detected = 0; // 重置长按检测标志
    }
}

// 向FIFO队列中添加按键事件
static void KeyAddEventToFIFO(KeyInstance *_instance, Key_Event_e event)
{
    // 检查队列是否已满
    if (_instance->fifo_count < KEY_FIFO_SIZE) {
        // 添加事件到队列尾部
        _instance->key_event_fifo[_instance->fifo_tail].event = event;
        _instance->key_event_fifo[_instance->fifo_tail].timestamp = HAL_GetTick();
        
        // 更新尾部索引和计数
        _instance->fifo_tail = (_instance->fifo_tail + 1) % KEY_FIFO_SIZE;
        _instance->fifo_count++;
    }
}

// 从FIFO队列中获取按键事件
Key_Event_e KeyGetEvent(KeyInstance *_instance)
{
    Key_Event_e event = KEY_NO_PRESS;
    
    // 检查队列是否为空
    if (_instance->fifo_count > 0) {
        // 从队列头部获取事件
        event = _instance->key_event_fifo[_instance->fifo_head].event;
        
        // 更新头部索引和计数
        _instance->fifo_head = (_instance->fifo_head + 1) % KEY_FIFO_SIZE;
        _instance->fifo_count--;
    }
    
    return event;
}

// 清空按键事件队列
void KeyClearEvent(KeyInstance *_instance)
{
    _instance->fifo_head = 0;
    _instance->fifo_tail = 0;
    _instance->fifo_count = 0;
}

KeyInstance *KeyRegister()
{
    //给Key实例分配内存,并使其初始化为0    
    KeyInstance *instance = (KeyInstance *)malloc(sizeof(KeyInstance));
    memset(instance, 0, sizeof(KeyInstance));

    //将Key的GPIO初始化配置传入到GPIO注册函数，为按键注册GPIO实例。
    instance->key_gpio = GPIORegister(&config.key_init_config);
    instance->key_gpio->gpio_callback = RecordKeyPress;
    key =instance;
    //返回给应用层实例中的key实例
    return instance;
}

void KeyTask(KeyInstance *_instance)
{
    // 处理按键释放逻辑
    if (_instance->key_pressed) {
        // 检查按键是否仍处于按下状态
        if (GPIO_ReadPin(_instance->key_gpio) == GPIO_PIN_RESET) {
            // 按键仍然按下，检查是否达到长按阈值
            uint32_t press_duration = HAL_GetTick() - _instance->key_long_press_start_time;
            if (press_duration >= config.long_press_time_threshold && !_instance->long_press_detected) {
                _instance->long_press_detected = 1;
                // 检测到长按，添加到FIFO队列
                KeyAddEventToFIFO(_instance, KEY_LONG_PRESS);
                _instance->long_press_count++;       // 增加长按计数   
            }
        } else {
            // 按键已经释放
            uint32_t press_duration = HAL_GetTick() - _instance->key_press_time;
            
            // 判断是短按还是长按
            if (press_duration >= 20 && press_duration < config.long_press_time_threshold) {
                // 短按
                KeyAddEventToFIFO(_instance, KEY_SHORT_PRESS);
                _instance->short_press_count++;      // 增加短按计数
            }
            // 清除按键按下标志
            _instance->key_pressed = 0;
            _instance->long_press_detected = 0;
        }
    } else {
        if (_instance->short_press_count == 0&& _instance->long_press_count == 0) {
            KeyAddEventToFIFO(_instance, KEY_NO_PRESS);
        }
    }
}