/*
 * i2.c
 *
 *  Created on: 2016��12��27��
 *      Author: 50430
 */
#include "i2c.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"


static void I2C_delay(void)
{

   u8 i=5; //��������Ż��ٶ�	����������͵�5����д��
   while(i)
   {
     i--;
   }
}


void IIC_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;

	/*Configure GPIO pins : SDA_Pin SCL_Pin */

	GPIO_InitStruct.Pin = SDA_Pin|SCL_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	IIC_SCL=1;
	IIC_SDA=1;
}
//����IIC��ʼ�ź�
void IIC_Start(void)
{
	SDA_OUT() ;     //sda�����
	IIC_SDA=1;
	IIC_SCL=1;
	delay_us(2);
	IIC_SDA=0;//START:when CLK is high,DATA change form high to low
	delay_us(2);
	IIC_SCL=0;//ǯסI2C���ߣ�׼�����ͻ��������
	delay_us(2);
}
//����IICֹͣ�ź�
void IIC_Stop(void)
{
	SDA_OUT();//sda�����
	IIC_SCL=0;
	delay_us(2);
	IIC_SDA=0;//STOP:when CLK is high DATA change form low to high
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SDA=1;//����I2C���߽����ź�
	delay_us(2);
}
//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
uint8_t IIC_Wait_Ack(void)
{
	uint8_t ucErrTime=0;

	IIC_SCL=1;delay_us(2);
	SDA_IN();      //SDA����Ϊ����
	IIC_SDA=1;delay_us(2);
	IIC_SCL=1;delay_us(2);
	while(READ_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			//SendChar("i2c hahaha \r\n");
			IIC_Stop();
			return 1;
		}
	}
	IIC_SCL=0;//ʱ�����0
	delay_us(2);
	return 0;
}
//����ACKӦ��
void IIC_Ack(void)
{
	IIC_SCL=0;
	delay_us(2);
	SDA_OUT();
	IIC_SDA=0;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
	delay_us(2);
}
//������ACKӦ��
void IIC_NAck(void)
{
	IIC_SCL=0;
	delay_us(2);
	SDA_OUT();
	IIC_SDA=1;
	delay_us(2);
	IIC_SCL=1;
	delay_us(2);
	IIC_SCL=0;
	delay_us(2);
}
//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��
void IIC_Send_Byte(uint8_t txd)
{
    uint8_t t;
	SDA_OUT();
    IIC_SCL=0;//����ʱ�ӿ�ʼ���ݴ���
    delay_us(2);
    for(t=0;t<8;t++)
    {
        IIC_SDA=(txd&0x80)>>7;
        txd<<=1;
        delay_us(2);   //��TEA5767��������ʱ���Ǳ����
		IIC_SCL=1;
		delay_us(2);
		IIC_SCL=0;
		delay_us(2);
    }
}
//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK
uint8_t IIC_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	IIC_SDA = 1;//����Ҫ�ͷ����߰��ֵܣ�������ݵ�ʱ��ack������SDA�����ͷţ����滹������ë����
	SDA_IN();//SDA����Ϊ����
    for(i=0;i<8;i++ )
	{
        IIC_SCL=0;
        delay_us(2);
		IIC_SCL=1;
        receive<<=1;
        if(READ_SDA)receive++;
        delay_us(2);
    }
    if (!ack){
        IIC_NAck();//����nACK
    }
    else{
        IIC_Ack(); //����ACK
    }
    return receive;
}


u8 IIC_Read_Reg(u8 addr,u8 reg)
{
	u8 res;
    IIC_Start();
	IIC_Send_Byte((addr<<1)|0);//����������ַ+д����
	IIC_Wait_Ack();		//�ȴ�Ӧ��
    IIC_Send_Byte(reg);	//д�Ĵ�����ַ
    IIC_Wait_Ack();		//�ȴ�Ӧ��
    IIC_Start();
	IIC_Send_Byte((addr<<1)|1);//����������ַ+������
    IIC_Wait_Ack();		//�ȴ�Ӧ��
	res=IIC_Read_Byte(0);//��ȡ����,����nACK
    IIC_Stop();			//����һ��ֹͣ����
	return res;
}

u8 IIC_Write_Reg(u8 addr,u8 reg,u8 data)
{
    IIC_Start();
	IIC_Send_Byte((addr<<1)|0);//����������ַ+д����
	if(IIC_Wait_Ack())	//�ȴ�Ӧ��
	{
		IIC_Stop();
		return 1;
	}
    IIC_Send_Byte(reg);	//д�Ĵ�����ַ
    IIC_Wait_Ack();		//�ȴ�Ӧ��
	IIC_Send_Byte(data);//��������
	if(IIC_Wait_Ack())	//�ȴ�ACK
	{
		IIC_Stop();
		return 1;
	}
    IIC_Stop();
	return 0;
}

u8 IIC_Write_Reg_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
	u8 i;
    IIC_Start();
	IIC_Send_Byte((addr<<1)|0);//����������ַ+д����
	if(IIC_Wait_Ack())	//�ȴ�Ӧ��
	{
		IIC_Stop();
		return 1;
	}
    IIC_Send_Byte(reg);	//д�Ĵ�����ַ
    IIC_Wait_Ack();		//�ȴ�Ӧ��
	for(i=0;i<len;i++)
	{
		IIC_Send_Byte(buf[i]);	//��������
		if(IIC_Wait_Ack())		//�ȴ�ACK
		{
			IIC_Stop();
			return 1;
		}
	}
    IIC_Stop();
	return 0;
}

u8 IIC_Read_Reg_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{

 	IIC_Start();
	IIC_Send_Byte((addr<<1)|0);//����������ַ+д����

	if(IIC_Wait_Ack())	//�ȴ�Ӧ��
	{
		IIC_Stop();
		return 1;
	}

    IIC_Send_Byte(reg);	//д�Ĵ�����ַ
    IIC_Wait_Ack();		//�ȴ�Ӧ��

    IIC_Start();
    IIC_Send_Byte((addr<<1)|1);//����������ַ+������
	IIC_Wait_Ack();		//�ȴ�Ӧ��

	while(len)
	{
		if(len==1)*buf=IIC_Read_Byte(0);//������,����nACK
		else *buf = IIC_Read_Byte(1);		//������,����ACK

		len--;
		buf++;
	}
    IIC_Stop();	//����һ��ֹͣ����

	return 0;

}
