#include "bsp_gpio.h"
#include <stddef.h>
#include <string.h>
#include "stdlib.h"
#include "gpio.h"

static uint8_t idx;
static GPIOInstance *gpio_instance[GPIO_MX_DEVICE_NUM];
GPIOInstance *GPIORegister(GPIO_Init_Config_s *GPIO_config)
{
    //给GPIO实例分配内存,并使其初始化为0
    GPIOInstance *instance = (GPIOInstance *)malloc(sizeof(GPIOInstance));
    memset(instance, 0, sizeof(GPIOInstance));
    
    //将GPIO的端口、引脚、电平进行初始化配置
    instance->GPIOx = GPIO_config->GPIOx;
    instance->GPIO_Pin = GPIO_config->GPIO_Pin;
    instance->pin_state = GPIO_config->pin_state;
    instance->exti_mode =GPIO_config->exti_mode;
    gpio_instance[idx++] = instance;
    //返回给模块实例中的GPIO实例
    return instance;
}
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    GPIOInstance *gpio;
    for(size_t i=0;i<idx;i++)
    {
        gpio=gpio_instance[i];
        if(gpio->GPIO_Pin==GPIO_Pin&&gpio->gpio_callback!=NULL)
        {
            gpio->gpio_callback(gpio);
        }
    } 
}
//简单封装 C语言作业玩的差不多了，不讲
void GPIOToggle(GPIOInstance *_instance)
{
    HAL_GPIO_TogglePin(_instance->GPIOx, _instance->GPIO_Pin);
}

void GPIOSet(GPIOInstance *_instance)
{
    HAL_GPIO_WritePin(_instance->GPIOx, _instance->GPIO_Pin, GPIO_PIN_SET);
}

void GPIOReset(GPIOInstance *_instance)
{
    HAL_GPIO_WritePin(_instance->GPIOx, _instance->GPIO_Pin, GPIO_PIN_RESET);
}

GPIO_PinState GPIO_ReadPin(GPIOInstance *_instance)
{
    return HAL_GPIO_ReadPin(_instance->GPIOx, _instance->GPIO_Pin);
}
