#ifndef __GPIO_H
#define __GPIO_H	 
#include "sys.h"

#define BEEP  PCout(13)	
#define RELAY PBout(12)	

void BEEP_AND_RELAY_GPIO_Init(void);//继电器初始化
u8 KEYSCAN(void);    //参考百度单片机矩阵键盘
void PA15_AND_PB3_5IN_AND_PB6_9OUT(void);
void PA15_AND_PB3_5OUT_AND_PB6_9IN(void);
void KEY_GPIO_Init(void);
	 				    
#endif
