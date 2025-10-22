#include "lcd1602.h"
#include "delay.h"
#include "sys.h"

u8 Dao_xu(u8 data)//倒叙函数
{
    u8 i = 0 ,temp = 0;;
	  
	  for(i = 0; i < 8; i++)
	 {
	   temp += (((data >> i) & 0x01) << (7 - i));
	 }
	 return temp;
}


/*------------------------------------------------
              写入命令函数
------------------------------------------------*/
 void LCD_Write_Com(unsigned char com) 
{                              
  LCD1602_RS=0;	 //命令
	delay_us(1);
  LCD1602_RW=0;
  delay_us(1);
  LCD1602_EN=1;                  
	delay_us(1);
  GPIO_Write(GPIOA,(GPIO_ReadOutputData(GPIOA)&0XFF00)+Dao_xu(com)); //在不影响A8-A15引脚的前提下，把数据写到A0-A7引脚     
	delay_us(100);
  LCD1602_EN=0;
}

/*------------------------------------------------
              写入数据函数
------------------------------------------------*/
 void LCD_Write_Data(unsigned char Data) 
{                                  
  LCD1602_RS=1;   //数据              
	delay_us(1);
  LCD1602_RW=0;
  delay_us(1);	
  LCD1602_EN=1;                  
	delay_us(1);
  GPIO_Write(GPIOA,(GPIO_ReadOutputData(GPIOA)&0XFF00)+Dao_xu(Data)); //在不影响A8-A15引脚的前提下，把数据写到A0-A7引脚   
	delay_us(100);
  LCD1602_EN=0;
}
/*------------------------------------------------
              写入字符串函数
------------------------------------------------*/
 void LCD_Write_String(unsigned char x,unsigned char y,unsigned char *s) 
 {     
 if (y == 0) 
 	{     
	 LCD_Write_Com(0x80 + x);  //第一行   
 	}
 else 
 	{     
 	LCD_Write_Com(0xC0 + x);  //第二行   
 	}        
	while (*s) //判断是否检测到结尾符 
 	{     
		 LCD_Write_Data( *s);//显示字符     
		 s ++;  //指针加1   
 	}
 }
/*------------------------------------------------
              写入字符函数
------------------------------------------------*/
 void LCD_Write_Char(unsigned char x,unsigned char y,unsigned char Data) 
 {     
 if (y == 0) 
 	{     
 	LCD_Write_Com(0x80 + x);    //第一行    
 	}    
 else 
 	{     
 	LCD_Write_Com(0xC0 + x);  //第二行     
 	}        
 LCD_Write_Data( Data); //显示字符      
 }
 

 /*------------------------------------------------
                清屏函数
------------------------------------------------*/
 void LCD_Clear(void) 
 { 
 LCD_Write_Com(0x01); 
 delay_ms(5);
 }

/*------------------------------------------------*/
 void LCD_Init(void) 
{
	  GPIO_InitTypeDef GPIO_InitStructure;
	  
	  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB, ENABLE);  //开启GPIOA GPIOB GPIOC时钟
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |
    GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; //输出速度50MHZ
    GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化GPIOA
	
	
	  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 |GPIO_Pin_1|GPIO_Pin_10;  // LCD1602 RS-RW-EN
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   //推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   //输出速度50MHZ
    GPIO_Init(GPIOB, &GPIO_InitStructure);   //GPIOB
	
   LCD_Write_Com(0x38);
   delay_ms(5);  
	 LCD_Write_Com(0x38);
   delay_ms(5);  
	 LCD_Write_Com(0x38);
   delay_ms(5);  
   LCD_Write_Com(0x08);    /*显示关闭*/
   delay_ms(5); 	
   LCD_Write_Com(0x01);    /*显示清屏*/
   delay_ms(5); 	
   LCD_Write_Com(0x06);    /*显示光标移动设置*/ 
	 delay_ms(5); 
   LCD_Write_Com(0x0C);    /*显示开及光标设置*/
	 delay_ms(5); 
}

