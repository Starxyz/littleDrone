#include <stm32f103xb.h>
#include "mpu6050.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"   
#include "i2c.h"
#include "stdbool.h"
#include "HAL.h"
#include "stdint.h"
#include "stmflash.h"

u8						mpu6050_buffer[14];					//iic��ȡ��������

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32F407������
//MPU6050 ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2014/5/9
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved									  
////////////////////////////////////////////////////////////////////////////////// 	

//��ʼ��MPU6050
//����ֵ:0,�ɹ�
//    ����,�������

bool MPU6050DetectGyro(gyro_t *gyro)
{
	u8 res;

	res=MPU_Read_Byte(MPU_DEVICE_ID_REG);
	if(res!=MPU_ADDR) return false;

	gyro->init = MPU_Init_Gyro;
	gyro->read = mpuGyroRead;
	//gyro->isDataReady = mpuIsDataReady;

	// 16.4 dps/lsb scalefactor
	gyro->scale = 2000.f / 32768.f;

	return true;
}

bool MPU6050DetectAcc(acc_t *acc)
{
    acc->init = MPU_Init_Acc;
    acc->read = mpuAccRead;
    //acc->revisionCode = (mpuDetectionResult.resolution == MPU_HALF_RESOLUTION ? 'o' : 'n'); // es/non-es variance between MPU6050 sensors, half of the naze boards are mpu6000ES.

	return true;
}

bool mpuAccRead(int16_t *data)
{
	u8 res;
	res = IIC_Read_Reg_Len(MPU_ADDR,MPU_ACCEL_XOUTH_REG,6,mpu6050_buffer);
	data[0] = ((((int16_t)mpu6050_buffer[0]) << 8) | mpu6050_buffer[1]);
	data[1] = ((((int16_t)mpu6050_buffer[2]) << 8) | mpu6050_buffer[3]);
	data[2] = ((((int16_t)mpu6050_buffer[4]) << 8) | mpu6050_buffer[5]);

	return !res;
}

bool mpuGyroRead(int16_t *data)
{
	u8 res;
	res = IIC_Read_Reg_Len(MPU_ADDR,MPU_ACCEL_XOUTH_REG+8,6,mpu6050_buffer+8);
	data[0] = ((((int16_t)mpu6050_buffer[8]) << 8) | mpu6050_buffer[9]);
	data[1] = ((((int16_t)mpu6050_buffer[10]) << 8) | mpu6050_buffer[11]);
	data[2] = ((((int16_t)mpu6050_buffer[12]) << 8) | mpu6050_buffer[13]);

	return !res;
}

void MPU6050_Read(void)
{
	IIC_Read_Reg_Len(MPU_ADDR,MPU_ACCEL_XOUTH_REG,14,mpu6050_buffer);
}


void MPU_Init_Gyro(void)
{ 
	u8 res;
	IIC_Init();//��ʼ��IIC����
	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X80);	//��λMPU6050
    delay_ms(300);
	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X00);	//����MPU6050 
	MPU_Set_Gyro_Fsr(3);					//�����Ǵ�����,��2000dps

	MPU_Set_LPF(42);
	//MPU_Set_Rate(50);						//���ò�����50Hz
	MPU_Write_Byte(MPU_INT_EN_REG,0X00);	//�ر������ж�
	MPU_Write_Byte(MPU_USER_CTRL_REG,0X00);	//I2C��ģʽ�ر�
//	MPU_Write_Byte(MPU_FIFO_EN_REG,0X00);	//�ر�FIFO
	MPU_Write_Byte(MPU_INTBP_CFG_REG,
			0 << 7 | 0 << 6 | 0 << 5 | 0 << 4 | 0 << 3 | 0 << 2 | 1 << 1 | 0 << 0);	//INT���ŵ͵�ƽ��Ч

	MPU_Write_Byte(MPU_PWR_MGMT2_REG,0X00);	//���ٶ��������Ƕ�����
	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X03);	//����CLKSEL,PLL Z��Ϊ�ο�
	//MPU_Set_Rate(50);						//���ò�����Ϊ50Hz

}

void MPU_Init_Acc(void)
{
	MPU_Set_Accel_Fsr(2);					//���ٶȴ�����,��8g
}
//����MPU6050�����Ǵ����������̷�Χ
//fsr:0,��250dps;1,��500dps;2,��1000dps;3,��2000dps
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
u8 MPU_Set_Gyro_Fsr(u8 fsr)
{
	return MPU_Write_Byte(MPU_GYRO_CFG_REG,fsr<<3);//���������������̷�Χ  
}
//����MPU6050���ٶȴ����������̷�Χ
//fsr:0,��2g;1,��4g;2,��8g;3,��16g
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
u8 MPU_Set_Accel_Fsr(u8 fsr)
{
	return MPU_Write_Byte(MPU_ACCEL_CFG_REG,fsr<<3);//���ü��ٶȴ����������̷�Χ  
}
//����MPU6050�����ֵ�ͨ�˲���
//lpf:���ֵ�ͨ�˲�Ƶ��(Hz)
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
u8 MPU_Set_LPF(u16 lpf)
{
	u8 data=0;
	if(lpf>=188)data=1;
	else if(lpf>=98)data=2;
	else if(lpf>=42)data=3;
	else if(lpf>=20)data=4;
	else if(lpf>=10)data=5;
	else data=6; 
	return MPU_Write_Byte(MPU_CFG_REG,data);//�������ֵ�ͨ�˲���  
}
//����MPU6050�Ĳ�����(�ٶ�Fs=1KHz)
//rate:4~1000(Hz)
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
u8 MPU_Set_Rate(u16 rate)
{
	u8 data;
	if(rate>1000)rate=1000;
	if(rate<4)rate=4;
	data=1000/rate-1;
	data=MPU_Write_Byte(MPU_SAMPLE_RATE_REG,data);	//�������ֵ�ͨ�˲���
 	return MPU_Set_LPF(rate/2);	//�Զ�����LPFΪ�����ʵ�һ��
}

//�õ��¶�ֵ
//����ֵ:�¶�ֵ(������100��)
short MPU_Get_Temperature(void)
{
    u8 buf[2]; 
    short raw;
	float temp;
	MPU_Read_Len(MPU_ADDR,MPU_TEMP_OUTH_REG,2,buf); 
    raw=((u16)buf[0]<<8)|buf[1];  
    temp=36.53+((double)raw)/340;  
    return temp*100;
}
//�õ�������ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
u8 MPU_Get_Gyroscope(short *gx,short *gy,short *gz)
{
    u8 buf[6],res;  
	res=MPU_Read_Len(MPU_ADDR,MPU_GYRO_XOUTH_REG,6,buf);
	if(res==0)
	{
		*gx=((u16)buf[0]<<8)|buf[1];  
		*gy=((u16)buf[2]<<8)|buf[3];  
		*gz=((u16)buf[4]<<8)|buf[5];
	} 	
    return res;;
}
//�õ����ٶ�ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
u8 MPU_Get_Accelerometer(short *ax,short *ay,short *az)
{
    u8 buf[6],res=1;  
	res=MPU_Read_Len(MPU_ADDR,MPU_ACCEL_XOUTH_REG,6,buf);
	if(res==0)
	{
		*ax=((u16)buf[0]<<8)|buf[1];  
		*ay=((u16)buf[2]<<8)|buf[3];  
		*az=((u16)buf[4]<<8)|buf[5];
	} 	
    return res;
}
//IIC����д
//addr:������ַ 
//reg:�Ĵ�����ַ
//len:д�볤��
//buf:������
//����ֵ:0,����
//    ����,�������
u8 MPU_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf)
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
//IIC������
//addr:������ַ
//reg:Ҫ��ȡ�ļĴ�����ַ
//len:Ҫ��ȡ�ĳ���
//buf:��ȡ�������ݴ洢��
//����ֵ:0,����
//    ����,�������
u8 MPU_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{ 
	addr =addr;
	while(len--)
	{
		*buf=MPU_Read_Byte(reg);
		buf++;
		reg++;
	}
	return 0;//+++++

// 	IIC_Start(); 
//	IIC_Send_Byte((addr<<1)|0);//����������ַ+д����	
//	if(IIC_Wait_Ack())	//�ȴ�Ӧ��
//	{
//		IIC_Stop();		 
//		return 1;		
//	}
//    IIC_Send_Byte(reg);	//д�Ĵ�����ַ
//    IIC_Wait_Ack();		//�ȴ�Ӧ��
//    IIC_Start();
//	IIC_Send_Byte((addr<<1)|1);//����������ַ+������	
//    IIC_Wait_Ack();		//�ȴ�Ӧ�� 
//	delay_us(4);//+++++++
//	while(len)
//	{
//		if(len==1)*buf=IIC_Read_Byte(0);//������,����nACK 
//		else *buf=IIC_Read_Byte(1);		//������,����ACK  
//		delay_us(4);
//		len--;
//		buf++; 
//	}    
//    IIC_Stop();	//����һ��ֹͣ���� 
//	return 0;	
}
//IICдһ���ֽ� 
//reg:�Ĵ�����ַ
//data:����
//����ֵ:0,����
//    ����,�������
u8 MPU_Write_Byte(u8 reg,u8 data) 				 
{ 
    IIC_Start(); 
	IIC_Send_Byte((MPU_ADDR<<1)|0);//����������ַ+д����	
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
//IIC��һ���ֽ� 
//reg:�Ĵ�����ַ 
//����ֵ:����������
u8 MPU_Read_Byte(u8 reg)
{
	u8 res;
    IIC_Start(); 
	IIC_Send_Byte((MPU_ADDR<<1)|0);//����������ַ+д����	
	IIC_Wait_Ack();		//�ȴ�Ӧ�� 
    IIC_Send_Byte(reg);	//д�Ĵ�����ַ
    IIC_Wait_Ack();		//�ȴ�Ӧ��
    IIC_Start();
	IIC_Send_Byte((MPU_ADDR<<1)|1);//����������ַ+������	
    IIC_Wait_Ack();		//�ȴ�Ӧ�� 
	res=IIC_Read_Byte(0);//��ȡ����,����nACK 
    IIC_Stop();			//����һ��ֹͣ���� 
	return res;		
}


