#ifndef __aroma_H__
#define __aroma_H__

#include "main.h"

#define AIRPUMP_DRIVE_MODE_DIRECT_GPIO      0U
#define AIRPUMP_DRIVE_MODE_PWM_SOFTSTART    1U

/* 气泵驱动方式选择：
 * AIRPUMP_DRIVE_MODE_DIRECT_GPIO: 保留原来的GPIO直接拉高/拉低控制
 * AIRPUMP_DRIVE_MODE_PWM_SOFTSTART: 启动时先软启动，再切回GPIO直驱
 */
#ifndef AIRPUMP_DRIVE_MODE
#define AIRPUMP_DRIVE_MODE AIRPUMP_DRIVE_MODE_PWM_SOFTSTART
#endif

typedef  enum	
{
	S_NORMAL,
	S_ERROR,
}operationCountS_t;

typedef  enum	
{
	TIMEING_OFF,
	TIMEING_1H,
	TIMEING_2H,
	TIMEING_4H,
	TIMEING_6H,
	TIMEING_8H,
	TIMEING_12H,
	TIMEING_CUSTOMIZE,    //自定义定时；
}timeing_t;

typedef  enum	
{
	SIMPLE,       //简单模式
	PROFESSIONAL, //专业模式
}parameterMode_t;

typedef  enum	
{
	STOP,       //停止状态
	WORK,      //工作状态
	PAUSE,      //暂停状态
}aromaWorkStatus_t;

typedef struct
{
	bool en;
	parameterMode_t parameterMode;                 //  参数运行模式
	aromaWorkStatus_t  workStatus,workStatusOld;   //香氛工作运行状态：停止，工作，暂停
	uint16_t currentWorkStatusRemainingTime;       //当前工作状态剩余时间，单位：S
	uint8_t concentration,concentrationOld;        //浓度（香氛），具体指香氛工作多少时间，停止多少时间
	bool timeingParameterUpdate;                   //定时参数更新标志 
	timeing_t timeingGear,timeingGearOld;           //香氛定时档位，“0”为关闭定时，24小时运行
	uint16_t timeLeft;                             //定时工作模式下的剩余时间（单位 min）
	uint16_t simpleModeCnt;                        //简单模式下，气泵运行浓度计时
	uint16_t simpleModeWorkSec,simpleModePauseSec; // 简单模式下，工作时间和暂停时间（单位：S）
	uint16_t currentWorkSec,currentPauseSec;        //当前工作模式的工作，暂停时间
	
	uint16_t currentEventWorkTimeInTotal;
	uint16_t currentEventPauseTimeInTotal;
	uint16_t currentEventWorkTimeInTotalOld;
	uint16_t currentEventPauseTimeInTotalOld;
//	uint32_t currentPauseTime;
	uint32_t runTimeInTotal;
	uint32_t runTimeInTotal_Aweek;           //精油一周消耗时间，单位“S”
	uint32_t runTimeInTotal_AweekOld;
	uint32_t startTime; 
}aroma_t;
extern aroma_t aroma;

typedef struct
{
	bool en; 
	bool SW; 
	bool SWOld;
	uint32_t cnt;	
	uint8_t status; 
	uint8_t statusOld;
}airpump_t;
extern airpump_t airpump;

typedef struct
{
	bool en; 
	bool SW; 
	uint8_t status; 
	uint8_t statusOld;
}fan_t;
extern fan_t fan;

typedef struct
{
	bool En; 
	uint8_t Up; 
	uint8_t Down; 
}moto_t;
extern moto_t moto;

void WorkInit(void);
void aroma_work_task(void);
void airpump_gpio_out(void);
void airpump_pwm_force_off(void);
void airpump_force_off(void);
aromaWorkStatus_t aroma_run_status(uint8_t num);
operationCountS_t airpump_operation_count_status(uint16_t workTime, uint16_t pauseTime);
void Work_Sleep_Config(void);


#endif


