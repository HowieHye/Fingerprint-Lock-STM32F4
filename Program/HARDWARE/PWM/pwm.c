#include "pwm.h"

// TIM2 PWM部分初始化
// PWM输出初始化
// arr：自动重装值
// psc：时钟预分频数

void TIM2_PWM_Init(u32 arr, u32 psc) {
  //此部分需手动修改IO口设置

  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
  TIM_OCInitTypeDef TIM_OCInitStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  // TIM2时钟使能
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE); //使能PORTA时钟

  GPIO_PinAFConfig(GPIOC, GPIO_PinSource8, GPIO_AF_TIM3);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;          // GPIOA0
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;       //复用功能
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz; //速度100MHz
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;     //推挽复用输出
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;       //上拉
  GPIO_Init(GPIOC, &GPIO_InitStructure);             //初始化PA0

  TIM_TimeBaseStructure.TIM_Prescaler = psc;                  //定时器分频
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; //向上计数模式
  TIM_TimeBaseStructure.TIM_Period = arr; //自动重装载值
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;

  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //初始化定时器2

  //初始化TIM2 Channel1 PWM模式
  TIM_OCInitStructure.TIM_OCMode =
      TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式2
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
  TIM_OCInitStructure.TIM_OCPolarity =
      TIM_OCPolarity_Low;            //输出极性:TIM输出比较极性低
  TIM_OCInitStructure.TIM_Pulse = 0; //比较初始值
  TIM_OC3Init(TIM3, &TIM_OCInitStructure); //根据T指定的参数初始化外设TIM2OC1

  TIM_OC3PreloadConfig(TIM3,
                       TIM_OCPreload_Enable); //使能TIM2在CCR1上的预装载寄存器

  TIM_ARRPreloadConfig(TIM3, ENABLE); // ARPE使能

  TIM_Cmd(TIM3, ENABLE);
}

void PWM_Init(void) {
  TIM2_PWM_Init(
      200 - 1,
      8400 -
          1); // 84M/84=1Mhz计数频率,重装载值1000，所以PWM频率为 1M/1000=1Khz.
}
