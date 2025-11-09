#ifndef LED_H
#define LED_H

#include "bsp_gpio.h"
#include "bsp_tim.h"
#include <stdint.h>
#include "bsp_pwm.h"

// led灯数
#define  Led_NUM 4

// led工作模式
typedef enum
{
    LED_DARK,
    LED_LIGHT,
    LED_FLOW,
    LED_ALL_LIGHT,
    LED_BLANK,
    LED_BREATH=0b0100,
}Led_Work_Mode_e;

typedef struct
{
    uint32_t last_update_time;
    uint8_t current_led;
}Flow_Led_s;

typedef struct
{

    uint8_t breath_direction;  //渐暗的方向、渐亮的方向
    float breath_duty;         //亮度
    float breath_step;         //每次变化的亮度
    uint16_t breath_interval;  //10ms*breath_interval变化一次
    uint32_t breath_counter;   //记录次数，到次数变化一次
}Breath_Led_s;

// 需要工作的led灯
// 枚举体这样子设计的原因是我可以通过位与/位或运算对需要工作的led灯进行筛选。
// 如：我可以设置成 LED_B|LED_G|LED_R ,意味着三盏灯都为需要工作的led灯
// _instance->type_mode & LED_B 与运算后判断是否为1，来判断是否有使用相应的灯。
typedef enum
{
    LED_NONE=0b0000,
    LED_B=0b0001,
    LED_G=0b0010,   
    LED_R=0b0100,
    LED_M=0b1000,
}Led_Type_Mode_e;

/* Led实例结构体定义 */
typedef struct
{
    // 具体要亮的led灯 及 工作模式
    Led_Work_Mode_e work_mode;
    Led_Type_Mode_e type_mode;
    //led 的 GPIO实例
    GPIOInstance *led_gpio[Led_NUM];
    // 流水灯结构体
    Flow_Led_s flow_led;
    TIMInstance  *led_tim;
    GPIO_Init_Config_s led_gpio_init_config[Led_NUM];
    uint32_t led_tim_counters[Led_NUM];
    uint16_t led_period[Led_NUM];
    PWMInstance  *breath_led_pwm;
    Breath_Led_s breath_led;
} LedInstance;

/* Led初始化配置 */
typedef struct
{
    // 需要工作的led灯 及 工作模式
    Led_Work_Mode_e work_mode;
    Led_Type_Mode_e type_mode;
    // led的初始化GPIO配置
    GPIO_Init_Config_s led_gpio_init_config[Led_NUM];
    TIM_Init_Config_s led_tim_init_config;
    uint16_t led_period[Led_NUM];
    PWM_Init_Config_s breath_led_pwm_init_config;
    Breath_Led_s breath_led_config;
} Led_Init_Config_s;

//注册
LedInstance *LedRegister(Led_Init_Config_s *config);
//设置需要工作的led灯
void LedSetType(LedInstance *_instance,Led_Type_Mode_e type_mode);
//设置led工作模式
void LedSetMode(LedInstance *_instance, Led_Work_Mode_e state);
//led做的事情 
void LedTask(LedInstance *_instance);

void SetLedPeriod(uint16_t ledb_period,uint16_t ledg_period,uint16_t ledr_period);
void BreathLedUpdate(LedInstance *_instance);
#endif // !LED_H