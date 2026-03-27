#ifndef __OIL_H__
#define __OIL_H__

#include "main.h"


//#define DEFAULT_OIL_CONSUME_SPEED_10      10
//#define OIL_CONSUME_SPEED_VARIATION_10    (DEFAULT_OIL_CONSUME_SPEED_10*3/10)
//#define DEFAULT_OIL_CONSUME_SPEED_15      15
//#define OIL_CONSUME_SPEED_VARIATION_15    (DEFAULT_OIL_CONSUME_SPEED_15*3/10)
//#define DEFAULT_OIL_CONSUME_SPEED_21      21
//#define OIL_CONSUME_SPEED_VARIATION_21    (DEFAULT_OIL_CONSUME_SPEED_21*3/10)
//#define DEFAULT_OIL_CONSUME_SPEED_40      40
//#define OIL_CONSUME_SPEED_VARIATION_40    (DEFAULT_OIL_CONSUME_SPEED_40*3/10)

typedef enum
{
	DEFAULT_OIL_CONSUME_SPEED_8 = 8,
	OIL_CONSUME_SPEED_VARIATION_8 = (DEFAULT_OIL_CONSUME_SPEED_8*3/10),
	DEFAULT_OIL_CONSUME_SPEED_10 = 10,
	OIL_CONSUME_SPEED_VARIATION_10 = (DEFAULT_OIL_CONSUME_SPEED_10*3/10),
	DEFAULT_OIL_CONSUME_SPEED_15 = 15,
	OIL_CONSUME_SPEED_VARIATION_15 = (DEFAULT_OIL_CONSUME_SPEED_15*3/10),
	DEFAULT_OIL_CONSUME_SPEED_21 = 21,
	OIL_CONSUME_SPEED_VARIATION_21 = (DEFAULT_OIL_CONSUME_SPEED_21*3/10),
	DEFAULT_OIL_CONSUME_SPEED_40 = 40,
	OIL_CONSUME_SPEED_VARIATION_40 = (DEFAULT_OIL_CONSUME_SPEED_40*3/10),
}oilSpeedType_t;

typedef enum
{
	OIL_150ml,
	OIL_180ml,
	OIL_200ml,
	OIL_240ml,
	OIL_250ml,
	OIL_500ml,
}oilVolumeType_t;


typedef struct {
	
	struct
	{
		uint32_t volumeType:	          3;   //精油瓶容量类型
		uint32_t armalFlag:	            1;   //精油告警 
		uint32_t curretVolumePercent:	  8; //当前容量百分比
		uint32_t curretVolumePercentOld: 8;
		uint32_t Reserverd:	             12; //最高位
	};
	uint16_t surplusDay;  //精油剩余天数
	uint16_t surplusDayOld; 
	uint16_t totalVolume;  //精油总量
	uint16_t curretVolume; //精油当前量
	uint16_t curretVolumeOld;
	uint16_t defaultConsumeSpeed;  //默认精油消耗速度，单位“0.1mL/h”
	uint16_t actualConsumeSpeed;  //实际精油消耗速度，单位“0.1mL/h”
	uint16_t actualConsumeSpeedOld;
	uint16_t actualConsumeSpeedMax;
	uint16_t actualConsumeSpeedMin;
	uint16_t warnThreshold;  //精油告警阈值
	uint32_t curretUsedTime;    //精油当前已使用时间，单位“S”
}oil_t;

extern oil_t oil;

void oil_init(oilVolumeType_t oilVolumeType , oilSpeedType_t oilSpeedType);
void oil_reset(void);
void oil_surplusDay_task(void);
void oil_currentVolume_calculate_task(void);

#endif
