#include "timer.h"
#include "delay.h"
#include "sys.h"
#include "gpio.h"  

//通用定时器中断初始化
//这里时钟选择为APB1的2倍，而APB1为36M
//arr：自动重装值。
//psc：时钟预分频数
//这里使用的是定时器2
void TIM2_Init(u16 arr,u16 psc)
{	 
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//使能TIM2时钟
	
	//初始化定时器2	 
	TIM_TimeBaseStructure.TIM_Period = arr; //设定计数器自动重装值 
	TIM_TimeBaseStructure.TIM_Prescaler =psc; 	//预分频器   
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位
  

	
	//中断分组初始化
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;  //TIM2中断
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;  //先占优先级1级
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;  //从优先级0级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE; //IRQ通道被使能
	NVIC_Init(&NVIC_InitStructure);  //根据NVIC_InitStruct中指定的参数初始化外设NVIC寄存器 
	
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);//允许更新中断 ,
	
  TIM_Cmd(TIM2,ENABLE ); 	//使能定时器2
}

extern u8 RELAY_TIME; 
extern u8 InitDisplay; 
extern u8 pass;
extern u8 ICpass;
extern u8 ReInputEn;

void TIM2_IRQHandler(void)//定时器2中断服务程序	 
{ 
	  static u8 time_count = 0;
	
		if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) //检查指定的TIM中断发生与否:TIM 中断源 
		{ 
			    if(time_count ++ >= 100)//1miao
					{
					      time_count = 0;
						  
                if(RELAY_TIME )  RELAY_TIME--;//继电器开启时间倒计时
						    else//继电器开启时间计时到0
								{
										if(pass==1 || ICpass==1)
										{
											  if(ReInputEn==0)
												{
													pass = 0;ICpass=0;
													InitDisplay = 1;//返回主页面标志置1
												}
										}
								}
					}
					TIM_ClearITPendingBit(TIM2, TIM_IT_Update); //清除中断标志位
	  }
}

