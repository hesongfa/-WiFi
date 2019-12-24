//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//����Э���
#include "onenet.h"

//�����豸
#include "esp8266.h"

//Ӳ������
#include "delay.h"
#include "usart.h"

//C��
#include <string.h>

#include "YuYin.h"

#include "MP3.h"


u8 WakeUp_Flag=0;//���ѱ�־λ


//////////////////////////////�̵�����ʼ��///////////////////////////////

void jidianqi_Init(void)
{
	
	GPIO_InitTypeDef gpio_initstruct;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA , ENABLE);	//��GPIOA��GPIOC��ʱ��
	
	gpio_initstruct.GPIO_Mode = GPIO_Mode_Out_PP;									//����Ϊ�������ģʽ
	gpio_initstruct.GPIO_Pin = GPIO_Pin_1 ;					//��ʼ��Pin5
	gpio_initstruct.GPIO_Speed = GPIO_Speed_50MHz;									//���ص����Ƶ��
	GPIO_Init(GPIOA, &gpio_initstruct);												//��ʼ��GPIOA
	GPIO_ResetBits(GPIOA, GPIO_Pin_1);                                           //��ʼ���رշ���

}

/////////////////////////////////////////////////////////////////////////

/*
************************************************************
*	�������ƣ�	Hardware_Init
*
*	�������ܣ�	Ӳ����ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		��ʼ����Ƭ�������Լ�����豸
************************************************************
*/
void Hardware_Init(void)
{
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	//�жϿ�������������

	delay_init();									//systick��ʼ��
		
	Usart2_Init(115200);							//����2������ESP8266��
	
	YuYin_Init();//��ʼ������ʶ��ģ��
	
	 MP3_Init();//��ʼ��MP3��������ģ��
	
	jidianqi_Init();                                //�̵�����ʼ�� 
	
	
}

/*
************************************************************
*	�������ƣ�	main
*
*	�������ܣ�	
*
*	��ڲ�����	��
*
*	���ز�����	0
*
*	˵����		
************************************************************
*/
int main(void)
{
	
	unsigned short timeCount = 0;	//���ͼ������
	
	unsigned char *dataPtr = NULL;
	
	Hardware_Init();				//��ʼ����ΧӲ��
	
	ESP8266_Init();					//��ʼ��ESP8266
	
	while(OneNet_DevLink())			//����OneNET
		delay_ms(500);

	
	while(1)
	{
		
		if(++timeCount >= 1000)									//���ͼ��10s
		{
			OneNet_SendData();									//��������		
			timeCount = 0;
			ESP8266_Clear();
		}
		
		dataPtr = ESP8266_GetIPD(0);
		if(dataPtr != NULL)
	    OneNet_RevPro(dataPtr);
		
		delay_ms(10);
		
		if(YuYin_RX_STA == 1)//�յ�����ָ��
		{

			if(YuYin_RX_CMD==49 || YuYin_RX_CMD==50)//����
			{
				WakeUp_Flag = 1;//���Ѵʱ�־λ��λ
				MP3_Star(5);  //���
				
			}else if(WakeUp_Flag == 1)
			{
				WakeUp_Flag=0;
				if(YuYin_RX_CMD%4 == 1)//��������
				{
					MP3_Star(1);	//��������
				}
				else if(YuYin_RX_CMD%4 == 2)//��������
				{
					MP3_Star(2); //��������
				}
				else if(YuYin_RX_CMD%4 == 3)//�к�����
				{
					 MP3_Star(3);	//�к�����
				}
				else//�ɻ�����
				{
					 MP3_Star(4); //�ɻ�����
				}
			}
		
			YuYin_RX_STA = 0;
		
		}
	}

}
