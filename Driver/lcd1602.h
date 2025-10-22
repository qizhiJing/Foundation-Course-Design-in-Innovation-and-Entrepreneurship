#ifndef __LCD1602_H
#define __LCD1602_H
#include "delay.h"
#include "sys.h"

#define LCD1602_RS PBout(10)	//数据命令
#define LCD1602_RW PBout(1)	//读写
#define LCD1602_EN PBout(0)	//使能
 
 void LCD_Init(void); 
 void LCD_Clear(void); 
 void LCD_Write_Com(unsigned char com); 
 void LCD_Write_Data(unsigned char Data); 
 void LCD_Write_String(unsigned char x,unsigned char y,unsigned char *s);
 void LCD_Write_Char(unsigned char x,unsigned char y,unsigned char Data); 
 
#endif



