#pragma once
#include "stdint.h"


//#define Debug  //����������������

//����ת
#define SLOW_THRO 200
//ң������
#define APP_YAW_DB	 70 //dead band
#define APP_PR_DB		 50
//����ɻ������б�Ƕ�
#define  Angle_Max  30.0
#define  YAW_RATE_MAX  180.0f/M_PI_F		//deg/s  
//������̬�����������ֿ�����ƫ�Ƶȴ����ĳ�ʼ��ƽ��
//#define  Rool_error_init   7      //����ɻ���ɳ���ƫ��Rool_error_init�����������޸�;����ƫ��Rool_error_init�����������޸�
//#define  Pitch_error_init  -5      //����ɻ���ɳ�ǰƫ��Pitch_error_init�����������޸�;����ƫ��Pitch_error_init�����������޸�
//���߲���
#define LAND_SPEED						1.2f		//m/s^2
#define ALT_VEL_MAX 					4.0f

enum {CLIMB_RATE=0,MANUAL,LANDING};
extern uint8_t altCtrlMode;
extern float hoverThrust;
extern uint8_t zIntReset;
extern uint8_t offLandFlag;
extern float altLand;
extern uint8_t isAltLimit;
extern float thrustZSp,thrustZInt;

// PID�ṹ��
typedef struct
{
    float P;
    float I;
    float D;
    float Desired;
    float Error;
    float PreError;
    float PrePreError;
    float Increment;
    float Integ;
		float iLimit;
    float Deriv;
    float Output;
 
}PID_Typedef;


//д��Flash�����ṹ��
typedef struct
{
  u16 WriteBuf[10];       //д��flash����ʱ����
  u16 ReadBuf[10];        //��ȡFlash����ʱ����
  
}Parameter_Typedef;


void Controler(void);
void PID_INIT(void);
void PID_Calculate(void);
float dbScaleLinear(float x, float x_end, float deadband);

void CtrlAttiAng(void);
void CtrlAttiRate(void);
void CtrlAlti(void);
void CtrlAltiVel(void);
void CtrlMotor(void);
void CtrlTest(void);
void CtrlAttiRateNew(void);
void CtrlAttiNew(void);
float estimateHoverThru(void);

void SetHeadFree(uint8_t on);

extern u16 PIDWriteBuf[3];//д��flash����ʱ���֣���NRF24L01_RXDATA[i]��ֵ

extern PID_Typedef pitch_angle_PID;	  //pitch�ǶȻ���PID
extern PID_Typedef pitch_rate_PID;		//pitch�����ʻ���PID

extern PID_Typedef roll_angle_PID;    //roll�ǶȻ���PID
extern PID_Typedef roll_rate_PID;     //roll�����ʻ���PID

extern PID_Typedef yaw_angle_PID;     //yaw�ǶȻ���PID
extern PID_Typedef yaw_rate_PID;      //yaw�Ľ����ʻ���PID

extern PID_Typedef	alt_PID;
extern PID_Typedef alt_vel_PID;


extern float gyroxGloble;
extern float gyroyGloble;

extern int16_t Motor[4];   //������PWM���飬�ֱ��ӦM1-M4
extern volatile unsigned char motorLock;


