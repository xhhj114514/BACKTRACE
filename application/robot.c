#include "robot.h"
#include "led.h"
#include "key.h"

//定义led实例
static LedInstance *led;
//定义key实例
static KeyInstance *key;

// 长按处理函数
void RobotInit()
{
    //为led设置工作模式及需要工作的灯
    Led_Init_Config_s led_config={
        .type_mode=LED_B|LED_G|LED_R|LED_M,
        .work_mode=LED_DARK,
        .led_period[0]=25,
        .led_period[1]=50,
        .led_period[2]=100,

    };

    //注册led实例
    led=LedRegister(&led_config);
    //注册key实例
    key=KeyRegister();
    
}

void RobotTask()
{
    Key_Event_e event;
    key->short_press_count%=4;
    // while((event = KeyGetEvent(key))){
      switch(event){
        case KEY_SHORT_PRESS:
            if(key->short_press_count==0)
            {
                LedSetMode(led, LED_DARK);
            }
            else if(key->short_press_count==1)
            {
                LedSetMode(led,LED_LIGHT);
            }
            else if(key->short_press_count==2)
            {
                LedSetMode(led, LED_FLOW);
            }
            else if (key->short_press_count==3)
            {
                LedSetMode(led, LED_BREATH);
            }
              break;
            
        case KEY_LONG_PRESS:
              LedSetMode(led,LED_BLANK);
              break;
        default:
              break;
      }

    // }
    LedTask(led);
    // KeyTask(key);
}
    void stack_overflow_test(void)
{
    volatile char buffer[100]; // 局部变量占用栈空间
    // 初始化数组以确保编译器不会优化掉
    for(int i = 0; i < 100; i++) {
        buffer[i] = i & 0xFF;
    }
    
    // 递归调用自身，无终止条件，导致无限递归
    stack_overflow_test();
}
