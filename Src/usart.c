#include "usart.h"
#include "sys.h"
//#include "ANO_DT.h"
//////////////////////////////////////////////////////////////////////////////////
//���ʹ��ucos,����������ͷ�ļ�����.
#if SYSTEM_SUPPORT_UCOS
#include "includes.h"					//ucos ʹ��
#endif



//////////////////////////////////////////////////////////////////////////////////
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//����1��ʼ��
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/12/05
//�汾��V1.6
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved
//********************************************************************************
//V1.3�޸�˵��
//֧����Ӧ��ͬƵ���µĴ��ڲ���������.
//�����˶�printf��֧��
//�����˴��ڽ��������.
//������printf��һ���ַ���ʧ��bug
//V1.4�޸�˵��
//1,�޸Ĵ��ڳ�ʼ��IO��bug
//2,�޸���USART_RX_STA,ʹ�ô����������ֽ���Ϊ2��14�η�
//3,������USART_REC_LEN,���ڶ��崮�����������յ��ֽ���(������2��14�η�)
//4,�޸���EN_USART1_RX��ʹ�ܷ�ʽ
//V1.5�޸�˵��
//1,�����˶�UCOSII��֧��
//////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB

//#if 1
//#pragma import(__use_no_semihosting)
////��׼����Ҫ��֧�ֺ���
//struct __FILE
//{
//	int handle;
//	/* Whatever you require here. If the only file you are using is */
//	/* standard output using printf() for debugging, no file handling */
//	/* is required. */
//};
///* FILE is typedef�� d in stdio.h. */
//FILE __stdout;
////����_sys_exit()�Ա���ʹ�ð�����ģʽ
//sys_exit(int x)
//{
//	x = x;
//}
////�ض���fputc����
//int fputc(int ch, FILE *f)
//{
//	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������
//	USART1->DR = (u8) ch;
//	return ch;
//}
//#endif
//end
//////////////////////////////////////////////////////////////////

#if EN_USART1_RX   //���ʹ���˽���
//����1�жϷ������
//ע��,��ȡUSARTx->SR�ܱ���Ī������Ĵ���
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���

void USART1_IRQHandler(void)
{
	u8 res;
#ifdef OS_CRITICAL_METHOD 	//���OS_CRITICAL_METHOD������,˵��ʹ��ucosII��.
	OSIntEnter();
#endif
	if(USART1->SR&(1<<5))//���յ�����
	{
		res=USART1->DR;

	//	ANO_DT_Data_Receive_Prepare(res);++++++++
//		if((USART_RX_STA&0x8000)==0)//����δ���
//		{
//			if(USART_RX_STA&0x4000)//���յ���0x0d
//			{
//				if(res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
//				else USART_RX_STA|=0x8000;	//���������
//			}else //��û�յ�0X0D
//			{
//				if(res==0x0d)USART_RX_STA|=0x4000;
//				else
//				{
//					USART_RX_BUF[USART_RX_STA&0X3FFF]=res;
//					USART_RX_STA++;
//					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����
//				}
//			}
//		}
	}
#ifdef OS_CRITICAL_METHOD 	//���OS_CRITICAL_METHOD������,˵��ʹ��ucosII��.
	OSIntExit();
#endif
}
#endif
//��ʼ��IO ����1
//pclk2:PCLK2ʱ��Ƶ��(Mhz)
//bound:������
//CHECK OK
//091209
void uart_init(u32 pclk2,u32 bound)
{
	float temp;
	u16 mantissa;
	u16 fraction;
	temp=(float)(pclk2*1000000)/(bound*16);//�õ�USARTDIV
	mantissa=temp;				 //�õ���������
	fraction=(temp-mantissa)*16; //�õ�С������
    mantissa<<=4;
	mantissa+=fraction;
	RCC->APB2ENR|=1<<2;   //ʹ��PORTA��ʱ��
	RCC->APB2ENR|=1<<14;  //ʹ�ܴ���ʱ��
	GPIOA->CRH&=0XFFFFF00F;//IO״̬����
	GPIOA->CRH|=0X000008B0;//IO״̬����

	RCC->APB2RSTR|=1<<14;   //��λ����1
	RCC->APB2RSTR&=~(1<<14);//ֹͣ��λ
	//����������
 	USART1->BRR=mantissa; // ����������
	USART1->CR1|=0X200C;  //1λֹͣ,��У��λ.
#if EN_USART1_RX		  //���ʹ���˽���
	//ʹ�ܽ����ж�
	USART1->CR1|=1<<8;    //PE�ж�ʹ��
	USART1->CR1|=1<<5;    //���ջ������ǿ��ж�ʹ��
	MY_NVIC_Init(3,3,USART1_IRQn,2);//��2��������ȼ�
#endif
}


void SendData(u8 *data,u8 length)
{
	u8 i;
	for(i=0;i<length;i++)
	{
		USART1->DR=data[i];
		while((USART1->SR&0X40)==0);
	}
}

void SendChar(char *s)//�ַ���һ��Ҫ��'\0'��β
{
	u8 t;
	for(t=0;s[t]!='\0';t++)
			{
				USART1->DR=s[t];
				while((USART1->SR&0X40)==0);//�뻻��;//�ȴ����ͽ���
			}
}

void Receive(char* s)
{
	u8 i,len;
	if(USART_RX_STA&0x8000)
		{
			len=USART_RX_STA&0x3FFF;//�õ��˴ν��յ������ݳ���
			for(i=0;i<len;i++)
			{
				s[i] = USART_RX_BUF[i];
			}
			s[i] = '\0';
		}
}
void SendDouble(double d)
{
	char s[20];
	sprintf(s,"%.2lf",d);
	SendChar(s);
}

void Send0x(int a)
{
	char s[20];
	sprintf(s,"%#x",a);
	SendChar(s);
}
void SendInt(int a)
{
	char s[20];
	sprintf(s,"%d",a);
	SendChar(s);

//	char i,j,s[10];
//	char c=1;
//	int b = a;
//	for(i=0;a!=0;i++)
//	{
//		a /=10;
//	}
//	if(b<0){b=-b;c=0;s[0] = '-';}
//	for(j=0;j<i;j++)
//	{
//
//		s[i-j-c] = b%10+48;
//			b/=10;
//	}
//	if(c==0)i++;
//	s[i] = '\0';
//	SendChar(&s[0]);
}
void _10xf(int num)
{
	char a=1,i;
	for(i=0;i<num;i++)
	{
		a *=10;
	}
}

void _n()
{
	USART1->DR='\r';
	while((USART1->SR&0X40)==0);
	USART1->DR='\n';
	while((USART1->SR&0X40)==0);
}
