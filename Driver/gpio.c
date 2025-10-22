#include "gpio.h"

//////////////////////////////////////////////////////////////////////////////////	 
//按键，继电器，蜂鸣器的GPIO设置								  
////////////////////////////////////////////////////////////////////////////////// 	   

void BEEP_AND_RELAY_GPIO_Init(void)
{

 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB|RCC_APB2Periph_GPIOC, ENABLE);	 //使能PB端口时钟

 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;				 // 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);			
 GPIO_ResetBits(GPIOB,GPIO_Pin_12);	//输出低电平
	
	 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;				 // 端口配置
	 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	 GPIO_Init(GPIOC, &GPIO_InitStructure);			
	 GPIO_ResetBits(GPIOC,GPIO_Pin_13);	//输出低电平
}

void KEY_GPIO_Init(void)
{
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO |RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB , ENABLE); 
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE);  //关闭JTAG模式 使A15,PB3，PB4变成普通IO口
}

 void PA15_AND_PB3_5OUT_AND_PB6_9IN(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 	
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);	 //使能PA,PB端口时钟
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;				 // 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOC 
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;				 // 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOC 
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;				 //端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //下拉输入
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB

}


 void PA15_AND_PB3_5IN_AND_PB6_9OUT(void)
{
 
 GPIO_InitTypeDef  GPIO_InitStructure;
 RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);	
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;				 // 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOA, &GPIO_InitStructure);					 //根据设定参数初始化GPIOC 
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;				 // 端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD; 		 //推挽输出
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOC 
	
 GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;				 //端口配置
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //下拉输入
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
 GPIO_Init(GPIOB, &GPIO_InitStructure);					 //根据设定参数初始化GPIOB

}

void KEY_DELAY(void)
{
    u16 i = 10000;
	  while(i--);
}

u8 KEYSCAN(void)    //参考百度单片机矩阵键盘
{ 
	   u8 temp = 0,num = 99;
	   PA15_AND_PB3_5OUT_AND_PB6_9IN(); //A15,B3,B4,B5为输出,B6,B7,B8,B9为输入
     GPIO_WriteBit(GPIOA,GPIO_Pin_15,1);	//A15输出高电平
	   GPIO_Write(GPIOB,(GPIO_ReadOutputData(GPIOB)|0X0038));     // B3,B4,B5输出高电平 
		 KEY_DELAY();//去抖动 
		 if((GPIO_ReadInputData(GPIOB) & 0X03C0 )!= 0) // B6,B7,B8,B9检测到高电平，说明有按键按下      
		 {                                                                                            
			  temp = (u8)((GPIO_ReadInputData(GPIOB) & 0X03C0) >> 2);  //   temp: XXXX 0000  temp: 0010 0000
			  PA15_AND_PB3_5IN_AND_PB6_9OUT(); //A15,B3,B4,B5为输入,B6,B7,B8,B9为输出
			  GPIO_Write(GPIOB,(GPIO_ReadOutputData(GPIOB)|0X03C0));//B6,B7,B8,B9输出高电平  
			  KEY_DELAY();//去抖动 
			  if((GPIO_ReadInputData(GPIOB) & 0X0038) != 0 || GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15))//A15,B3,B4,B5检测到高电平，说明有按键按下 
		    {
					 temp += (u8)((GPIO_ReadInputData(GPIOB) & 0X0038) >> 2) + GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15);// temp: XXXX XXXX  
					 //return temp;                                                                                    //  temp: 0000 0001  
					                                                                                                     
					 while((GPIO_ReadInputData(GPIOB) & 0X0038) != 0 || GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_15));
					 
					 switch(temp)// temp数据对应引脚：B9,B8,B7,B6,B5,B4,B3,A15
					 {
						  case 33: num = 15; break;
						  case 36: num = 6; break;
						  case 65: num = 0; break;
						  case 66: num = 2; break;
						  case 68: num = 5; break;
						  case 24: num = 10; break;
						  case 20: num = 11; break;
						 	case 132: num = 4; break;
						  case 130: num = 1; break;
						  case 18: num = 12; break;
						  case 34: num = 3; break;
						  case 17: num = 13; break;     
						  case 40: num = 9; break;
						 	case 72: num = 8; break;
						  case 136: num = 7; break;    
						 	case 129: num = 14; break;
						  default : break;
           }
					
				}	
           				
     }
		   return num;	
	}



