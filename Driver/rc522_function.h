#ifndef __RC522_FUNCTION_H
#define	__RC522_FUNCTION_H


#include "stm32f10x_it.h"


#define          macDummy_Data              0x00


void             PcdReset                   ( void );                       //复位
void             M500PcdConfigISOType       ( u8 type );                    //工作方式
char             PcdRequest                 ( u8 req_code, u8 * pTagType ); //寻卡
char             PcdAnticoll                ( u8 * pSnr);                   //读卡号
char             IC_CMT                     ( u8 * PID, u8 * KEY, u8 RW, u8 * Dat );
u8 GET_PID( u8 *PID);
unsigned char             IC_test                    ( void );
#endif /* __RC522_FUNCTION_H */

