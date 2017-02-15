/*
 * HAL.c
 *
 *  Created on: 2017��1��3��
 *      Author: 50430
 */

#pragma once

#include "stdint.h"
#include "sensor.h"

typedef struct gyro_s {
    sensorGyroInitFuncPtr init;                             // initialize function
    sensorReadFuncPtr read;                                 // read 3 axis data function
    sensorReadFuncPtr temperature;                          // read temperature if available
    sensorIsDataReadyFuncPtr isDataReady;                   // check if sensor has new readings
    float scale;                                            // scalefactor
} gyro_t;

typedef struct acc_s {
	sensorAccInitFuncPtr init;                             // initialize function
    sensorReadFuncPtr read;                                 // read 3 axis data function
    uint16_t acc_1G;
} acc_t;

typedef struct mag_s {
    sensorInitFuncPtr init;                                 // initialize function
    sensorReadFuncPtr read;                                 // read 3 axis data function
} mag_t;

typedef struct baro_s {
    uint16_t ut_delay;
    uint16_t up_delay;
    baroOpFuncPtr start_ut;
    baroOpFuncPtr get_ut;
    baroOpFuncPtr start_up;
    baroOpFuncPtr get_up;
    baroCalculateFuncPtr calculate;
} baro_t;

typedef struct motor_s {
	volatile uint32_t *value[4];
}motor_t;

typedef struct attitude_s {
	float pitch;
	float roll;
	float yaw;
}attitude_t;


enum rc_e{
	rc_rol_num		= 0,
	rc_pit_num 		,
	rc_thr_num		,
	rc_yaw_num		,
	rc_aux1_num		,
	rc_aux2_num		,
	rc_push_num		,
	rc_ball_num
};

//typedef void (*rcUpdateFunPtr)(void);
typedef struct rc_s {
	float thr;
	float rol;
	float pit;
	float yaw;
	int16_t value[8];		//ͨ��ֵ������Ҫ����ȥ�ɻ���ֵ����1000~2000��
	int8_t	 direct[8];		//ͨ�����򡣣�-1������1������
	int16_t  trim[4];		//ͨ��΢����������
//	rcUpdateFunPtr rc_update;

}rc_t;

/*
 * \brief ��ع���ṹ��
 *
 * \para  raw_data : 12λad��������ԭʼ����
 *        voltage  : ת�����������ѹֵ
 *        vol_alarm: ������ѹ
 *        scale    : raw_dataת��Ϊvoltageʱ�ı���ϵ��
 * \addition ��ʽ: 	voltage = ((float)raw_data/4096)*3.33*scale;
 * */
typedef struct battery_s{
	uint16_t raw_data;
	uint16_t voltage;
	uint16_t vol_alarm;
	float scale;
}battery_t;

enum model_s{
	ON,
	OFF,
	SINGLE_FLASH,
	DOUBLE_FLASH,
	SINGLE_FLASH_500MS,
};
typedef struct led_s{
	uint8_t state;	/*0 for off,1 for on*/
	enum model_s model;
}led_t;

extern acc_t acc;
extern gyro_t gyro;
extern mag_t mag;
extern baro_t baro;
extern motor_t motor;
extern rc_t rc;
extern battery_t battery;

void motor_init();
void motor_out(int16_t m1,int16_t m2,int16_t m3,int16_t m4);
extern void detectAcc();
extern void detectGyro();
extern void detectMag();
extern void detectBaro();
extern uint32_t recalculateBarometerTotal(uint8_t baroSampleCount, uint32_t pressureTotal, int32_t newPressureReading);
