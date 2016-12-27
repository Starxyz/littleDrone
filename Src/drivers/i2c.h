/*
 * i2c.h
 *
 *  Created on: 2016��12��27��
 *      Author: 50430
 */

#ifndef SRC_DRIVERS_I2C_H_
#define SRC_DRIVERS_I2C_H_

#include "stm32f103xb.h"


#define SDA_IN()  {GPIOA->CRH &= ~(0xF<<12);GPIOA->CRH |= (0x8<<12);}	//PA11����ģʽ
#define SDA_OUT() {GPIOA->CRH &= ~(0xF<<12);GPIOA->CRH |= (0x1<<12);} //PA11���ģʽ
//IO��������
#define IIC_SCL    PAout(12) //SCL
#define IIC_SDA    PAout(11) //SDA
#define READ_SDA   PAin(11)  //����SDA

//IIC���в�������
void IIC_Init(void);                //��ʼ��IIC��IO��
void IIC_Start(void);				//����IIC��ʼ�ź�
void IIC_Stop(void);	  			//����IICֹͣ�ź�
void IIC_Send_Byte(uint8_t txd);			//IIC����һ���ֽ�
uint8_t IIC_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
uint8_t IIC_Wait_Ack(void); 				//IIC�ȴ�ACK�ź�
void IIC_Ack(void);					//IIC����ACK�ź�
void IIC_NAck(void);				//IIC������ACK�ź�

#endif /* SRC_DRIVERS_I2C_H_ */
