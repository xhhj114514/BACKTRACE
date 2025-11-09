#ifndef BSP_GPIO_H
#define BSP_GPIO_H

#include "gpio.h"
#include "stdint.h"

// GPIO实例数
#define GPIO_MX_DEVICE_NUM 10

/**
 * @brief GPIO实例结构体定义
 */

 typedef enum
{
    GPIO_EXTI_MODE_RISING,
    GPIO_EXTI_MODE_FALLING,
    GPIO_EXTI_MODE_RISING_FALLING,
    GPIO_EXTI_MODE_NONE,
} GPIO_EXTI_MODE_e;

typedef struct gpio
{
    //GPIO的端口、引脚、电平
    GPIO_TypeDef *GPIOx;        // GPIOA,GPIOB,GPIOC...
    GPIO_PinState pin_state;    // 引脚状态,Set,Reset;not frequently used
    uint16_t GPIO_Pin;          // 引脚号,
    //中断模式      
    GPIO_EXTI_MODE_e exti_mode;
    //中断后需要调用的回调函数（在模块层设置）
    void (*gpio_callback)(struct gpio*);
} GPIOInstance;

/**
 * @brief GPIO初始化配置结构体定义
 */
typedef struct
{
    //GPIO端口、引脚、电平的基本配置
    GPIO_TypeDef *GPIOx;        // GPIOA,GPIOB,GPIOC...
    GPIO_PinState pin_state;    // 引脚状态,Set,Reset
    uint16_t GPIO_Pin;          // 引脚号,这里的引脚号是GPIO_PIN_0,GPIO_PIN_1...
    GPIO_EXTI_MODE_e exti_mode;
} GPIO_Init_Config_s;

GPIOInstance *GPIORegister(GPIO_Init_Config_s *GPIO_config);
void GPIOToggle(GPIOInstance *_instance);
void GPIOSet(GPIOInstance *_instance);
void GPIOReset(GPIOInstance *_instance);
GPIO_PinState GPIO_ReadPin(GPIOInstance *_instance);

#endif // !BSP_GPIO_H