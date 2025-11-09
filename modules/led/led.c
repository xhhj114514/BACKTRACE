#include "bsp_gpio.h"
#include "led.h"
#include "stdlib.h"
#include "main.h"
#include "string.h"
#include "bsp_tim.h"
#include "bsp_pwm.h"

// 添加全局LED实例指针
static LedInstance *led = NULL;


void BreathLedUpdate(LedInstance *_instance)
{
    if(_instance == NULL || _instance->breath_led_pwm == NULL) return;
    
    if(_instance->breath_led.breath_direction) 
    {
        // 渐亮
        _instance->breath_led.breath_duty += _instance->breath_led.breath_step;
        if(_instance->breath_led.breath_duty >= 1) 
        {
            _instance->breath_led.breath_duty = 1;
            _instance->breath_led.breath_direction = 0;
        }
    }
    else 
    {
        // 渐暗
        _instance->breath_led.breath_duty -= _instance->breath_led.breath_step;
        if(_instance->breath_led.breath_duty <= 0) 
        {
            _instance->breath_led.breath_duty = 0;
            _instance->breath_led.breath_direction = 1;
        }
    }
    // 设置PWM占空比
    PWMSetDutyRatio(_instance->breath_led_pwm, _instance->breath_led.breath_duty);
}

static void LedToggle(TIMInstance *tim)
{
    if( led->led_tim== NULL) return;
    
    // LED0: 0.25s闪烁 (25次 × 10ms = 250ms)
    //将原本的数字改成led->led_period[0]
    if(++led->led_tim_counters[0] >= led->led_period[0]) 
    {
        led->led_tim_counters[0] = 0;
        if(led->type_mode & LED_B) 
        {
            GPIOToggle(led->led_gpio[0]);
        }
        else
        {
            GPIOSet(led->led_gpio[0]);            
        }
 
    }
    if(++led->led_tim_counters[1] >= led->led_period[1]) 
    {
        led->led_tim_counters[1] = 0;
        if(led->type_mode & LED_G) 
        {
            GPIOToggle(led->led_gpio[1]);
        }
        else
        {
            GPIOSet(led->led_gpio[1]);            
        }
    }
    if(++led->led_tim_counters[2] >= led->led_period[2]) 
    {
        led->led_tim_counters[2] = 0;
        if(led->type_mode & LED_R) 
        {
            GPIOToggle(led->led_gpio[2]);
        }
        else
        {
            GPIOSet(led->led_gpio[2]);            
        }
    }
    // 呼吸灯更新
    if(led->type_mode & LED_M)
    {
        if(led->work_mode&LED_BREATH)
        {
            if(++led->breath_led.breath_counter >= led->breath_led.breath_interval) 
            {
                led->breath_led.breath_counter = 0;
                BreathLedUpdate(led);
            }
        }
    }
}

// led的初始化GPIO配置
static Led_Init_Config_s led_config={
    .led_gpio_init_config[0]={
        .GPIO_Pin=GPIO_PIN_5,
        .GPIOx=GPIOA,
    },
    .led_gpio_init_config[1]={
        .GPIO_Pin=GPIO_PIN_6,
        .GPIOx=GPIOA,
    },
    .led_gpio_init_config[2]={
        .GPIO_Pin=GPIO_PIN_7,
        .GPIOx=GPIOA,
    },
    .led_gpio_init_config[3]= {
        .GPIO_Pin = GPIO_PIN_0,
        .GPIOx = GPIOB,
    },
    .led_tim_init_config={
        .htim=&htim3,
        .callback=LedToggle,
    },
    .breath_led_pwm_init_config = {
        .htim = &htim3,
        .channel = TIM_CHANNEL_3,
    },
};

//为了更好的观察各个灯的gpio变化。
static GPIOInstance *ledb_gpio;
static GPIOInstance *ledg_gpio;
static GPIOInstance *ledr_gpio;
static GPIOInstance *ledm_gpio;

// Led注册函数
LedInstance *LedRegister(Led_Init_Config_s *config)
{
    LedInstance *instance = (LedInstance *)malloc(sizeof(LedInstance));
    memset(instance, 0, sizeof(LedInstance));
    instance->led_gpio[0] = GPIORegister(&led_config.led_gpio_init_config[0]);
    instance->led_gpio[1] = GPIORegister(&led_config.led_gpio_init_config[1]);
    instance->led_gpio[2] = GPIORegister(&led_config.led_gpio_init_config[2]);

    instance->led_tim  = TIMRegister(&led_config.led_tim_init_config);
    instance->led_period[0]=config->led_period[0];
    instance->led_period[1]=config->led_period[1];
    instance->led_period[2]=config->led_period[2];

    instance->breath_led_pwm = PWMRegister(&led_config.breath_led_pwm_init_config);
    instance->breath_led = config->breath_led_config;

    instance->work_mode = config->work_mode;
    instance->type_mode = config->type_mode;

    led = instance;
    return instance;
}

//简单封装 C语言作业玩的差不多了，不讲
void LedSetType(LedInstance *_instance,Led_Type_Mode_e type_mode) 
{ 
    _instance->type_mode = type_mode;
}

void LedSetMode(LedInstance *_instance, Led_Work_Mode_e state)
{
    _instance->work_mode = state;
}

static void LedDark(LedInstance *_instance)
{ 
    for(int i=0;i<3;i++)
    {
        GPIOSet(_instance->led_gpio[i]);
    }

    if(!(_instance->work_mode & LED_BREATH))
    {
        // 关闭呼吸灯PWM
        if(_instance->breath_led_pwm) {
            PWMSetDutyRatio(_instance->breath_led_pwm, 1);
        }
    }
}   

static void LedLight(LedInstance *_instance)
{
    if(_instance == NULL) return;

    if(_instance->type_mode & LED_B)
    {
        GPIOReset(_instance->led_gpio[0]);
    }
    else 
    {
        GPIOSet(_instance->led_gpio[0]);
    }
    if(_instance->type_mode & LED_G)
    {
        GPIOReset(_instance->led_gpio[1]);
    }
    else 
    {
        GPIOSet(_instance->led_gpio[1]);
    }
    if(_instance->type_mode & LED_R)
    {
        GPIOReset(_instance->led_gpio[2]);
    }
    else 
    {
        GPIOSet(_instance->led_gpio[2]);
    }

    if(!(_instance->work_mode & LED_BREATH))
    {
        if(_instance->type_mode & LED_M)
        {
            PWMSetDutyRatio(_instance->breath_led_pwm, 0.0f);
        }
        else
        {
            PWMSetDutyRatio(_instance->breath_led_pwm, 1.0f);
        }
    }
}
static void LedFlow(LedInstance *_instance)
{
    //获取当前时间
    uint32_t current_time = HAL_GetTick();
    //保证最后一盏灯的下一盏灯是第一盏灯
    _instance->flow_led.current_led %= Led_NUM;
    // 检查是否达到切换时间间隔（300ms）
    if(current_time - _instance->flow_led.last_update_time >= 300) 
    {
        // 更新时间戳
        _instance->flow_led.last_update_time = current_time;
        // 关闭所有Led
        LedDark(_instance);
        // 点亮当前Led 
        GPIOReset(_instance->led_gpio[_instance->flow_led.current_led]);
        // 下一盏灯预备
        _instance->flow_led.current_led++;
    }
}

void LedTask(LedInstance *_instance)
{
    switch (_instance->work_mode& ~ LED_BREATH) 
    {
        case LED_LIGHT:
            LedLight(_instance);
            _instance->led_tim->callback=NULL;
            break;
        case LED_DARK:
            LedDark(_instance);
            _instance->led_tim->callback=NULL;
            break;
        case LED_FLOW:
            LedFlow(_instance);
            _instance->led_tim->callback=NULL;
            break;
        case LED_BLANK:
            _instance->led_tim->callback=LedToggle;
            break;
        default:
            break;
    }

    if(_instance->work_mode & LED_BREATH)
    {
        _instance->led_tim->callback = LedToggle;
    }

  
}
void SetLedPeriod(uint16_t ledb_period,uint16_t ledg_period,uint16_t ledr_period)
{
    led->led_period[0]=ledb_period;
    led->led_period[1]=ledg_period;
    led->led_period[2]=ledr_period;
}