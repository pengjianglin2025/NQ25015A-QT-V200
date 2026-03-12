
#include "mkey.h"
#include "app_gpio.h"
#include "wf433.h"
#include "aroma.h"

key_t key;

void Key_Init(void)
{
	key.pressFlag = 0;
	key.type.BYTE = 0;
	key.keep.BYTE = 0;
	key.clickCnt = 0;
	key.cnt = 0;
	key.idleCnt = 0;
}
void Key_Rset(void)
{
	key.pressFlag = 0;
	key.type.BYTE = 0;
	key.keep.BYTE = 0;
	key.clickCnt = 0;
	key.cnt = 0;
	key.idleCnt = 0;
}

/*************按键锁状态设置，必须清计数***************/
void keyLock_status_set(keyLock_t sta)
{
	key.lockStatus = sta;
	key.idleCnt = 0;
}

void Key_Task0(void)
{
	
	if(!POWER_KEY_GET)
	{
		key.idleCnt = 0;
		power.offTime = 0;
		
		if(key.cnt == 5)
		{
			key.clickCnt++;
		}
		else if(key.cnt == 200)
		{
			if(power.status != POWER_OFF)
			{
				power.status = POWER_OFF;
			}
			else if(power.status == POWER_OFF)
			{
				power.status = POWER_ON;
				aroma.en = ON;
			}
			power.onTime = 0;
			remotePairingTime = 0;
				
			key.clickCnt = 0;//key.cnt = 2000;
		}
		
		key.cnt++;
		if(key.cnt > 2500) 
		{
			key.cnt = 2500;
			key.status = KEY_IDLE;
			if(power.status != POWER_OFF)
			{
				power.status = POWER_OFF;
			}
		}
		else
		{
			key.status = KEY_ONLINE;
		}
	}
	else
	{
		if(key.idleCnt > 5) key.cnt = 0;
		
		if(key.idleCnt == 10)
		{
			if(key.clickCnt == 3)
			{
				key.clickCnt = 0;
				if(power.status == POWER_ON)//((PowerOnTime < 500) && (power.status == ON) && (bat.status != BAT_CHARGE))
				{
				}
			}
		}
		if(++key.idleCnt >= 100)
		{
			key.idleCnt = 251;
			key.clickCnt = 0;
			key.status = KEY_IDLE;
		}
	}
//	if(power.status) POWER_OUT_KEY = 1;
//		else POWER_OUT_KEY = 0;
}


void KeySleepSet(void)
{
}
