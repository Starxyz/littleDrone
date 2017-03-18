/*    
      ____                      _____                  +---+
     / ___\                     / __ \                 | R |
    / /                        / /_/ /                 +---+
   / /   ________  ____  ___  / ____/___  ____  __   __
  / /  / ___/ __ `/_  / / _ \/ /   / __ \/ _  \/ /  / /
 / /__/ /  / /_/ / / /_/  __/ /   / /_/ / / / / /__/ /
 \___/_/   \__,_/ /___/\___/_/    \___ /_/ /_/____  /
                                                 / /
                                            ____/ /
                                           /_____/
*/
/* Control.c file
��д�ߣ�С��  (Camel) ���顢Nieyong
����E-mail��375836945@qq.com
���뻷����MDK-Lite  Version: 4.23
����ʱ��: 2014-01-28
���ܣ�
1.PID������ʼ��
2.���ƺ���

------------------------------------
*/
//#include <stm32f1xx.h>
#include "control.h"
#include "math.h"
#include "mpu6050.h"
#include "imu.h"
#include "led.h"
#include "stmflash.h"
#include "stdio.h"
#include "ANO_DT.h"

extern uint32_t micros(void);

uint8_t offLandFlag=0;


volatile unsigned char motorLock=1;

int16_t Motor[4]={0};   //������PWM���飬�ֱ��ӦM1-M4
float rollSp =0,pitchSp =0;		//���ݶ����������¼���õ�������roll pitch
float Thro=0,Roll=0,Pitch=0,Yaw=0;


//----PID�ṹ��ʵ����----
PID_Typedef pitch_angle_PID;	//pitch�ǶȻ���PID
PID_Typedef pitch_rate_PID;		//pitch�����ʻ���PID

PID_Typedef roll_angle_PID;   //roll�ǶȻ���PID
PID_Typedef roll_rate_PID;    //roll�����ʻ���PID

PID_Typedef yaw_angle_PID;    //yaw�ǶȻ���PID
PID_Typedef yaw_rate_PID;     //yaw�����ʻ���PID

PID_Typedef	alt_PID;
PID_Typedef alt_vel_PID;

float gyroxGloble = 0;
float gyroyGloble = 0;


S_FLOAT_XYZ DIF_ACC;		//ʵ��ȥ�������ļ��ٶ�
S_FLOAT_XYZ EXP_ANGLE;	//�����Ƕ�
S_FLOAT_XYZ DIF_ANGLE;	//ʵ�����������ĽǶ�

uint32_t ctrlPrd=0;
uint8_t headFreeMode=0;
float headHold=0;

union _dat{
	uint16_t full;
	uint8_t  byte[2];
}d_temp;

//��������PID_Postion_Cal()
//������λ��ʽPID
static void PID_Postion_Cal(PID_Typedef * PID,float target,float measure,int32_t dertT)
{
	float termI=0;
	float dt= dertT/1000000.0;

	//���=����ֵ-����ֵ
	PID->Error=target-measure;

	PID->Deriv= (PID->Error-PID->PreError)/dt;

	PID->Output=(PID->P * PID->Error) + (PID->I * PID->Integ) + (PID->D * PID->Deriv);    //PID:��������+���ֻ���+΢�ֻ���

	PID->PreError=PID->Error;
	//�����ڽǶȻ��ͽ��ٶȻ���

//	if(FLY_ENABLE && offLandFlag){
	if(motorLock == 0){
		if(fabs(PID->Output) < Thro )		              //�����Ż���ʱ������
		{
			termI=(PID->Integ) + (PID->Error) * dt;     //���ֻ���
			if(termI > - PID->iLimit && termI < PID->iLimit && PID->Output > - PID->iLimit && PID->Output < PID->iLimit)       //��-300~300ʱ�Ž��л��ֻ���
					PID->Integ=termI;
		}
	}else{
		PID->Integ= 0;
	}
}


void SetHeadFree(uint8_t on)
{
	if(on==1){
		headHold=imu.yaw;
		headFreeMode=1;
	}else{
		headFreeMode=0;
	}
}


//��������CtrlAttiAng(void)
//�������Է�������̬���ƣ�pitch��roll��yaw�������У�����PID�еĽǶȻ�����
void CtrlAttiAng(void)
{
		static uint32_t tPrev=0;
		float angTarget[3]={0};
		float dt=0,t=0;
		t=micros();
		dt=(tPrev>0)?(t-tPrev):0;
		tPrev=t;
		
		//pid�⻷��������  [first]/2+1 = 11
		d_temp.full = (uint16_t)dt;
		str0[20] = d_temp.byte[1];
		str0[21] = d_temp.byte[0];

		altCtrlMode = MANUAL;

		if(altCtrlMode==MANUAL){
			angTarget[ROLL]=(float)(rc.rol);
			angTarget[PITCH]=(float)(rc.pit);
		}else{
			angTarget[ROLL]=rollSp;
			angTarget[PITCH]=pitchSp;
		}

//		if(headFreeMode){
//			#ifdef YAW_CORRECT
//        float radDiff = -(imu.yaw - headHold) * M_PI_F / 180.0f;
//			#else
//				float radDiff = (imu.yaw - headHold) * M_PI_F / 180.0f;
//			#endif
//        float cosDiff = cosf(radDiff);
//        float sinDiff = sinf(radDiff);
//        float tarPitFree = angTarget[PITCH] * cosDiff + angTarget[ROLL] * sinDiff;
//        angTarget[ROLL] = angTarget[ROLL] * cosDiff - angTarget[PITCH] * sinDiff;
//        angTarget[PITCH] = tarPitFree;
//		}
 
		PID_Postion_Cal(&pitch_angle_PID,angTarget[PITCH],imu.pitch,dt);
		PID_Postion_Cal(&roll_angle_PID,angTarget[ROLL],imu.roll,dt);	 
}




//��������CtrlAttiRate(void)
//�������Է�������̬���ƣ�pitch��roll��yaw�������У�����PID�еĽ��ٶȻ�����
void CtrlAttiRate(void)
{
 	float yawRateTarget=0;
	static uint32_t tPrev=0;

	float dt=0,t=0;
	t=micros();
	dt=(tPrev>0)?(t-tPrev):0;
	tPrev=t;
		
	//pid�ڻ��������� [first]/2+1 = 10
	d_temp.full = (uint16_t)dt;
	str0[18] = d_temp.byte[1];
	str0[19] = d_temp.byte[0];

	yawRateTarget=-(float)rc.yaw;
	
	//ע�⣬ԭ����pid��������Ӧ���� adֵ,��ת֮
	PID_Postion_Cal(&pitch_rate_PID,pitch_angle_PID.Output,imu.gyro[PITCH]*180.0f/M_PI_F,dt);
	PID_Postion_Cal(&roll_rate_PID,roll_angle_PID.Output,imu.gyro[ROLL]*180.0f/M_PI_F,dt);//gyroxGloble
	PID_Postion_Cal(&yaw_rate_PID,yawRateTarget,imu.gyro[YAW]*180.0f/M_PI_F,dt);//DMP_DATA.GYROz


	Pitch = pitch_rate_PID.Output;
	Roll  = roll_rate_PID.Output;
	Yaw   = yaw_rate_PID.Output; 
}

//cut deadband, move linear
float dbScaleLinear(float x, float x_end, float deadband)
{
	if (x > deadband) {
		return (x - deadband) / (x_end - deadband);

	} else if (x < -deadband) {
		return (x + deadband) / (x_end - deadband);

	} else {
		return 0.0f;
	}
}


float thrInit;

#define ALT_FEED_FORWARD  		0.5f
#define THR_MAX								1.0f		//max thrust
#define TILT_MAX 					(Angle_Max * M_PI_F / 180.0 )
const float ALT_CTRL_Z_DB = 0.1f;	//
float spZMoveRate;

uint8_t altCtrlMode;					//normal=0  CLIMB rate, normal .  tobe tested
float hoverThrust=0;
uint8_t zIntReset=1;	//integral reset at first . when change manual mode to climb rate mode
float thrustZInt=0, thrustZSp=0;
float thrustXYSp[2]={0,0};	//roll pitch
uint8_t recAltFlag=0;
float holdAlt=0;
uint8_t satZ=0,satXY=0;	//�Ƿ������


#define ALT_LIMIT							2.0f		//�޸� 3.5
uint8_t isAltLimit=0;
float altLand;



//��������estimateHoverThru()
//���룺��
//���: Ԥ���õ�����ͣ���Ż�׼ֵ
//������Ԥ����ͣ���Ż�׼ֵ��ֱ��Ӱ�쵽�÷�������z����ͣ
//��ͣ����ֵ��������У���ص�ѹ
//Get a estimated value for hold throttle.It will have a direct affection on hover
//Battery voltage
//float estimateHoverThru(void){
//	float hoverHru = -0.55f;
//
//	//��ص�ѹ���
//	Battery.BatteryAD  = GetBatteryAD();
//	Battery.BatteryVal = Battery.Bat_K * (Battery.BatteryAD/4096.0) * Battery.ADRef;//ʵ�ʵ�ѹ ֵ����
//
//	if(Battery.BatteryVal > 4.05){
//		hoverHru = -0.25f;
//	}else if(Battery.BatteryVal > 3.90){
//		hoverHru = -0.40f;
//	}else if(Battery.BatteryVal > 3.80){
//		hoverHru = -0.45f;
//	}else if(Battery.BatteryVal > 3.70){
//		hoverHru = -0.50f;
//	}else{
//		hoverHru = -0.55f;
//	}
//
//
////	if(Battery.BatteryVal > 4.05){
////		hoverHru = -0.05f;
////	}else if(Battery.BatteryVal > 3.90){
////		hoverHru = -0.10f;
////	}else if(Battery.BatteryVal > 3.80){
////		hoverHru = -0.15f;
////	}else if(Battery.BatteryVal > 3.70){
////		hoverHru = -0.20f;
////	}else{
////		hoverHru = -0.25f;
////	}
//
//	return hoverHru;
//}


//��������estimateMinThru()
//���룺��
//���: Ԥ���õ�����С����ֵ
//������Ԥ����С����ֵ�����ݻ��ء���ص�������
//���Ź�С���½��ٶȹ���ʱ������ʧ�⣬��������½�ʱ����ζ�������������fuzzy control ��������Сʱ�ø������̬����
//��������У���ص�ѹ
//float estimateMinThru(void){
//	float minThru = -0.55f;
//
//	//��ص�ѹ���
//	Battery.BatteryAD  = GetBatteryAD();
//	Battery.BatteryVal = Battery.Bat_K * (Battery.BatteryAD/4096.0) * Battery.ADRef;//ʵ�ʵ�ѹ ֵ����
//
//	if(Battery.BatteryVal > 4.05){
//		minThru = -0.30f;
//	}else if(Battery.BatteryVal > 3.90){
//		minThru = -0.40f;
//	}else{
//		minThru = -0.55f;
//	}
//
//	return minThru;
//}

//��������CtrlAlti()
//���룺��
//���: ���ս�������ȫ�ֱ���thrustZSp
//���������Ƹ߶ȣ�Ҳ���Ǹ߶���ͣ���ƺ���
//only in climb rate mode and landind mode. now we don't work on manual mode
//void CtrlAlti(void)
//{
//	float manThr=0,alt=0,velZ=0;
//	float altSp=0;
//	float posZVelSp=0;
//	float altSpOffset,altSpOffsetMax=0;
//	float dt=0,t=0;
//	static float tPrev=0,velZPrev=0;
//	float posZErr=0,velZErr=0,valZErrD=0;
//	float thrustXYSpLen=0,thrustSpLen=0;
//	float thrustXYMax=0;
//	float minThrust;
//
//	//get dt
//	//��֤dt���㲻�ܱ���ϣ����ָ��£�����dt���󣬻��ֱ�����
//	if(tPrev==0){
//			tPrev=micros();
//			return;
//	}else{
//			t=micros();
//			dt=(t-tPrev) /1000000.0f;
//			tPrev=t;
//	}
//
//	//only in climb rate mode and landind mode. now we don't work on manual mode
//	//�ֶ�ģʽ��ʹ�øø߶ȿ����㷨
//	if(MANUAL == altCtrlMode || !FLY_ENABLE){
//		return;
//	}
//
//	//--------------pos z ctrol---------------//
//	//get current alt
//	alt=-nav.z;
//	//get desired move rate from stick
//	manThr=RC_DATA.THROTTLE / 1000.0f;
//	spZMoveRate= -dbScaleLinear(manThr-0.5f,0.5f,ALT_CTRL_Z_DB);	// scale to -1~1 . NED frame
//	spZMoveRate = spZMoveRate * ALT_VEL_MAX;	// scale to vel min max
//
//	//get alt setpoint in CLIMB rate mode
//	altSp 	=-nav.z;						//only alt is not in ned frame.
//	altSp  -= spZMoveRate * dt;
//	//limit alt setpoint
//	altSpOffsetMax=ALT_VEL_MAX / alt_PID.P * 2.0f;
//	altSpOffset = altSp-alt;
//	if( altSpOffset > altSpOffsetMax){
//		altSp=alt +  altSpOffsetMax;
//	}else if( altSpOffset < -altSpOffsetMax){
//		altSp=alt - altSpOffsetMax;
//	}
//
//	//�޸�
//	if(isAltLimit)
//	{
//		if(altSp - altLand > ALT_LIMIT)
//		{
//				altSp=altLand+ALT_LIMIT;
//				spZMoveRate=0;
//		}
//	}
//
//	// pid and feedforward control . in ned frame
//	posZErr= -(altSp - alt);
//	posZVelSp = posZErr * alt_PID.P + spZMoveRate * ALT_FEED_FORWARD;
//	//consider landing mode
//	if(altCtrlMode==LANDING)
//		posZVelSp = LAND_SPEED;
//
//	//��ȡһ��Ԥ����Z����ͣ��׼ֵ����������е�ص�ѹ
//	//get hold throttle. give it a estimated value
//	if(zIntReset){
//		thrustZInt = estimateHoverThru();
//		zIntReset = 0;
//	}
//
//	velZ=nav.vz;
//	velZErr = posZVelSp - velZ;
//	valZErrD = (spZMoveRate - velZ) * alt_PID.P - (velZ - velZPrev) / dt;	//spZMoveRate is from manual stick vel control
//	velZPrev=velZ;
//
//	thrustZSp= velZErr * alt_vel_PID.P + valZErrD * alt_vel_PID.D + thrustZInt;	//in ned frame. thrustZInt contains hover thrust
//
//	//������С�½�����
//	minThrust = estimateMinThru();
//	if(altCtrlMode!=LANDING){
//		if (-thrustZSp < minThrust){
//			thrustZSp = -minThrust;
//		}
//	}
//
//	//�붯���������	testing
//	satXY=0;
//	satZ=0;
//	thrustXYSp[0]= sinf(RC_DATA.ROOL * M_PI_F /180.0f) ;//Ŀ��Ƕ�ת���ٶ�
//	thrustXYSp[1]= sinf(RC_DATA.PITCH * M_PI_F /180.0f) ; 	//��һ��
//	thrustXYSpLen= sqrtf(thrustXYSp[0] * thrustXYSp[0] + thrustXYSp[1] * thrustXYSp[1]);
//	//limit tilt max
//	if(thrustXYSpLen >0.01f )
//	{
//		thrustXYMax=-thrustZSp * tanf(TILT_MAX);
//		if(thrustXYSpLen > thrustXYMax)
//		{
//				float k=thrustXYMax / thrustXYSpLen;
//				thrustXYSp[1] *= k;
//				thrustXYSp[0] *= k;
//				satXY=1;
//				thrustXYSpLen= sqrtf(thrustXYSp[0] * thrustXYSp[0] + thrustXYSp[1] * thrustXYSp[1]);
//		}
//
//	}
//	//limit max thrust!!
//	thrustSpLen=sqrtf(thrustXYSpLen * thrustXYSpLen + thrustZSp * thrustZSp);
//	if(thrustSpLen > THR_MAX)
//	{
//			if(thrustZSp < 0.0f)	//going up
//			{
//						if (-thrustZSp > THR_MAX)
//						{
//								/* thrust Z component is too large, limit it */
//								thrustXYSp[0] = 0.0f;
//								thrustXYSp[1] = 0.0f;
//								thrustZSp = -THR_MAX;
//								satXY = 1;
//								satZ = 1;
//
//							}
//							else {
//								float k = 0;
//								/* preserve thrust Z component and lower XY, keeping altitude is more important than position */
//								thrustXYMax = sqrtf(THR_MAX * THR_MAX- thrustZSp * thrustZSp);
//								k=thrustXYMax / thrustXYSpLen;
//								thrustXYSp[1] *=k;
//								thrustXYSp[0] *= k;
//								satXY=1;
//							}
//			}
//			else {		//going down
//							/* Z component is negative, going down, simply limit thrust vector */
//							float k = THR_MAX / thrustSpLen;
//							thrustZSp *= k;
//							thrustXYSp[1] *=k;
//							thrustXYSp[0] *= k;
//							satXY = 1;
//							satZ = 1;
//						}
//
//	}
//	rollSp= asinf(thrustXYSp[0]) * 180.0f /M_PI_F;
//	pitchSp = asinf(thrustXYSp[1]) * 180.0f /M_PI_F;
//
//
//	// if saturation ,don't integral
//	if(!satZ )//&& fabs(thrustZSp)<THR_MAX
//	{
//			thrustZInt += velZErr * alt_vel_PID.I * dt;
//			if (thrustZInt > 0.0f)
//							thrustZInt = 0.0f;
//	}
//}
//

//��������CtrlMotor()
//���룺��
//���: 4�������PWM���
//���������PWM�����Ƶ�����������ᱻ��ѭ����100Hzѭ������
#define RANGE 200
void CtrlMotor(void)
{
		float  cosTilt = imu.accb[2] / CONSTANTS_ONE_G;

		altCtrlMode = MANUAL;
		Thro = rc.thr * 0.8;
		if(Thro>800)Thro = 800;

//		if(altCtrlMode==MANUAL)
//		{
//			DIF_ACC.Z =  imu.accb[2] - CONSTANTS_ONE_G;
//			Thro = rc.thr;
//			cosTilt=imu.DCMgb[2][2];
//			Thro=Thro/cosTilt;
//		}else{
//			Thro=(-thrustZSp) * 1000;// /imu.DCMgb[2][2];  //��ǲ�����Ч��������ʱ����
//			if(Thro>1000)
//				Thro=1000;
//		}



		if(Pitch>RANGE)Pitch = RANGE;
		else if(Pitch<-RANGE)Pitch = -RANGE;
		if(Roll>RANGE)Roll = RANGE;
		else if(Roll<-RANGE)Roll = -RANGE;


		//�����ֵ�ںϵ��ĸ����
		Motor[3] = (int16_t)(Thro - Pitch - Roll - Yaw );    //M3
		Motor[1] = (int16_t)(Thro + Pitch + Roll - Yaw );    //M1
		Motor[2] = (int16_t)(Thro - Pitch + Roll + Yaw );    //M4
		Motor[0] = (int16_t)(Thro + Pitch - Roll + Yaw );    //M2

//   	if((FLY_ENABLE!=0))
	if(motorLock == 0 && rc.thr >80)
   			motor_out(Motor[0],Motor[1],Motor[2],Motor[3]);
	else
			motor_out(0,0,0,0);
}
