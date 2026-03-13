#ifndef _CLOCK_H
#define _CLOCK_H

#include "main.h"
#include "n32wb03x_rtc.h"
#include "ns_log.h"


extern const uint32_t hourToSec[24];
extern const uint16_t minToSec[60];
extern const uint16_t hourToMin[24];

//HBYTE 是 4-BIT, BYTE 是 8-BIT 数据型态，WORD 为 16 BIT， EWORD 为 24 BIT，DWORD 为 32 BIT。
typedef union
{
	uint8_t BYTE;
	struct
	{
		uint8_t L:	          4; 
		uint8_t H:	          4; //最高位
	};
}HBYTE_t;

typedef enum
{
	CLOCK_SUN,
	CLOCK_MON,
	CLOCK_TUE,
	CLOCK_WED,
	CLOCK_THU,
	CLOCK_FRI,
	CLOCK_SAT,
}clockWeek_t;

#define  CLOCK_WEEK_MAX   CLOCK_SAT
#define  CLOCK_WEEK_MIN   CLOCK_SUN

typedef	struct
{
	uint16_t YearBasic;  
	uint8_t rtcStatus;
	uint32_t totalSec;    //24小时内当前的总共秒数
}clock_t;  //时钟的十进制结构
extern clock_t clockNow;

enum
{
	ByHand,
	ByNet,
};

/**
* @brief        日期结构体对象
*/
typedef struct {
    uint16_t year;
    uint16_t month;
    uint16_t day;
    uint16_t hour;
    uint16_t min;
    uint16_t sec;
}date_time_t;


extern RTC_TimeType RTC_TimeStructure;
extern RTC_DateType RTC_DateStructure;

void RTC_DateAndTimeDefaultVale(void);
void RTC_CLKSourceConfig(uint8_t ClkSrc);
void RTC_PrescalerConfig(void);
ErrorStatus RTC_DateRegulate(void);
ErrorStatus RTC_TimeRegulate(void);
void RTC_DateShow(void);
void RTC_TimeShow(void);

#endif
