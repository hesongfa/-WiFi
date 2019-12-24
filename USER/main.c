//单片机头文件
#include "stm32f10x.h"

//网络协议层
#include "onenet.h"

//网络设备
#include "esp8266.h"

//硬件驱动
#include "delay.h"
#include "usart.h"

//C库
#include <string.h>

#include "YuYin.h"

#include "MP3.h"


u8 WakeUp_Flag=0;//唤醒标志位


//////////////////////////////继电器初始化///////////////////////////////

void jidianqi_Init(void)
{
	
	GPIO_InitTypeDef gpio_initstruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);	//打开GPIOA和GPIOC的时钟
	
	gpio_initstruct.GPIO_Mode = GPIO_Mode_Out_PP;									//设置为推挽输出模式
	gpio_initstruct.GPIO_Pin = GPIO_Pin_1 ;					//初始化Pin5
	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;									//承载的最大频率
	GPIO_Init(GPIOA, &gpio_initstruct);												//初始化GPIOA
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);                                           //初始化关闭风扇

}

/////////////////////////////////////////////////////////////////////////

/*
************************************************************
*	函数名称：	Hardware_Init
*
*	函数功能：	硬件初始化
*
*	入口参数：	无
*
*	返回参数：	无
*
*	说明：		初始化单片机功能以及外接设备
************************************************************
*/
void Hardware_Init(void)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//中断控制器分组设置

	delay_init();									//systick初始化
		
	Usart2_Init(115200);							//串口2，驱动ESP8266用
	
	YuYin_Init();//初始化语音识别模块
	
	 MP3_Init();//初始化MP3语音播报模块
	
	jidianqi_Init();                                //继电器初始化 
	
	
}

/*
************************************************************
*	函数名称：	main
*
*	函数功能：	
*
*	入口参数：	无
*
*	返回参数：	0
*
*	说明：		
************************************************************
*/
int main(void)
{
	
	unsigned short timeCount = 0;	//发送间隔变量
	
	unsigned char *dataPtr = NULL;
	
	Hardware_Init();				//初始化外围硬件
	
	ESP8266_Init();					//初始化ESP8266
	
	while(OneNet_DevLink())			//接入OneNET
		delay_ms(500);

	
	while(1)
	{
		
		if(++timeCount >= 1000)									//发送间隔10s
		{
			OneNet_SendData();									//发送数据		
			timeCount = 0;
			ESP8266_Clear();
		}
		
		dataPtr = ESP8266_GetIPD(0);
		if(dataPtr != NULL)
	    OneNet_RevPro(dataPtr);
		
		delay_ms(10);
		
		if(YuYin_RX_STA == 1)//收到语音指令
		{

			if(YuYin_RX_CMD==49 || YuYin_RX_CMD==50)//唤醒
			{
				WakeUp_Flag = 1;//唤醒词标志位置位
				MP3_Star(5);  //你好
				
			}else if(WakeUp_Flag == 1)
			{
				WakeUp_Flag=0;
				if(YuYin_RX_CMD%4 == 1)//厨余垃圾
				{
					MP3_Star(1);	//厨余垃圾
				}
				else if(YuYin_RX_CMD%4 == 2)//其他垃圾
				{
					MP3_Star(2); //其他垃圾
				}
				else if(YuYin_RX_CMD%4 == 3)//有害垃圾
				{
					 MP3_Star(3);	//有害垃圾
				}
				else//可回收物
				{
					 MP3_Star(4); //可回收物
				}
			}
		
			YuYin_RX_STA = 0;
		
		}
	}

}
