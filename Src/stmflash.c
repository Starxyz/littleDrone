#include "stmflash.h"
#include "delay.h"
#include "usart.h"
#include "mpu6050.h"
#include "control.h"
#include "ANO_DT.h"

#define EE_6050_ACC_X_OFFSET_ADDR	0
#define EE_6050_ACC_Y_OFFSET_ADDR	1
#define EE_6050_ACC_Z_OFFSET_ADDR	2
#define EE_6050_GYRO_X_OFFSET_ADDR	3
#define EE_6050_GYRO_Y_OFFSET_ADDR	4
#define EE_6050_GYRO_Z_OFFSET_ADDR	5
#define EE_PID_RATE_ROLL_P			6
#define EE_PID_RATE_ROLL_I			7
#define EE_PID_RATE_ROLL_D			8
#define EE_PID_RATE_PIT_P			9
#define EE_PID_RATE_PIT_I			10
#define EE_PID_RATE_PIT_D			11
#define EE_PID_RATE_YAW_P			12
#define EE_PID_RATE_YAW_I			13
#define EE_PID_RATE_YAW_D			14
#define EE_PID_ANGLE_ROLL_P			15
#define EE_PID_ANGLE_ROLL_I			16
#define EE_PID_ANGLE_ROLL_D			17
#define EE_PID_ANGLE_PIT_P			18
#define EE_PID_ANGLE_PIT_I			19
#define EE_PID_ANGLE_PIT_D			20
#define EE_PID_ANGLE_YAW_P			21
#define EE_PID_ANGLE_YAW_I			22
#define EE_PID_ANGLE_YAW_D			23
#define EE_MAG_X_OFFSET_ADDR		24
#define EE_MAG_Y_OFFSET_ADDR		25
#define EE_MAG_Z_OFFSET_ADDR		26

u32 VirtAddVarTab[] = {
		0x1EE00, 0x1EE02, 0x1EE04,//acc_offset(XYZ)
		0x1EE06, 0x1EE08, 0x1EE0A,//gyro_offset(XYZ)
		0x1EE1C, 0x1EE1E, 0x1EE20,//rate_rol(P I D)
		0x1EE22, 0x1EE24, 0x1EE26,//rate_pit(P I D)
		0x1EE28, 0x1EE2A, 0x1EE2C,//rate_yaw(P I D)
		0x1EE2E, 0x1EE30, 0x1EE32,//ang_rol(P I D)
		0x1EE34, 0x1EE36, 0x1EE38,//ang_pit(P I D)
		0x1EE3A, 0x1EE3C, 0x1EE3E,//ang_yaw(P I D)
		0x1EE40, 0x1EE42, 0x1EE44,//mag_offset(XYZ)
};

//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//STM32 FLASH ��������	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2013/7/27
//�汾��V1.1
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////	
//********************************************************************************
//V1.1�޸�˵��
//������STMFLASH_Write������ַƫ�Ƶ�һ��bug.
//////////////////////////////////////////////////////////////////////////////////

																		 
/*******************************************************************************
*FLASH as EEPROM
*Fuction
*
*
********************************************************************************/

void Para_ResetToFactorySetup()
{
	roll_rate_PID.P  = 1.5; roll_rate_PID.I  = 0.00; roll_rate_PID.D  = 0.040;

	pitch_rate_PID.P = 1.5 ;pitch_rate_PID.I = 0.00; pitch_rate_PID.D = 0.040;

	yaw_rate_PID.P   = 1.0; yaw_rate_PID.I   = 0.00; yaw_rate_PID.D   = 0;

	roll_angle_PID.P  = 1.5; roll_angle_PID.I  = 0.00; roll_angle_PID.D  = 0.040;

	pitch_angle_PID.P = 1.5 ;pitch_angle_PID.I = 0.00; pitch_angle_PID.D = 0.040;

	yaw_angle_PID.P   = 1.0; yaw_angle_PID.I   = 0.00; yaw_angle_PID.D   = 0;


	EE_SAVE_PID();
	f.send_pid1 = 1;
	f.send_pid2 = 1;
//	f.send_pid3 = 1;
//	f.send_pid4 = 1;
//	f.send_pid5 = 1;
//	f.send_pid6 = 1;

}

void Param_SavePID()
{
	EE_SAVE_PID();
}

void EE_SAVE_ACC_OFFSET(void)
{
	EE_WriteVariable(VirtAddVarTab[EE_6050_ACC_X_OFFSET_ADDR], (int16_t)(imu.accOffset[X] * 1000));
	EE_WriteVariable(VirtAddVarTab[EE_6050_ACC_Y_OFFSET_ADDR], (int16_t)(imu.accOffset[Y] * 1000));
	EE_WriteVariable(VirtAddVarTab[EE_6050_ACC_Z_OFFSET_ADDR], (int16_t)(imu.accOffset[Z] * 1000));
}
void EE_READ_ACC_OFFSET(void)
{
	int16_t acc_offset[3];

	EE_ReadVariable(VirtAddVarTab[EE_6050_ACC_X_OFFSET_ADDR], &acc_offset[0]);
	EE_ReadVariable(VirtAddVarTab[EE_6050_ACC_Y_OFFSET_ADDR], &acc_offset[1]);
	EE_ReadVariable(VirtAddVarTab[EE_6050_ACC_Z_OFFSET_ADDR], &acc_offset[2]);

	imu.accOffset[X] = acc_offset[0] / 1000.f;
	imu.accOffset[Y] = acc_offset[1] / 1000.f;
	imu.accOffset[Z] = acc_offset[2] / 1000.f;
}
void EE_SAVE_GYRO_OFFSET(void)
{
	EE_WriteVariable(VirtAddVarTab[EE_6050_GYRO_X_OFFSET_ADDR], (int16_t)(imu.gyroOffset[X] * 1000));
	EE_WriteVariable(VirtAddVarTab[EE_6050_GYRO_Y_OFFSET_ADDR], (int16_t)(imu.gyroOffset[Y] * 1000));
	EE_WriteVariable(VirtAddVarTab[EE_6050_GYRO_Z_OFFSET_ADDR], (int16_t)(imu.gyroOffset[Z] * 1000));
}
void EE_READ_GYRO_OFFSET(void)
{
	int16_t gyro_offset[3];

	EE_ReadVariable(VirtAddVarTab[EE_6050_GYRO_X_OFFSET_ADDR], &gyro_offset[0]);
	EE_ReadVariable(VirtAddVarTab[EE_6050_GYRO_Y_OFFSET_ADDR], &gyro_offset[1]);
	EE_ReadVariable(VirtAddVarTab[EE_6050_GYRO_Z_OFFSET_ADDR], &gyro_offset[2]);

	imu.gyroOffset[X] = gyro_offset[0] / 1000.f;
	imu.gyroOffset[Y] = gyro_offset[1] / 1000.f;
	imu.gyroOffset[Z] = gyro_offset[2] / 1000.f;
}

void EE_SAVE_MAG_OFFSET(void)
{
	EE_WriteVariable(VirtAddVarTab[EE_MAG_X_OFFSET_ADDR], imu.magOffset[X]);
	EE_WriteVariable(VirtAddVarTab[EE_MAG_Y_OFFSET_ADDR], imu.magOffset[Y]);
	EE_WriteVariable(VirtAddVarTab[EE_MAG_Z_OFFSET_ADDR], imu.magOffset[Z]);

}

void EE_READ_MAG_OFFSET(void)
{
	EE_ReadVariable(VirtAddVarTab[EE_MAG_X_OFFSET_ADDR], &imu.magOffset[X]);
	EE_ReadVariable(VirtAddVarTab[EE_MAG_Y_OFFSET_ADDR], &imu.magOffset[Y]);
	EE_ReadVariable(VirtAddVarTab[EE_MAG_Z_OFFSET_ADDR], &imu.magOffset[Z]);
}


void EE_SAVE_PID(void)
{
	u16 _temp;
	_temp = (uint16_t)(roll_rate_PID.P * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_RATE_ROLL_P],_temp);
	_temp = (uint16_t)(roll_rate_PID.I * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_RATE_ROLL_I],_temp);
	_temp = (uint16_t)(roll_rate_PID.D * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_RATE_ROLL_D],_temp);
	_temp = (uint16_t)(pitch_rate_PID.P * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_RATE_PIT_P],_temp);
	_temp = (uint16_t)(pitch_rate_PID.I * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_RATE_PIT_I],_temp);
	_temp = (uint16_t)(pitch_rate_PID.D * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_RATE_PIT_D],_temp);
	_temp = (uint16_t)(yaw_rate_PID.P * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_RATE_YAW_P],_temp);
	_temp = (uint16_t)(yaw_rate_PID.I * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_RATE_YAW_I],_temp);
	_temp = (uint16_t)(yaw_rate_PID.D * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_RATE_YAW_D],_temp);

	_temp = (uint16_t)(roll_angle_PID.P * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_ANGLE_ROLL_P],_temp);
	_temp = (uint16_t)(roll_angle_PID.I * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_ANGLE_ROLL_I],_temp);
	_temp = (uint16_t)(roll_angle_PID.D * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_ANGLE_ROLL_D],_temp);
	_temp = (uint16_t)(pitch_angle_PID.P * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_ANGLE_PIT_P],_temp);
	_temp = (uint16_t)(pitch_angle_PID.I * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_ANGLE_PIT_I],_temp);
	_temp = (uint16_t)(pitch_angle_PID.D * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_ANGLE_PIT_D],_temp);
	_temp = (uint16_t)(yaw_angle_PID.P * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_ANGLE_YAW_P],_temp);
	_temp = (uint16_t)(yaw_angle_PID.I * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_ANGLE_YAW_I],_temp);
	_temp = (uint16_t)(yaw_angle_PID.D * 1000);
	EE_WriteVariable(VirtAddVarTab[EE_PID_ANGLE_YAW_D],_temp);
}
void EE_READ_PID(void)
{
	u16 _temp;

	roll_rate_PID.iLimit = 300;
	pitch_rate_PID.iLimit = 300;
	yaw_rate_PID.iLimit = 300;
	roll_angle_PID.iLimit = 300;
	pitch_angle_PID.iLimit = 300;
	yaw_angle_PID.iLimit = 300;

	EE_ReadVariable(VirtAddVarTab[EE_PID_RATE_ROLL_P],&_temp);
	roll_rate_PID.P = (float)_temp / 1000;
	EE_ReadVariable(VirtAddVarTab[EE_PID_RATE_ROLL_I],&_temp);
	roll_rate_PID.I = (float)_temp / 1000;
	EE_ReadVariable(VirtAddVarTab[EE_PID_RATE_ROLL_D],&_temp);
	roll_rate_PID.D = (float)_temp / 1000;
	EE_ReadVariable(VirtAddVarTab[EE_PID_RATE_PIT_P],&_temp);
	pitch_rate_PID.P = (float)_temp / 1000;
	EE_ReadVariable(VirtAddVarTab[EE_PID_RATE_PIT_I],&_temp);
	pitch_rate_PID.I = (float)_temp / 1000;
	EE_ReadVariable(VirtAddVarTab[EE_PID_RATE_PIT_D],&_temp);
	pitch_rate_PID.D = (float)_temp / 1000;
	EE_ReadVariable(VirtAddVarTab[EE_PID_RATE_YAW_P],&_temp);
	yaw_rate_PID.P = (float)_temp / 1000;
	EE_ReadVariable(VirtAddVarTab[EE_PID_RATE_YAW_I],&_temp);
	yaw_rate_PID.I = (float)_temp / 1000;
	EE_ReadVariable(VirtAddVarTab[EE_PID_RATE_YAW_D],&_temp);
	yaw_rate_PID.D = (float)_temp / 1000;

	EE_ReadVariable(VirtAddVarTab[EE_PID_ANGLE_ROLL_P],&_temp);
	roll_angle_PID.P = (float)_temp / 1000;
	EE_ReadVariable(VirtAddVarTab[EE_PID_ANGLE_ROLL_I],&_temp);
	roll_angle_PID.I = (float)_temp / 1000;
	EE_ReadVariable(VirtAddVarTab[EE_PID_ANGLE_ROLL_D],&_temp);
	roll_angle_PID.D = (float)_temp / 1000;
	EE_ReadVariable(VirtAddVarTab[EE_PID_ANGLE_PIT_P],&_temp);
	pitch_angle_PID.P = (float)_temp / 1000;
	EE_ReadVariable(VirtAddVarTab[EE_PID_ANGLE_PIT_I],&_temp);
	pitch_angle_PID.I = (float)_temp / 1000;
	EE_ReadVariable(VirtAddVarTab[EE_PID_ANGLE_PIT_D],&_temp);
	pitch_angle_PID.D = (float)_temp / 1000;
	EE_ReadVariable(VirtAddVarTab[EE_PID_ANGLE_YAW_P],&_temp);
	yaw_angle_PID.P = (float)_temp / 1000;
	EE_ReadVariable(VirtAddVarTab[EE_PID_ANGLE_YAW_I],&_temp);
	yaw_angle_PID.I = (float)_temp / 1000;
	EE_ReadVariable(VirtAddVarTab[EE_PID_ANGLE_YAW_D],&_temp);
	yaw_angle_PID.D = (float)_temp / 1000;

	f.send_pid1 = 1;
	f.send_pid2 = 1;
}

u8 EE_WriteVariable(u32 addr,u16 data)
{
	STMFLASH_Write(STM32_FLASH_BASE+(u32)addr,&data,2);
	return 0;
}

u8 EE_ReadVariable(u32 addr,u16* data)
{
	STMFLASH_Read(STM32_FLASH_BASE+(u32)addr,data,2);
	return 0;
}

//����STM32��FLASH
void STMFLASH_Unlock(void)
{
  FLASH->KEYR=MY_FLASH_KEY1;//д���������.
  FLASH->KEYR=MY_FLASH_KEY2;
}
//flash����
void STMFLASH_Lock(void)
{
  FLASH->CR|=1<<7;//����
}
//�õ�FLASH״̬
u8 STMFLASH_GetStatus(void)
{	
	u32 res;		
	res=FLASH->SR; 
	if(res&(1<<0))return 1;		    //æ
	else if(res&(1<<2))return 2;	//��̴���
	else if(res&(1<<4))return 3;	//д��������
	return 0;						//�������
}
//�ȴ��������
//time:Ҫ��ʱ�ĳ���
//����ֵ:״̬.
u8 STMFLASH_WaitDone(u16 time)
{
	u8 res;
	do
	{
		res=STMFLASH_GetStatus();
		if(res!=1)break;//��æ,����ȴ���,ֱ���˳�.
		delay_us(1);
		time--;
	 }while(time);
	 if(time==0)res=0xff;//TIMEOUT
	 return res;
}
//����ҳ
//paddr:ҳ��ַ
//����ֵ:ִ�����
u8 STMFLASH_ErasePage(u32 paddr)
{
	u8 res=0;
	res=STMFLASH_WaitDone(0X5FFF);//�ȴ��ϴβ�������,>20ms    
	if(res==0)
	{ 
		FLASH->CR|=1<<1;//ҳ����
		FLASH->AR=paddr;//����ҳ��ַ 
		FLASH->CR|=1<<6;//��ʼ����		  
		res=STMFLASH_WaitDone(0X5FFF);//�ȴ���������,>20ms  
		if(res!=1)//��æ
		{
			FLASH->CR&=~(1<<1);//���ҳ������־.
		}
	}
	return res;
}
//��FLASHָ����ַд�����
//faddr:ָ����ַ(�˵�ַ����Ϊ2�ı���!!)
//dat:Ҫд�������
//����ֵ:д������
u8 STMFLASH_WriteHalfWord(u32 faddr, u16 dat)
{
	u8 res;	   	    
	res=STMFLASH_WaitDone(0XFF);	 
	if(res==0)//OK
	{
		FLASH->CR|=1<<0;//���ʹ��
		*(vu16*)faddr=dat;//д������
		res=STMFLASH_WaitDone(0XFF);//�ȴ��������
		if(res!=1)//�����ɹ�
		{
			FLASH->CR&=~(1<<0);//���PGλ.
		}
	} 
	return res;
} 
//��ȡָ����ַ�İ���(16λ����) 
//faddr:����ַ 
//����ֵ:��Ӧ����.
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}
#if STM32_FLASH_WREN	//���ʹ����д   
//������д��
//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��   
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		STMFLASH_WriteHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//��ַ����2.
	}  
} 
//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
//pBuffer:����ָ��
//NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
#if STM32_FLASH_SIZE<256
#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
#define STM_SECTOR_SIZE	2048
#endif		 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//�����2K�ֽ�
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)	
{
	u32 secpos;	   //������ַ
	u16 secoff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	u16 secremain; //������ʣ���ַ(16λ�ּ���)	   
 	u16 i;    
	u32 offaddr;   //ȥ��0X08000000��ĵ�ַ
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE))){
		SendChar("error flash\r\n");
		return;//�Ƿ���ַ
	}
	STMFLASH_Unlock();						//����
	offaddr=WriteAddr-STM32_FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
	secpos=offaddr/STM_SECTOR_SIZE;			//������ַ  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//����ʣ��ռ��С   
	if(NumToWrite<=secremain)secremain=NumToWrite;//�����ڸ�������Χ
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//������������������
		for(i=0;i<secremain;i++)//У������
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			STMFLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//�����������
			for(i=0;i<secremain;i++)//����
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//д����������  
		}else STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if(NumToWrite==secremain)break;//д�������
		else//д��δ����
		{
			secpos++;				//������ַ��1
			secoff=0;				//ƫ��λ��Ϊ0 	 
		   	pBuffer+=secremain;  	//ָ��ƫ��
			WriteAddr+=secremain*2;	//д��ַƫ��(16λ���ݵ�ַ,��Ҫ*2)	   
		   	NumToWrite-=secremain;	//�ֽ�(16λ)���ݼ�
			if(NumToWrite>(STM_SECTOR_SIZE/2))secremain=STM_SECTOR_SIZE/2;//��һ����������д����
			else secremain=NumToWrite;//��һ����������д����
		}	 
	};	
	STMFLASH_Lock();//����
}
#endif

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr+=2;//ƫ��2���ֽ�.	
	}
}

//////////////////////////////////////////������///////////////////////////////////////////
//WriteAddr:��ʼ��ַ
//WriteData:Ҫд�������
void Test_Write(u32 WriteAddr,u16 WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);//д��һ���� 
}
















