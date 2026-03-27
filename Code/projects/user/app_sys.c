#include "app_sys.h"

/* 统一应用关机态硬件输出，避免关机链路分散到多个模块。 */
void app_sys_power_apply_off_outputs(void)
{
	BOOST_6291_EN_CLR;
	POWER_CTRL_STDBY_SET;
	airpump_force_off();
	WF433_EN_SW_SET;
	LED_RED_SET;
	LED_GREEN_SET;
	LED_BLUE_SET;
	LED_COM1_CLR;
	LED_COM2_CLR;

	aroma.en = OFF;
	airpump.SW = 0;
	fan.SW = OFF;

	if(CHARGE_ACTIVE_LEVEL_GET)
	{
		CHARGE_CTRL_EN_SET;
	}
	else
	{
		CHARGE_CTRL_EN_CLR;
	}
}

/* 统一应用开机态关键硬件恢复。 */
void app_sys_power_apply_on_outputs(void)
{
	POWER_CTRL_STDBY_CLR;
}

/* 系统层统一的电源状态切换入口。 */
void app_sys_power_set(powerS_t new_status)
{
	if(power.state == new_status)
	{
		return;
	}

	power.state = new_status;

	if(new_status == POWER_OFF)
	{
		Iap_Write();
		ns_ble_disconnect();
		for(uint8_t i=0; i<20; i++)
		{
			rwip_schedule();
		}
		ns_ble_adv_stop();
		for(uint8_t i=0; i<20; i++)
		{
			rwip_schedule();
		}
		app_sys_power_apply_off_outputs();

		if(!CHARGE_ACTIVE_LEVEL_GET)
		{
			/* 未插电关机时，直接停止应用层周期定时器，避免反复浅睡唤醒。 */
			ke_timer_clear(APP_5MS_EVT, TASK_APP);
			ke_timer_clear(APP_20MS_EVT, TASK_APP);
			ke_timer_clear(APP_100MS_EVT, TASK_APP);
			ke_timer_clear(APP_500MS_EVT, TASK_APP);
			ke_timer_clear(APP_1S_EVT, TASK_APP);
			power.powerOffTicks = 20;
		}
	}
	else if(new_status == POWER_ON)
	{
		app_sys_power_apply_on_outputs();
		ns_ble_adv_start();
	}
}
