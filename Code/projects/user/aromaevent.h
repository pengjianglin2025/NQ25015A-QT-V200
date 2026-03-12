#ifndef __AROMAEVENT_H__
#define __AROMAEVENT_H__

#include "main.h"

enum
{
	WORK_TIME_MIN = 5,                       //工作时间——最小值
	WORK_TIME_MAX = 30,                      //工作时间——最大值
	WORK_TIME_DEFAULT = 30,                  //默认工作时间
	WORK_OVERTIME = (WORK_TIME_MAX+10),      //判断气泵工作超时时间
	//WORK_TIME_STEP = 5,
	PAUSE_TIME_MIN = 90,                       //暂停时间——最小值
	PAUSE_TIME_MAX = 500,                      //暂停时间——最大值
	PAUSE_TIME_DEFAULT = 90,                  //默认暂停时间
	//PAUSE_TIME_STEP = 5,
	PAUSE_OVERTIME = (PAUSE_TIME_MAX+10),      //判断气泵暂停超时时间
	SET_TIME_STEP = 5,                         //app设置时间的步进值
};


typedef union
{
	uint8_t BYTE;
	struct
	{
		uint8_t MON_EN:	          1; 
		uint8_t TUE_EN:	          1; 
		uint8_t WED_EN:	          1; 
		uint8_t THU_EN:	          1; 
		uint8_t FRI_EN:	          1; 
		uint8_t SAT_EN:	          1; 
		uint8_t SUN_EN:	          1; 
		uint8_t B7:	          1; //最高位
	};
}weekEn_t;

enum
{
	EVENT_MON,
	EVENT_TUE,
	EVENT_WED,
	EVENT_THU,
	EVENT_FRI,
	EVENT_SAT,
	EVENT_SUN,
};

#define  EVENT_WEEK_MAX   EVENT_SUN
#define  EVENT_WEEK_MIN   EVENT_MON

typedef struct {
	uint8_t startTimeHour;        //工作开始时间小时
	uint8_t startTimeMinutes;     //工作开始时间分钟
	uint8_t stopTimeHour;         //工作结束时间小时
	uint8_t stopTimeMinutes;      //工作结束时间分钟
  uint16_t workTime;            //工作时间
  uint16_t pauseTime;           //暂停时间
	uint8_t workPer;             //工作浓度
	weekEn_t weekEn;             //工作日 第0位代表星期一....依次第6位代表星期天
	bool en;             //事件使能
	uint32_t id;
}eventParameter_t;
extern eventParameter_t runEvent[5];

typedef struct
{
	bool status;    //事件状态：在定时范围内（有事件），不在定时范围内（无事件）
	uint8_t num;    //当前事件的序号，无事件时值为0
	bool dataComparisonResult;
}event_t;
extern event_t event;
//extern uint32_t currentClock_DEC.totalSec;    //实时时间 单位：S   
extern const eventParameter_t initEvent[5];

void event_date_init(void);
void event_Task(void);
void Calculate_TotalRunTime_Aweek(void);

#endif

