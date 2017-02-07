/*
 * imu.c
 *
 *  Created on: 2016��12��29��
 *      Author: 50430
 */


#include "imu.h"
#include "maths.h"
#include "math.h"
#include "axis.h"
#include "HAL.h"
#include "imu.h"
#include "MPU6050.h"
#include "filter.h"
#include "stmflash.h"

#define RtA 		57.324841				//���ȵ��Ƕ�
#define AtR    	0.0174533				//�ȵ��Ƕ�
#define Acc_G 	0.0011963				//���ٶȱ��G
#define Gyro_G 	0.0610351				//���ٶȱ�ɶ�   �˲�����Ӧ����2000��ÿ��
#define Gyro_Gr	0.0010653				//���ٶȱ�ɻ���	�˲�����Ӧ����2000��ÿ��
#define FILTER_NUM 100

S_INT16_XYZ ACC_AVG;			//ƽ��ֵ�˲����ACC
S_FLOAT_XYZ GYRO_I;				//�����ǻ���
S_FLOAT_XYZ EXP_ANGLE;		//�����Ƕ�
S_FLOAT_XYZ DIF_ANGLE;		//�����Ƕ���ʵ�ʽǶȲ�
attitude_t now_attitude;	//��Ԫ��������ĽǶ�


S_FLOAT_XYZ MAG_P;


int16_t	ACC_X_BUF[FILTER_NUM],
		ACC_Y_BUF[FILTER_NUM],
		ACC_Z_BUF[FILTER_NUM];	//���ٶȻ��������˲�����

int16_t gyro_data[3];
int16_t acc_data[3];
int16_t mag_data[3];

float gyro_rdata[3];
float acc_rdata[3];

float gyro_ldata[3];
float acc_ldata[3];


void Prepare_Data(void)
{
	static uint16_t filter_cnt=0;
	int32_t temp1=0,temp2=0,temp3=0;
	uint16_t i;

	gyro.read(gyro_data);
	acc.read(acc_data);

	for(i=0;i<3;i++)
	{
			acc_rdata[i]= (float)acc_data[i] *ACC_SCALE * CONSTANTS_ONE_G ;
			gyro_rdata[i]=(float)gyro_data[i] * GYRO_SCALE * M_PI_F /180.f;		//deg/s
	}

	acc_ldata[X] = LPF2pApply_1(acc_rdata[X] - ACC_OFFSET.X);
	acc_ldata[Y] = LPF2pApply_2(acc_rdata[Y] - ACC_OFFSET.Y);
	acc_ldata[Z] = LPF2pApply_3(acc_rdata[Z] - ACC_OFFSET.Z);
	gyro_ldata[X] = LPF2pApply_4(gyro_rdata[X] - GYRO_OFFSET.X);
	gyro_ldata[Y] = LPF2pApply_5(gyro_rdata[Y] - GYRO_OFFSET.Y);
	gyro_ldata[Z] = LPF2pApply_6(gyro_rdata[Z] - GYRO_OFFSET.Z);

			if(!GYRO_OFFSET_OK)
			{
				static float	tempgx=0,tempgy=0,tempgz=0;
				static uint16_t cnt_g=0;
		// 		LED1_ON;
				if(cnt_g==0)
				{
					GYRO_OFFSET.X=0;
					GYRO_OFFSET.Y=0;
					GYRO_OFFSET.Z=0;
					tempgx = 0;
					tempgy = 0;
					tempgz = 0;
					cnt_g = 1;
					return;
				}
				tempgx+= gyro_rdata[X];
				tempgy+= gyro_rdata[Y];
				tempgz+= gyro_rdata[Z];
				if(cnt_g==200)
				{
					GYRO_OFFSET.X=tempgx/cnt_g;
					GYRO_OFFSET.Y=tempgy/cnt_g;
					GYRO_OFFSET.Z=tempgz/cnt_g;
					cnt_g = 0;
					GYRO_OFFSET_OK = 1;
					EE_SAVE_GYRO_OFFSET();//��������+++
					return;
				}
				cnt_g++;
			}
			if(!ACC_OFFSET_OK)
			{
				static float	tempax=0,tempay=0,tempaz=0;
				static uint16_t cnt_a=0;
		// 		LED1_ON;
				if(cnt_a==0)
				{
					ACC_OFFSET.X = 0;
					ACC_OFFSET.Y = 0;
					ACC_OFFSET.Z = 0;
					tempax = 0;
					tempay = 0;
					tempaz = 0;
					cnt_a = 1;
					return;
				}
				tempax+= acc_rdata[X];
				tempay+= acc_rdata[Y];
				tempaz+= acc_rdata[Z];
				if(cnt_a==300)
				{
					ACC_OFFSET.X=tempax/cnt_a;
					ACC_OFFSET.Y=tempay/cnt_a;
					ACC_OFFSET.Z=tempaz/cnt_a - CONSTANTS_ONE_G;
					cnt_a = 0;
					ACC_OFFSET_OK = 1;
					EE_SAVE_ACC_OFFSET();//��������+++
					return;
				}
				cnt_a++;
			}

	GYRO_I.Z += gyro_ldata[Z] * 0.0013 * 180.f / M_PI_F;
//	MPU6050_Dataanl();

//	ACC_X_BUF[filter_cnt] = MPU6050_ACC_LAST.X;//���»�����������
//	ACC_Y_BUF[filter_cnt] = MPU6050_ACC_LAST.Y;
//	ACC_Z_BUF[filter_cnt] = MPU6050_ACC_LAST.Z;
//	for(i=0;i<FILTER_NUM;i++)
//	{
//		temp1 += ACC_X_BUF[i];
//		temp2 += ACC_Y_BUF[i];
//		temp3 += ACC_Z_BUF[i];
//	}
//	ACC_AVG.X = temp1 / FILTER_NUM;
//	ACC_AVG.Y = temp2 / FILTER_NUM;
//	ACC_AVG.Z = temp3 / FILTER_NUM;
//	filter_cnt++;
//	if(filter_cnt==FILTER_NUM)	filter_cnt=0;

	//GYRO_I.X += MPU6050_GYRO_LAST.X*Gyro_G*0.001;//0.001��ʱ����,����prepare��ִ������
	//GYRO_I.Y += MPU6050_GYRO_LAST.Y*Gyro_G*0.001;
	//GYRO_I.Z += MPU6050_GYRO_LAST.Z*Gyro_G*0.0013;
	//MAG_P.X += mag_data[0];
}

void Get_Attitude(void)
{
	IMUupdate(gyro_ldata[X],gyro_ldata[Y],gyro_ldata[Z],
				acc_ldata[X],acc_ldata[Y],acc_ldata[Z]);	//*0.0174ת�ɻ���
}
////////////////////////////////////////////////////////////////////////////////
#define Kp 10.0f                        // proportional gain governs rate of convergence to accelerometer/magnetometer
#define Ki 0.008f                          // integral gain governs rate of convergence of gyroscope biases
#define halfT 0.001f                   // half the sample period�������ڵ�һ��

float q0 = 1, q1 = 0, q2 = 0, q3 = 0;    // quaternion elements representing the estimated orientation
float exInt = 0, eyInt = 0, ezInt = 0;    // scaled integral error
void IMUupdate(float gx, float gy, float gz, float ax, float ay, float az)
{
  float norm;
//  float hx, hy, hz, bx, bz;
  float vx, vy, vz;// wx, wy, wz;
  float ex, ey, ez;

  // �Ȱ���Щ�õõ���ֵ���
  float q0q0 = q0*q0;
  float q0q1 = q0*q1;
  float q0q2 = q0*q2;
//  float q0q3 = q0*q3;
  float q1q1 = q1*q1;
//  float q1q2 = q1*q2;
  float q1q3 = q1*q3;
  float q2q2 = q2*q2;
  float q2q3 = q2*q3;
  float q3q3 = q3*q3;

	if(ax*ay*az==0)
 		return;

  norm = sqrt(ax*ax + ay*ay + az*az);       //acc���ݹ�һ��
  ax = ax /norm;
  ay = ay / norm;
  az = az / norm;

  // estimated direction of gravity and flux (v and w)              �����������������/��Ǩ
  vx = 2*(q1q3 - q0q2);												//��Ԫ����xyz�ı�ʾ
  vy = 2*(q0q1 + q2q3);
  vz = q0q0 - q1q1 - q2q2 + q3q3 ;

  // error is sum of cross product between reference direction of fields and direction measured by sensors
  ex = (ay*vz - az*vy) ;                           					 //�������������õ���־������
  ey = (az*vx - ax*vz) ;
  ez = (ax*vy - ay*vx) ;

  exInt = exInt + ex * Ki;								  //�������л���
  eyInt = eyInt + ey * Ki;
  ezInt = ezInt + ez * Ki;

  // adjusted gyroscope measurements
  gx = gx + Kp*ex + exInt;					   							//�����PI�󲹳��������ǣ����������Ư��
  gy = gy + Kp*ey + eyInt;
  gz = gz + Kp*ez + ezInt;				   							//�����gz����û�й۲��߽��н��������Ư�ƣ����ֳ����ľ��ǻ����������Լ�

  // integrate quaternion rate and normalise						   //��Ԫ�ص�΢�ַ���
  q0 = q0 + (-q1*gx - q2*gy - q3*gz)*halfT;
  q1 = q1 + (q0*gx + q2*gz - q3*gy)*halfT;
  q2 = q2 + (q0*gy - q1*gz + q3*gx)*halfT;
  q3 = q3 + (q0*gz + q1*gy - q2*gx)*halfT;

  // normalise quaternion
  norm = sqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
  q0 = q0 / norm;
  q1 = q1 / norm;
  q2 = q2 / norm;
  q3 = q3 / norm;

//  float headingRadians  = atan2((double)mag_data[1] , (double)mag_data[0]);
////  if(headingRadians < 0)
////      headingRadians += 2*M_PI;
////  if(headingRadians > 2*M_PI)
////        headingRadians -= 2*M_PI;
//
//  int headingDegrees = headingRadians * 180/M_PI;
//
//  if(headingDegrees < 0)
//	  headingDegrees += 360;
//  if(headingDegrees > 360)
//	  headingDegrees -= 360;
  now_attitude.yaw = GYRO_I.Z; //atan2(2 * q1 * q2 + 2 * q0 * q3, -2 * q2*q2 - 2 * q3* q3 + 1)* 57.3; // yaw
  now_attitude.pitch = asin(-2 * q1 * q3 + 2 * q0* q2)* 57.3; // pitch
  now_attitude.roll = atan2(2 * q2 * q3 + 2 * q0 * q1, -2 * q1 * q1 - 2 * q2* q2 + 1)* 57.3; // roll
}

