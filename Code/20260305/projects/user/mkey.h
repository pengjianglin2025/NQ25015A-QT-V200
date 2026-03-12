#ifndef __MKEY_H__
#define __MKEY_H__

#include "main.h"

#define    BSET(x,y)    x|=(1<<y)	//置1
#define    BCLR(x,y)    x&=~(1<<y)	//置0



typedef enum
{
	KEY_IDLE,
	KEY_ONLINE,
}keyS_t;

typedef enum
{
	UNLOCK,
	LOCK,
}keyLock_t;


//HBYTE 是 4-BIT, BYTE 是 8-BIT 数据型态，WORD 为 16 BIT， EWORD 为 24 BIT，DWORD 为 32 BIT。
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
}keyData_t;

typedef struct 
{
	keyS_t status;
	keyData_t scan,scanOld,keep;
	uint8To8_t type;
	keyLock_t lockStatus;
	bool pressFlag;
//	bool reset,onceFlag;
	uint16_t cnt,idleCnt,delayTime;
	uint8_t clickCnt;
}key_t;
extern key_t key;


void Key_Init(void);
void Key_Task0(void);
void keyLock_status_set(keyLock_t sta);
void Key_Rset(void);
void KeySleepSet(void);

#endif
