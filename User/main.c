#include "sys.h"
#include "delay.h"
#include "usart1.h"
#include "lcd1602.h"
#include "gpio.h"
#include "as608.h"
#include "string.h"
#include "timer.h"
#include "stmflash.h"
#include "rc522_config.h"
#include "rc522_function.h"
#include "stm32f10x.h" 

// Device header


#define FLASH_SAVE_ADDR   0X08004096 				//设置FLASH 保存地址(必须为偶数)#define FLASH_SAVE_ADDR   0X0801F000 	

#define MAX_PEOPLE   5 //最大存储5张IC卡
#define SIZE 10

#define uchar unsigned char
#define uint  unsigned int

//定义标识
unsigned char FlagKeyPress = 0;  //有键按下标志，处理完毕清0
unsigned char keycode;    //按键值获取变量
unsigned char operation=0;		//操作标志位
unsigned char pass=0;			//密码正确标志
unsigned char ICpass=0;			//IC正确标志

unsigned char CurrentPassword[6]={0,0,0,0,0,0}; 														//当前密码值
unsigned char InputData[6]={0,0,0,0,0,0};			 //输入密码暂存区
unsigned char initpassword[6]={0,0,0,0,0,0};
unsigned char TempPassword[6];		//重置密码缓存区
unsigned char PressNum=0;			   //密码输入位数记数
unsigned char CorrectCont;			//正确输入计数
unsigned char ReInputEn=0;		 //重置输入充许标志	
unsigned char ReInputCont; 			//重新输入计数
unsigned char ErrorCont;			//错误次数计数

unsigned char RELAY_TIME;	//继电器开启时间
unsigned char InitDisplay=1;//返回主页面标志

unsigned char Register = 0;//注册卡标志
unsigned char Delete = 0;//删除卡标志
unsigned char RegFingerprint = 0;//注册指纹标志

unsigned short user_id = 0;//用户指纹ID

u8 ID_BUF[8],ID_TEMP_Buffer[10]; //ID_TEMP_Buffer注册过的卡号
u8 ucArray_ID [ 4 ] ;//存放IC卡号

unsigned char zc=0;			//注册

//////////////////////////////////////////////////////////////////////////////////	 
//函数声明					  
////////////////////////////////////////////////////////////////////////////////// 
void WRITE_DATA_TO_FLASH(u8* ID_Buffer,u8 LEN);//数据写入STM32内部FLASH
void READ_DATA_FOR_FLASH(u8* ID_TEMP_Buffer ,u8 LEN);//从STM32内部FLASH读出数据
void CHECK_NEW_MCU(void);
u8 RC522_SCAN(u8* BUF);//扫描IC卡
void COMPER_ID_MODE(void);//正常待机刷卡函数
void ADD_ID_MODE(void);//注册IC函数
void DEL_ID_MODE(void);//删除IC卡函数

void DataInit()
{
	  unsigned char i=0;				//密码输入位数记数
	  PressNum=0;				//密码输入位数记数
		for(i=0;i<6;i++)InputData[i]=0;    //密码暂存区清零
}

void BuzzerRingsNum(u8 num)//蜂鸣器函数
{
		unsigned char i=0;
	  for(i=0; i<num; i++)
		{
				BEEP = 1;
			  delay_ms(100);
			  BEEP = 0;
			  if(num!=1)
				delay_ms(50);
		}
}

void display_user_id()//显示指纹ID
{
    LCD_Write_Com(0xc0);
    LCD_Write_Data('U');
    LCD_Write_Data('s');
    LCD_Write_Data('e');
    LCD_Write_Data('r');
    LCD_Write_Data('s');
    LCD_Write_Data(' ');
    LCD_Write_Data('i');
    LCD_Write_Data('d');
	  LCD_Write_Data(user_id+0x30);
    LCD_Write_Data(' ');
    LCD_Write_Data('r');
    LCD_Write_Data('e');
    LCD_Write_Data('g');
    LCD_Write_Data(' ');
	  LCD_Write_Data(' ');
    LCD_Write_Data(' ');
    LCD_Write_Data(' ');
}

//========================重置密码==================================================
//==================================================================================
void ResetPassword(void)
{
	unsigned char i=0;
	
	if(pass==0)//密码错误
	{
			BuzzerRingsNum(3);//蜂鸣器响3声
			LCD_Write_String(0,1,"      error     "); 
			delay_ms(1000);
			LCD_Write_String(0,1,"password:       ");
			LCD_Write_Com(0x80+0x40+9);
			LCD_Write_Com(0x0F);//光标闪烁
		  DataInit();  //将输入数据计数器清零
	}
	else
	{
    	if(ReInputEn==1)
			{
					if(PressNum==6)// 新密码必须为6
					{
							ReInputCont++;				
							if(ReInputCont==2)
							{
                if((TempPassword[0]==InputData[0])&&(TempPassword[1]==InputData[1])&&(TempPassword[2]==InputData[2])&&  
									 (TempPassword[3]==InputData[3])&&(TempPassword[4]==InputData[4])&&(TempPassword[5]==InputData[5]))  //将两次输入的新密码作对比
										{
												LCD_Write_String(0,1,"ResetPasswordOK ");	
												BuzzerRingsNum(2);
												WRITE_DATA_TO_FLASH(TempPassword,6);  				// 将新密码写入STM32内部FLASH
												delay_ms(100);
												READ_DATA_FOR_FLASH(CurrentPassword,6); 			//从STM32内部FLASH里读出存储密码
												delay_ms(1000);
												LCD_Write_String(0,1,"password:       ");
												LCD_Write_Com(0x80+0x40+9);
												LCD_Write_Com(0x0F);//光标闪烁
										}
										else//两次输入的密码不一致
										{
												BuzzerRingsNum(3);//蜂鸣器响3声
												LCD_Write_String(0,1,"      error     "); 
												delay_ms(1000);
												LCD_Write_String(0,1,"password:       ");
												LCD_Write_Com(0x80+0x40+9);
												LCD_Write_Com(0x0F);//光标闪烁
										}
										ReInputEn=0;			//关闭重置功能
										ReInputCont=0;
										CorrectCont=0;
							}
							else
							{
								BuzzerRingsNum(1);
								LCD_Write_String(0,1,"input again     ");		//提示你再次输入	
								for(i=0;i<6;i++)
								{
									TempPassword[i]=InputData[i];		//将第一次输入的数据暂存起来						
								}
							}
							DataInit();  //将输入数据计数器清零
					 }
				}
	}

}

//=======================取消所有操作============================================
void Cancel(void)
{	
		unsigned char i;
	
		LCD_Write_String(0,1,"password:       ");
		BuzzerRingsNum(2);				//提示音,响两声
		for(i=0;i<6;i++)InputData[i]=0;    //密码暂存区清零
		RELAY=0;					//关闭锁
		BEEP =0;					//报警关
		pass=0;					//密码正确标志清零
		ReInputEn=0;			//重置输入充许标志清零
	  ErrorCont=0;			//密码错误输入次数清零
		CorrectCont=0;			//密码正确输入次数清零
		ReInputCont=0;			//重置密码输入次数清零 
		PressNum=0;					//输入位数计数器清零
	  ICpass = 0;
	  InitDisplay=1;
}


//==========================确认键，并通过相应标志位执行相应功能===============================
void Ensure(void)
{
	  if(PressNum == 6)//必须输入6位数密码才有效
		{
			  if((InputData[0]==0)&&(InputData[1]==0)&&(InputData[2]==0)&&(InputData[3]==0)&&(InputData[4]==0)&&(InputData[5]==3))//000003密码初始化为000000
				{
						WRITE_DATA_TO_FLASH(initpassword,6);  				// 强制将初始密码写入STM32内部存储
					  delay_ms(100);
					  READ_DATA_FOR_FLASH(CurrentPassword,6); 			//从STM32内部FLASH里读出存储密码
					  LCD_Write_String(0,1,"Init password...");
					  BEEP = 1;
					  delay_ms(1000);
					  BEEP = 0;
						LCD_Write_String(0,1,"password:       ");
						LCD_Write_Com(0x80+0x40+9);
						LCD_Write_Com(0x0F);//光标闪烁
				}
				else if((InputData[0]==0)&&(InputData[1]==0)&&(InputData[2]==0)&&(InputData[3]==0)&&(InputData[4]==0)&&(InputData[5]==1))//000001注册指纹、注册卡、删除卡
				{
								LCD_Write_String(0,1,"                ");
								LCD_Write_String(0,2,"    manager     ");
								BuzzerRingsNum(2);				//提示音,响两声
								zc=1;
				}
				
				else if((InputData[0]==0)&&(InputData[1]==0)&&(InputData[2]==0)&&(InputData[3]==0)&&(InputData[4]==0)&&(InputData[5]==2))//000002指纹清空
				{
					  if (FINGERPRINT_Cmd_Delete_All_Model() == 0)
						{
								LCD_Write_String(0,1," clear finger ok");
								BuzzerRingsNum(2);				//提示音,响两声
						}
						user_id = 0;
						delay_ms(1000);
						InitDisplay = 1;
				}
				
				else
				{
						if((InputData[0]==CurrentPassword[0])&&(InputData[1]==CurrentPassword[1])&&(InputData[2]==CurrentPassword[2])&&  
							 (InputData[3]==CurrentPassword[3])&&(InputData[4]==CurrentPassword[4])&&(InputData[5]==CurrentPassword[5]))  //密码匹配正确
						{ 
							  CorrectCont++;
							  if(CorrectCont==1)		//正确输入计数，当只有一次正确输入时，开锁，
								{
										ErrorCont=0;
										LCD_Write_String(0,1,"      open      ");  //显示开锁
										RELAY = 1; 					//继电器开启
										RELAY_TIME = 15; 		//继电器开启15秒
										pass = 1;						//密码正确标志
										BuzzerRingsNum(2);				//提示音,响两声
								}
								else              	//当两次正确输入时，开启重置密码功能
								{
										LCD_Write_String(0,1,"SetNewWordEnable");
									  BuzzerRingsNum(2);				//提示音,响两声
									  ReInputEn=1;									//允许重置密码输入
										CorrectCont=0;									//正确计数器清零
								}
						}
						else
						{
							  ErrorCont++;
							  LCD_Write_String(0,1,"      error     "); //显示错误
							  if(ErrorCont==3)//错误输入计数达三次时，报警
								{
									   do
										{
											  LCD_Write_String(0,1,"Keyboard Locked!");
											  RELAY=0;					//关闭锁
												BEEP = !BEEP;
												delay_ms(55);
										}while(1);
								}
								else
								{
									  /////////////////////密码输错一次，蜂鸣器长响一声
									  BEEP=1;
										delay_ms(1000);
									  BEEP=0;
										LCD_Write_String(0,1,"password:       ");
										LCD_Write_Com(0x80+0x40+9);
										LCD_Write_Com(0x0F);//光标闪烁
								}
						}
				}
				DataInit();  //将输入数据计数器清零，为下一次输入作准备
		}
}


//按键响应程序，参数是键值
//返回键值：
//         7       8      9      10(A)//注册IC卡
//         4       5      6      11(B)//删除IC卡
//         1       2      3      12(C)//注册指纹
//        14(返回) 0   15(确定)  13(D)//修改密码
void KeyPress(uchar keycode)
{
	  unsigned char i;
	
    switch (keycode)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
        if(Register==0&&Delete==0&&RegFingerprint==0)
				{
						if(PressNum < 6)                   					//当输入的密码少于6位时，接受输入并保存，大于6位时则无效。
						{  
							LCD_Write_String(0,1,"input:          ");//显示输入
							BuzzerRingsNum(1);								//按键提示音						
							for(i=0; i<=PressNum; i++)
							{
								LCD_Write_Com(0x80+0x46+i);
								LCD_Write_Data('*');			//显示*号
							}
							InputData[PressNum]=keycode;		//存储密码
							PressNum++;
						}
						else										//输入数据位数大于6后，忽略输入
						{
							PressNum=6;
						}
				}
				break;
    case 10:   //A键
			  if(RegFingerprint==0&&zc==1)
				{
						Register = !Register;
						Delete = 0;
						LCD_Write_Com(0x0C);
						if(Register==1)
						{
								LCD_Write_String(0,0,"  Add IC MODE   ");
						}
						else InitDisplay = 1;
				}
        break;
    case 11:	//B键
			  if(RegFingerprint==0&&zc==1)
				{
						Delete = !Delete;
						Register = 0;
						LCD_Write_Com(0x0C);
						if(Delete==1)
						{
								LCD_Write_String(0,0," Delete IC MODE   "); 
						}
						else InitDisplay = 1;
			  }
        break;
		case 12:	//C键
        if(Register==0&&Delete==0&&zc==1)
				{
							RegFingerprint=!RegFingerprint;
							if(RegFingerprint==1)
							{
									display_user_id();
							}
							else {InitDisplay = 1;zc=0;}
				}
        break;
    case 13:   //D键
			  if(Register==0&&Delete==0&&RegFingerprint==0)
				{
						ResetPassword();
				}
        break;
		case 14:   //*键
			  if(Register==0&&Delete==0&&RegFingerprint==0)
				{
						Cancel();
				}					
        break;
    case 15:   //#键
			  if(Register==0&&Delete==0&&RegFingerprint==0)
				{
						Ensure();
				}
        break;

    }
}

void  finger_ctrl(void)
{
    u8 ret;
    if(PS_Sta == 1)//感应到有手指按上去
		{
				if (RegFingerprint == 1)//指纹录入
				{
						ret = AS608_Add_Fingerprint(user_id);//指纹添加新用户
						if (ret== 0)
						{
								user_id ++;
								if (user_id > 9)//最多录9个指纹
										user_id = 0;

								display_user_id();//显示
								BuzzerRingsNum(1);//蜂鸣器响
						}
				}
				if (RegFingerprint ==0)
				{
						unsigned short ret_userid = 0;

						ret_userid = AS608_Find_Fingerprint();//扫描指纹
						if (ret_userid != 0xFFFE)//说明识别到指纹
						{
								LCD_Write_String(0,1,"      open      ");  //显示开锁
								RELAY = 1; 					//继电器开启
								RELAY_TIME = 15; 		//继电器开启1秒
								pass = 1;						//密码正确标志
								BuzzerRingsNum(2);				//提示音,响两声
							
							
						}
						else 
						{
							ErrorCont++;
							LCD_Write_String(0,1,"      error     "); //显示错误
							if(ErrorCont==3)//错误输入计数达三次时，报警
							{
									 do
									{
											LCD_Write_String(0,1,"Keyboard Locked!");
											RELAY=0;					//关闭锁
											BEEP = !BEEP;
											delay_ms(55);
									}while(1);
							}
							else
							{
									/////////////////////密码输错一次，蜂鸣器长响一声
									BEEP=1;
									delay_ms(1000);
									BEEP=0;
									LCD_Write_String(0,1,"password:       ");
									LCD_Write_Com(0x80+0x40+9);
									LCD_Write_Com(0x0F);//光标闪烁
							}	
						}
				}
		}
}




int main(void)
{	
		delay_init();	    //延时函数初始化	  
	  NVIC_Configuration();
		KEY_GPIO_Init();    //按键初始化
		BEEP_AND_RELAY_GPIO_Init();//蜂鸣器继电器初始化
		delay_ms(500);       //上电瞬间加入一定延时在初始化
	  DataInit();
		LCD_Init();         //屏幕初始化
	  LCD_Write_String(0,0,"   Loading...   ");
	  LCD_Write_String(0,1,"                ");
	  CHECK_NEW_MCU();//单片机校验
	  PS_StaGPIO_Init();
	  RC522_Init();
		PcdReset (); //复位RC522 
		M500PcdConfigISOType ( 'A' );//设置工作方式	
	  uart1_Init(57600);   //串口初始化
	  TIM2_Init(999,719);    //以100Hz计数，定时10ms
		//Tout = ((arr+1)*(psc+1))/Tclk ;  
		//Tclk:定时器输入频率(单位MHZ)
		//Tout:定时器溢出时间(单位us)
	  LCD_Write_String(0,0,"T:00.00 H:00.00");
		while(1)
		{			
          if(InitDisplay==1)
					{
							InitDisplay = 0;
						  BEEP = 0;
						  DataInit();
						  CorrectCont=0;									//正确计数器清零
						  RELAY = 0; //继电器关闭
						  LCD_Write_String(0,1,"password:       ");
							LCD_Write_Com(0x80+0x40+9);
							LCD_Write_Com(0x0F);//光标闪烁
					}	
					finger_ctrl();//指纹处理函数
					if(Delete == 0&&Register==0)
					COMPER_ID_MODE();	
					ADD_ID_MODE();	
					DEL_ID_MODE();					
					//获取按键
					keycode = KEYSCAN();
					//有效键值0-15
					if ((keycode<16)&&(FlagKeyPress==0))
					{
							FlagKeyPress = 1;
							KeyPress(keycode);
							FlagKeyPress = 0;
					}
					delay_ms(10);
		}
		
}

void WRITE_IC_NUM_TO_FLASH(u8* ID_Buffer,u8 space)//IC卡号写入STM32内部FLASH
{
		  STMFLASH_Write(FLASH_SAVE_ADDR + 0x10 * space,(u16*)ID_Buffer,SIZE); 
      delay_ms(100);                             //内部FLASH寿命不长，防止误操作反复擦鞋
}

void READ_IC_NUM_FOR_FLASH(u8* ID_TEMP_Buffer ,u8 space)//从STM32内部FLASH读出IC卡号
{
		 STMFLASH_Read(FLASH_SAVE_ADDR + 0x10 * space,(u16*)ID_TEMP_Buffer,SIZE);
}
 
void WRITE_DATA_TO_FLASH(u8* ID_Buffer,u8 LEN)//密码写入STM32内部FLASH
{
		STMFLASH_Write(FLASH_SAVE_ADDR + 0x10*MAX_PEOPLE*2,(u16*)ID_Buffer,LEN); 
    delay_ms(100);                             //内部FLASH寿命不长，防止误操作反复擦写
}

void READ_DATA_FOR_FLASH(u8* ID_TEMP_Buffer ,u8 LEN)//从STM32内部FLASH读出密码
{
		STMFLASH_Read(FLASH_SAVE_ADDR + 0x10*MAX_PEOPLE*2,(u16*)ID_TEMP_Buffer,LEN);
}

void CHECK_NEW_MCU(void)  // 检查是否是新的单片机，是的话清空存储区，否则保留
{
	  u8 comper_str[6],i=0;
    u8 clear_str[10] = {0};
		memset(clear_str,'0',sizeof(clear_str));//把clear_str这个数组全部清 ‘0’
	  STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)comper_str,5); //从0X0801F010这个地址读出数据 
	  comper_str[5] = '\0';
	  if(strstr(comper_str,"FDYDZ") == NULL)  //新的单片机
		{
			 WRITE_DATA_TO_FLASH(initpassword,6); //单片机第一次使用时，密码为6个0
			 for(i = 0; i < MAX_PEOPLE; i++)//把存卡内部缓存全部清零
			 {
					WRITE_IC_NUM_TO_FLASH(clear_str,i);//存IC的地址内容全部清零
			 }
			 STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)"FDYDZ",5); //写入“FDYDZ”，方便下次校验
	  }
		READ_DATA_FOR_FLASH(CurrentPassword,6); 					//从STM32内部FLASH里读出存储密码
}

u8 RC522_SCAN(u8* BUF)//扫描IC卡
{
	   if(Delete == 1||Register==1)
		 {
				 static u8 i = 0;
				 switch(i)
				 {
					 case 0:    LCD_Write_String(0,1,"      ()        "); break;
					 case 1:    LCD_Write_String(0,1,"     (())       "); break;
					 case 2:    LCD_Write_String(0,1,"    ((()))      "); break;
					 case 3:    LCD_Write_String(0,1,"   (((())))     "); break;
					 case 4:    LCD_Write_String(0,1,"  ((((()))))    "); break;
					 case 5:    LCD_Write_String(0,1,"                "); break;
					 default :  i = 0; break;
				 }
				 i++;
			}
		 if((GET_PID(ucArray_ID) == MI_OK))//获取RC522卡号
		 {
			sprintf(BUF,"%02X%02X%02X%02X", ucArray_ID [ 0 ], ucArray_ID [ 1 ], ucArray_ID [ 2 ], ucArray_ID [ 3 ] );//吧IC卡号以字符串的形式拷贝到BUF中
			return 1;
		 }		 
		return 0; 
}


void COMPER_ID_MODE(void)//正常待机刷卡函数
{
		 if(RC522_SCAN(ID_BUF)) //检测到有卡刷入
		 {  
			 u8 i = 0;
			 ICpass = 0;
			 LCD_Write_Com(0x0C);//关闭光标
			 LCD_Write_String(0,1," IC:             ");
			 LCD_Write_String(4,1,ID_BUF);//显示卡号
			 BEEP = 1; //蜂鸣器响
			 delay_ms(500);
			 BEEP = 0;
       			 
			 for(i = 0; i < MAX_PEOPLE; i++)//从5张IC卡中扫描
			 {
					READ_IC_NUM_FOR_FLASH(ID_TEMP_Buffer,i);//读取STM32内部FLASH存储的卡号
					if(strstr(ID_TEMP_Buffer,ID_BUF) != NULL ) //查找匹配正确
					{
						 LCD_Write_String(0,1,"      open      ");  //显示开锁
						 RELAY = 1; 					//继电器开启
						 RELAY_TIME = 15; 		//继电器开启15秒
						 ICpass = 1;						//IC正确标志	
						 ErrorCont=0;						
						 break;  
					}
					
					else
					{
						ICpass = 0;
					}
			 }
			 if(ICpass == 0)
			 {
					ErrorCont++;
					LCD_Write_String(0,1,"      error     "); //显示错误
					if(ErrorCont==3)//错误输入计数达三次时，报警
					{
							 do
							{
									LCD_Write_String(0,1,"Keyboard Locked!");
									RELAY=0;					//关闭锁
									BEEP = !BEEP;
									delay_ms(55);
							}while(1);
					}
					else
					{
							/////////////////////密码输错一次，蜂鸣器长响一声
							BEEP=1;
							delay_ms(1000);
							BEEP=0;
							LCD_Write_String(0,1,"password:       ");
							LCD_Write_Com(0x80+0x40+9);
							LCD_Write_Com(0x0F);//光标闪烁
					}	
					//break;			
				  InitDisplay = 1;
			 }
		 }
}

void ADD_ID_MODE(void)//注册IC函数
{
	 if(Register == 1)
	 { 
	     if(RC522_SCAN(ID_BUF)) //检测到有卡刷入
			 {
				 u8 i = 0;
         Register = 0;
				 zc=0;				 
				 LCD_Write_String(0,1," IC:             ");
			   LCD_Write_String(4,1,ID_BUF);//显示卡号
				 BEEP = 1; //蜂鸣器响
				 delay_ms(500);
         BEEP = 0; 
			   for(i = 0; i < MAX_PEOPLE; i++)//从5张IC卡中扫描
				 {
				    READ_IC_NUM_FOR_FLASH(ID_TEMP_Buffer,i);//读取STM32内部FLASH存储的卡号
					  if(strstr(ID_TEMP_Buffer,ID_BUF) != NULL)//检测到相同ID
							break;
					  if(ID_TEMP_Buffer[8] == '0' )              //ID_TEMP_Buffer第8位是0，说明是没有存储过的 XXXXXXXXD
				      break;  
				 }
				 delay_ms(1000);
				 if(i == MAX_PEOPLE)    LCD_Write_String(0,1,"   memery full  "); //IC卡号最多存5张，如果等于5，说明之前存过5张卡了。则不能继续注册IC
				 else
				 {
				     sprintf(ID_TEMP_Buffer,"%s%d",ID_BUF,1);//把IC卡号拷贝到ID_TEMP_Buffer中，第8位存储1，说明卡号被存储
				     WRITE_IC_NUM_TO_FLASH(ID_TEMP_Buffer,i);//存储到STM32内部FLASH
				     LCD_Write_String(0,1,"   save ok!  ");//显示存储成功
				 }
				 delay_ms(1000); 
         InitDisplay = 1;				 
			   }
		}
}

void DEL_ID_MODE(void)//删除IC卡函数
{
	 if(Delete == 1)
	 {
	     if(RC522_SCAN(ID_BUF)) //检测到有卡刷入
			 {  
				 u8 i = 0;
				 Delete = 0;
				 zc=0;
				 LCD_Write_String(0,1," IC:             ");
			   LCD_Write_String(4,1,ID_BUF);//显示卡号
				 BEEP = 1; //蜂鸣器响
				 delay_ms(500);
         BEEP = 0; 
			   for(i = 0; i < MAX_PEOPLE; i++)//从5张IC卡中扫描
				 {
				    READ_IC_NUM_FOR_FLASH(ID_TEMP_Buffer,i);//读取STM32内部FLASH存储的卡号
					  if(strstr(ID_TEMP_Buffer,ID_BUF) != NULL)              //找到IC卡号
				      break;  
				 }
				 delay_ms(1000);
				 if(i == MAX_PEOPLE)    LCD_Write_String(0,1,"   find fail  "); //扫描5次都没有扫描到要删除的卡号
				 else
				 {
						sprintf(ID_TEMP_Buffer,"%02X%02X%02X%02X%d",0,0,0,0,0);//ID_TEMP_Buffer缓存全部写入0
						WRITE_IC_NUM_TO_FLASH(ID_TEMP_Buffer,i);//存储到STM32内部FLASH
					  LCD_Write_String(0,1,"   Delete ok!     ");//显示清除成功
				 }
				 delay_ms(1000);  
				 InitDisplay = 1;	
			 }
		}
}


