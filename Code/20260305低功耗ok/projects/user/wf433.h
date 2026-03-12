#ifndef _WF433_H_
#define _WF433_H_

#include "main.h"



//#define HEARD_NUM1       0X75
//#define HEARD_NUM2       0X3C
#define   DUAL_MODE     1   //是否支持双模式工作

#define MAX_BYTE_LONG       3

//struct 
//{
//	uint8_t wf433DataRightFlag: 1;
//	uint8_t HighLevelFlag:    1;
//	uint8_t SyncFlag:         1;
//	uint8_t ReceiveOverFlag:  1;
//	uint8_t :                 4;
//}FBinry;

enum
{
	WF_ON  = 0x21,
	WF_OFF  = 0x22,
//	WF_AROMA  = 0x23,
//	WF_DENSITY_UP  = 0x24,
//	WF_DENSITY_DOWN  = 0x25,
	WF_L1 = 0x26,
	WF_L2 = 0x27,
	WF_L3 = 0x28,
	WF_L4 = 0x29,
	WF_L5 = 0x2A,
	WF_L6 = 0x2B,
	WF_1H = 0x2C,
	WF_4H = 0x2D,
	WF_8H = 0x2E,
	WF_6H = 0x2F,
	WF_12H = 0x31,
	WF_LOW = 0x32,
	WF_MED = 0x33,
	WF_HIGH = 0x34,
};

//enum
//{
//	WF_LOW = 0x04,
//	WF_MED = 0x07,
//	WF_HIGH = 0x10,
//	WF_ON  = 0x01,
//	WF_OFF  = 0x03,
//	WF_2H = 0x06,
//	WF_4H = 0x09,
//	WF_8H = 0x12,
//};

extern uint8_t wf433DataRightFlag;
extern uint8_t CodeMatchingFlag;  //  对码完成标志
extern uint8_t RollingCode1, RollingCode2;
extern uint16_t remotePairingTime;

void Wf433_Pin_Config(void);
void wf433_task(void);
void wf433_data_task(void);
void Gear_Set(uint16_t workTime, uint16_t pauseTime);
void Timeing_Set(uint8_t hour);

#endif
