
#include "mkey.h"
#include "app_gpio.h"
#include "wf433.h"
#include "aroma.h"

key_t key;

/* 关机时立即关闭外部负载，避免等待周期任务造成残留电流。 */
static void key_power_hw_shutdown(void)
{
	EN_6291_CLR;
	/* 关机后拉高 STDBY，确保 NU1680 进入待机 */
	NU1680_STDBY_SET;
	AIRPUMP_OUT_OFF;
	WF433_EN_SW_SET;
	LED_RED_SET;
	LED_GREEN_SET;
	LED_BLUE_SET;
	LED_COM1_CLR;
	LED_COM2_CLR;
	aroma.en = OFF;
	airpump.SW = 0;
	fan.SW = OFF;
	if(DC_IN_GET)
	{
		EN_WSL2309_SET;
	}
	else
	{
		EN_WSL2309_CLR;
	}
}

/* 统一电源状态切换入口：同步处理 BLE 与外设电源。 */
static void key_power_status_set(powerS_t new_status)
{
	if(power.status == new_status)
	{
		return;
	}

	power.status = new_status;
	if(new_status == POWER_OFF)
	{
		/* 关机时先断连并停广播，再执行外围关断。 */
		ns_ble_disconnect();
		ns_ble_adv_stop();
		key_power_hw_shutdown();
	}
		else if(new_status == POWER_ON)
	{
		/* 开机后先释放 STDBY，再恢复广播，避免外围仍在待机态 */
		NU1680_STDBY_CLR;
		ns_ble_adv_start();
	}
}

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
				key_power_status_set(POWER_OFF);
			}
			else if(power.status == POWER_OFF)
			{
				key_power_status_set(POWER_ON);
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
				key_power_status_set(POWER_OFF);
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
