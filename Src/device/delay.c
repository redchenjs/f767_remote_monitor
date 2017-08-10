/*
 * delay.c
 *
 *  Created on: 2017年8月4日
 *      Author: redchenjs
 */
#include "stm32f7xx_hal.h"

static uint8_t  fac_us=0;							//us延时倍乘数
static uint16_t fac_ms=0;							//ms延时倍乘数,在os下,代表每个节拍的ms数

//初始化延迟函数
//当使用OS的时候,此函数会初始化OS的时钟节拍
//SYSTICK的时钟固定为AHB时钟的1/8
//SYSCLK:系统时钟频率
void delay_init(uint8_t SYSCLK)
{
 	SysTick->CTRL &= ~(1<<2);					//SYSTICK使用外部时钟源
	fac_us = SYSCLK/8;						//不论是否使用OS,fac_us都需要使用
	fac_ms = ((uint32_t)SYSCLK*1000)/8;		//非OS下,代表每个ms需要的systick时钟数
}

//延时nus
//nus为要延时的us数.
//注意:nus的值,不要大于798915us(最大值即2^24/fac_us@fac_us=21)
void delay_us(uint32_t nus)
{
	uint32_t temp;
	SysTick->LOAD = nus*fac_us; 				//时间加载
	SysTick->VAL = 0x00;        				//清空计数器
	SysTick->CTRL = 0x01 ;      				//开始倒数
	do {
		temp = SysTick->CTRL;
	} while ((temp&0x01) && !(temp&(1<<16)));	//等待时间到达
	SysTick->CTRL = 0x00;      	 			//关闭计数器
	SysTick->VAL  = 0X00;       				//清空计数器
}
//延时nms
//注意nms的范围
//SysTick->LOAD为24位寄存器,所以,最大延时为:
//nms<=0xffffff*8*1000/SYSCLK
//SYSCLK单位为Hz,nms单位为ms
//对216M条件下,nms<=621ms
void delay_xms(uint16_t nms)
{
	uint32_t temp;
	SysTick->LOAD = (uint32_t)nms * fac_ms;		//时间加载(SysTick->LOAD为24bit)
	SysTick->VAL  = 0x00;           			//清空计数器
	SysTick->CTRL = 0x01 ;          			//开始倒数
	do {
		temp = SysTick->CTRL;
	} while ((temp&0x01) && !(temp&(1<<16)));	//等待时间到达
	SysTick->CTRL = 0x00;       				//关闭计数器
	SysTick->VAL  = 0X00;     		  			//清空计数器
}

//延时nms
//nms:0~65535
void delay_ms(uint16_t nms)
{
	uint8_t repeat=nms/540;						//这里用540,是考虑到某些客户可能超频使用,
											//比如超频到248M的时候,delay_xms最大只能延时541ms左右了
	uint16_t remain=nms%540;
	while (repeat) {
		delay_xms(540);
		repeat--;
	}
	if (remain) {
		delay_xms(remain);
	}
}
