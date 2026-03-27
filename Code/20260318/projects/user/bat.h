#ifndef _BAT_H
#define _BAT_H

#include "main.h"

enum
	{
		VOLTAGE_RANGE2_VALUE = 3900,  //3.9
		VOLTAGE_RANGE1_VALUE = 3600,  //3.6
		VOLTAGE_LOW_VALUE    = 3400,  //3.4
		VOLTAGE_STOP_VALUE   = 3200,   //3.2
	};
	
enum
	{
		CHARGE_OFF = 0,
		CHARGE_ON,
	};
	
typedef enum
{
	VOLTAGE_RANGE1 = 0,
	VOLTAGE_RANGE2,
	VOLTAGE_LOW,
	VOLTAGE_STOP,
}volRange_t;	

typedef enum
{
	BAT_DISCHARGE = 0,
	BAT_CHARGE ,
}batS_t;

typedef struct 
{
	batS_t status;
	volRange_t volRange;
	uint16_t volage;
	uint32_t fullCnt;
	bool chargeFull;
//	bool chargeAwakenFlag; //充电唤醒标志，（实现从没充电状态到充电状态时自动开机）
}bat_t;
extern bat_t bat;

void BAT_Init(void);
void RCC_ADC_Configuration(void);
void BAT_Task(void);
void Bat_Monitor(void);
void BAT_Sleep_Config(void);

#endif
