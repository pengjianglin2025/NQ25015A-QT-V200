
#ifndef _MAIN_H_
#define _MAIN_H_


/***********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include <stdbool.h> // standard boolean definitions
#include <stdint.h>  // standard integer functions
#include <string.h>


#define SYSTICK_100US           ((uint32_t)10000)
#define SYSTICK_1MS           ((uint32_t)1000)

typedef union
{
	uint8_t BYTE;
	struct
	{
		uint8_t B0:	          1; 
		uint8_t B1:	          1; 
		uint8_t B2:	          1; 
		uint8_t B3:	          1; 
		uint8_t B4:	          1; 
		uint8_t B5:	          1; 
		uint8_t B6:	          1; 
		uint8_t B7:	          1; //最高位
	};
}uint8To8_t;

typedef union
{
	uint8_t BYTE;
	struct
	{
		uint8_t HBYTE0:	        4;
		uint8_t HBYTE1:	        4; //最高位
	};
}uint8To2_t;

typedef union
{
	uint16_t WORD;
	struct
	{
		uint16_t BYTE0:	        8;
		uint16_t BYTE1:	        8; //最高位
	};
}uint16To2_t;

typedef union
{
	uint32_t DWORD;
	struct
	{
		uint32_t BYTE0:	        8; 
		uint32_t BYTE1:	        8; 
		uint32_t BYTE2:	        8;
		uint32_t BYTE3:	        8; //最高位
	};
}uint32To4_t;

typedef union
{
	uint32_t DWORD;
	struct
	{
		uint32_t HBYTE0:	        4; 
		uint32_t HBYTE1:	        4; 
		uint32_t HBYTE2:	        4;
		uint32_t HBYTE3:	        4; 
		uint32_t HBYTE4:	        4; 
		uint32_t HBYTE5:	        4;
		uint32_t HBYTE6:	        4;
		uint32_t HBYTE7:	        4; //最高位
	};
}uint32To8_t;


#include "n32wb03x.h"
#include "rwip.h"
#include "ns_ble.h"
#include "ns_sleep.h"
#include "ns_delay.h"
#include "ns_log.h"
#include "app_usart.h"
#include "app_gpio.h"
#include "app_ble.h"
#include "clock.h"
#include "mkey.h"
#include "led.h"
#include "bat.h"
#include "wf433.h"
#include "aroma.h"
#include "aromaevent.h"
#include "protocol.h"
#include "oil.h"
#include "iap.h"

enum
{
	OFF,
	ON,
};

typedef enum
{
	POWER_OFF,
	POWER_ON,
	POWER_CHARGE,
}powerS_t;

typedef struct 
{
	powerS_t state,prevState;
	bool sleepReady;
	uint16_t powerOffTicks,powerOnTicks,idleTicks;
}power_t;
extern power_t power;

typedef enum
{
	FULL_WORKING,     //全工作状态
	SET_EVENT,     //设置参数状态
	SET_CLOCK,     //设置时间状态
	MODE_RESET,
}WStatus_t;
extern WStatus_t globalWorkState;

typedef enum
{
	OIL_NULL,
	OIL_CALCULATE,  //计算型
	OIL_CHECK,     //探测型
}oilType_t;

typedef struct
{
	oilType_t CheckStyle; //精油检测方式
	uint8_t moto;
	uint8_t atmosphereLight;
	uint8_t quantityOfElectricity;
	uint8_t humanBodyInduction;
	uint8_t fan;
	uint8_t keyLock;
	uint8_t concentrationNum;   //简单模式下的浓度模式数量
}function_t;
extern function_t function;


#endif 


