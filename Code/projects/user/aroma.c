#include "aroma.h"

//整个香氛运行包含气泵，风扇两个物理层支撑单元。
//香氛运行的控制为事件，有事件发生香氛就开始。

moto_t moto;

//uint32_t currentClock_DEC.totalSec;    //实时时间 单位：S 
aroma_t aroma;
fan_t fan;
airpump_t airpump;

/* PB5 对应 TIM3_CH2(AF2)，使用硬件 PWM 做气泵软启动。 */
static bool s_airpump_pwm_inited = false;
static bool s_airpump_output_pwm_mode = false;
static bool s_airpump_gpio_high = false;

#define AIRPUMP_PWM_PERIOD             (999U)  /* PWM周期(ARR)，与预分频共同决定频率 */
#define AIRPUMP_PWM_PRESCALER          (3U)    /* PWM预分频(PSC) */
#define AIRPUMP_DUTY_MAX_PERMILLE      (1000U)
#define AIRPUMP_SOFTSTART_TOTAL_TICK   (30U)   /* 30 * 5ms = 150ms */
#define AIRPUMP_SOFTSTART_DUTY_MIN     (250U)  /* 软启动起始占空比 25% */

/* 切回原GPIO直通输出，沿用软启动前已验证的稳定驱动方式。 */
static void airpump_gpio_set_output(bool high_level)
{
	GPIO_InitType GPIO_InitStructure;

	if((!s_airpump_output_pwm_mode) && (s_airpump_gpio_high == high_level))
	{
		return;
	}

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);

	GPIO_InitStruct(&GPIO_InitStructure);
	GPIO_InitStructure.Pin = GPIO_PIN_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

	if(high_level)
	{
		AIRPUMP_OUT_ON;
	}
	else
	{
		AIRPUMP_OUT_OFF;
	}

	s_airpump_output_pwm_mode = false;
	s_airpump_gpio_high = high_level;
}

/* 软启动阶段切换到TIM3_CH2硬件PWM输出。 */
static void airpump_gpio_set_pwm_output(void)
{
	GPIO_InitType GPIO_InitStructure;

	if(s_airpump_output_pwm_mode)
	{
		return;
	}

	RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);

	GPIO_InitStruct(&GPIO_InitStructure);
	GPIO_InitStructure.Pin = GPIO_PIN_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_MODE_AF_PP;
	GPIO_InitStructure.GPIO_Alternate = GPIO_AF2_TIM3;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);

	s_airpump_output_pwm_mode = true;
}

/* 将千分比占空比换算为CCR2比较值。 */
static void airpump_pwm_set_duty_permille(uint16_t duty_permille)
{
	uint32_t compare;
	if(duty_permille > AIRPUMP_DUTY_MAX_PERMILLE)
	{
		duty_permille = AIRPUMP_DUTY_MAX_PERMILLE;
	}

	compare = ((uint32_t)(AIRPUMP_PWM_PERIOD + 1U) * duty_permille) / AIRPUMP_DUTY_MAX_PERMILLE;
	TIM_SetCmp2(TIM3, (uint16_t)compare);
}

/* 供关机路径调用的硬关断接口：PWM占空比清零。 */
void airpump_pwm_force_off(void)
{
	if(!s_airpump_pwm_inited)
	{
		return;
	}

	airpump_pwm_set_duty_permille(0U);
}

/* 彻底关闭气泵输出：既关PWM，也将PB5切回GPIO低电平。 */
void airpump_force_off(void)
{
	airpump_pwm_force_off();
	airpump_gpio_set_output(false);
	airpump.SW = 0;
}

/* 初始化PB5为TIM3_CH2硬件PWM输出，仅初始化一次。 */
static void airpump_pwm_init(void)
{
	TIM_TimeBaseInitType TIM_TimeBaseStructure;
	OCInitType TIM_OCInitStructure;

	if(s_airpump_pwm_inited)
	{
		return;
	}

	RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_TIM3, ENABLE);

	TIM_InitTimBaseStruct(&TIM_TimeBaseStructure);
	TIM_TimeBaseStructure.Prescaler = AIRPUMP_PWM_PRESCALER;
	TIM_TimeBaseStructure.CntMode = TIM_CNT_MODE_UP;
	TIM_TimeBaseStructure.Period = AIRPUMP_PWM_PERIOD;
	TIM_InitTimeBase(TIM3, &TIM_TimeBaseStructure);

	TIM_InitOcStruct(&TIM_OCInitStructure);
	TIM_OCInitStructure.OcMode = TIM_OCMODE_PWM1;
	TIM_OCInitStructure.OutputState = TIM_OUTPUT_STATE_ENABLE;
	TIM_OCInitStructure.OcPolarity = TIM_OC_POLARITY_HIGH;
	TIM_OCInitStructure.Pulse = 0;
	TIM_InitOc2(TIM3, &TIM_OCInitStructure);
	TIM_ConfigOc2Preload(TIM3, TIM_OC_PRE_LOAD_ENABLE);
	TIM_ConfigArPreload(TIM3, ENABLE);
	TIM_EnableCtrlPwmOutputs(TIM3, ENABLE);
	TIM_Enable(TIM3, ENABLE);

	s_airpump_pwm_inited = true;
}

void WorkInit(void)
{
#if (AIRPUMP_DRIVE_MODE == AIRPUMP_DRIVE_MODE_PWM_SOFTSTART)
	airpump_pwm_init();
#endif
	/* 上电初始化时默认保持气泵关断。 */
	airpump_pwm_force_off();
	airpump_gpio_set_output(false);
	airpump.status = S_NORMAL;
	fan.status = S_NORMAL;
	event.dataComparisonResult = 0;
	event.num = 1;
	airpump.SW = 0;
	aroma.parameterMode = PROFESSIONAL;
	aroma.timeingGear = 0;
}


void aroma_work_task(void)
{
	static uint32_t cntA;
	static uint16_t cntB;
	static uint32_t workTimeOld,pauseTimeOld;
	static uint8_t minOld,secOld;
	
	if(power.state == POWER_ON)
	{
		BOOST_6291_EN_SET;
		if(aroma.parameterMode == SIMPLE)
		{
			aroma.startTime = 0;
			
			if(aroma.en == OFF)
			{
				airpump.SW = 0;
				fan.SW = OFF;
				cntB = 0; aroma.simpleModeCnt = 0;
				aroma.timeLeft = 0;
			}
			else if(!aroma.timeingGear)
			{
				aroma.timeLeft = 1440;    //如果剩余时间为24*60=1440说明定时功能取消，24小时运行模式
				cntB = 0;
			}
			else if(aroma.timeingGear)
			{
				if((aroma.timeingGearOld != aroma.timeingGear) || (aroma.timeingParameterUpdate))
				{
					aroma.timeingGearOld = aroma.timeingGear;
					aroma.timeingParameterUpdate = 0;
					switch(aroma.timeingGear)
					{
						case TIMEING_OFF: {aroma.timeLeft = 1440;}break;
						case TIMEING_1H: {aroma.timeLeft = 60;}break;
						case TIMEING_2H: {aroma.timeLeft = 120;}break;
						case TIMEING_4H: {aroma.timeLeft = 240;}break;
						case TIMEING_6H: {aroma.timeLeft = 360;}break;
						case TIMEING_8H: {aroma.timeLeft = 480;}break;
						case TIMEING_12H: {aroma.timeLeft = 720;}break;
						case TIMEING_CUSTOMIZE: {}break;  //自定义定时时间由app设置
					}
				}
				
				if(minOld != RTC_TimeStructure.Minutes)
				{
					minOld = RTC_TimeStructure.Minutes;
					upData.DPID027Back = 1;
					if(aroma.timeLeft)  
					{
						aroma.timeLeft--;
					}
					else
					{
						aroma.timeingGear = TIMEING_OFF;
						aroma.en = OFF;
					}
				}
			}
			
			if(aroma.timeLeft)
			{
				switch(aroma.concentration)
				{
					case 0:
					{
						aroma.simpleModeWorkSec = 15;
						aroma.simpleModePauseSec = 180;
					}
					break;
					case 1:
					{
						aroma.simpleModeWorkSec = 30;
						aroma.simpleModePauseSec = 180;
					}
					break;
					case 2:
					{
						aroma.simpleModeWorkSec = 30;
						aroma.simpleModePauseSec = 90;
					}
					break;
				}
				aroma.currentWorkSec = aroma.simpleModeWorkSec;
				aroma.currentPauseSec = aroma.simpleModePauseSec;
				
				if(aroma.simpleModeCnt < aroma.simpleModeWorkSec)
				{
					airpump.SW = 1;
					aroma.workStatus = WORK;
					aroma.currentWorkStatusRemainingTime = aroma.simpleModeWorkSec - aroma.simpleModeCnt;
				}
				else 
				{
					airpump.SW = 0;
					aroma.workStatus = PAUSE;
					aroma.currentWorkStatusRemainingTime = aroma.simpleModeWorkSec + aroma.simpleModePauseSec - aroma.simpleModeCnt;
				}
				
//					if(aroma.workStatus == WORK)
//					{
//						aroma.currentWorkStatusRemainingTime = aroma.simpleModeWorkSec - aroma.simpleModeCnt;
//					}
//					else
//					{
//						aroma.currentWorkStatusRemainingTime = aroma.simpleModeWorkSec + aroma.simpleModePauseSec - aroma.simpleModeCnt;
//					}
				if(secOld != RTC_TimeStructure.Seconds)
				{
					secOld = RTC_TimeStructure.Seconds;
					if(++aroma.simpleModeCnt >= aroma.simpleModeWorkSec + aroma.simpleModePauseSec) aroma.simpleModeCnt = 0;
				}
				
				airpump.status = airpump_operation_count_status(aroma.simpleModeWorkSec, aroma.simpleModePauseSec);
			}
		}
		else
		{
			cntB = 0; 
			aroma.timeingGear = 0;
			aroma.simpleModeCnt = 0;
			
			if((aroma.en == OFF) || (globalWorkState != FULL_WORKING))
			{
				airpump.SW = 0;
				fan.SW = OFF;
				aroma.startTime = 0;
//				aroma.currentPauseTime = 60;
				aroma.workStatus = STOP;
				
				aroma.currentWorkSec = 0;
				aroma.currentPauseSec = 0;
			}
			else if(!event.status)
			{
				airpump.SW = 0;
				fan.SW = OFF;
				aroma.startTime = 0;
//				aroma.currentPauseTime = 60;
				aroma.workStatus = STOP;
				aroma.currentWorkSec = 0;
				aroma.currentPauseSec = 0;
			}
			else
			{
				aroma.workStatus = aroma_run_status(event.num);
				
				if(aroma.workStatus == WORK)
				{
					airpump.SW = 1;
					fan.SW = ON;
					airpump.status= S_NORMAL;
				}
				else
				{
					airpump.SW = 0;
					fan.SW = OFF;
				}
				
				if((workTimeOld != runEvent[event.num].workTime) || (pauseTimeOld != runEvent[event.num].pauseTime))
				{
					workTimeOld = runEvent[event.num].workTime;
					pauseTimeOld = runEvent[event.num].pauseTime;
					airpump.cnt = 0;
				}
				
				aroma.currentWorkSec = runEvent[event.num].workTime;
				aroma.currentPauseSec = runEvent[event.num].pauseTime;
				
				airpump.status = airpump_operation_count_status(runEvent[event.num].workTime,runEvent[event.num].pauseTime);
			}
		}
		
		if(aroma.workStatusOld != aroma.workStatus)
		{
			aroma.workStatusOld = aroma.workStatus;
			upData.DPID024Back = 1;
		}
		
		if(aroma.concentrationOld != aroma.concentration)
		{
			aroma.concentrationOld = aroma.concentration;
			upData.DPID027Back = 1;
		}
		
		if((airpump.statusOld != airpump.status) || (fan.statusOld != fan.status))
		{
			airpump.statusOld = airpump.status;
			fan.statusOld = fan.status;
		}
	}
	else
	{
		BOOST_6291_EN_CLR;
		airpump.SW = 0;
		fan.SW = OFF;
		aroma.startTime = 0;
//		aroma.currentPauseTime = 60;
	}
	
}

void airpump_gpio_out(void)
{
	static uint8_t softstart_tick = 0;
	static bool airpump_enable_old = false;
	bool airpump_enable;
#if (AIRPUMP_DRIVE_MODE == AIRPUMP_DRIVE_MODE_PWM_SOFTSTART)
	uint16_t duty_permille;
#endif
	
	/* 仅在开泵且无故障时输出。 */
	airpump_enable = (airpump.SW) && (airpump.status != S_ERROR) && (clockNow.rtcStatus != S_ERROR);

#if (AIRPUMP_DRIVE_MODE == AIRPUMP_DRIVE_MODE_PWM_SOFTSTART)
	if(!s_airpump_pwm_inited)
	{
		airpump_pwm_init();
	}
#endif

	if(airpump_enable)
	{
#if (AIRPUMP_DRIVE_MODE == AIRPUMP_DRIVE_MODE_DIRECT_GPIO)
		airpump_gpio_set_output(true);
#elif (AIRPUMP_DRIVE_MODE == AIRPUMP_DRIVE_MODE_PWM_SOFTSTART)
		if(!airpump_enable_old)
		{
			/* 新一次启动从软启动起点开始爬升。 */
			softstart_tick = 0;
			airpump_gpio_set_pwm_output();
		}
		
		if(softstart_tick < AIRPUMP_SOFTSTART_TOTAL_TICK)
		{
			/* 软启动阶段按线性斜坡提升占空比。 */
			airpump_gpio_set_pwm_output();
			duty_permille = AIRPUMP_SOFTSTART_DUTY_MIN +
				(uint16_t)(((uint32_t)(AIRPUMP_DUTY_MAX_PERMILLE - AIRPUMP_SOFTSTART_DUTY_MIN) * softstart_tick) / AIRPUMP_SOFTSTART_TOTAL_TICK);
			airpump_pwm_set_duty_permille(duty_permille);
			softstart_tick++;
		}
		else
		{
			/* 软启动结束后回到原GPIO常高驱动，避免持续依赖PWM输出链路。 */
			airpump_gpio_set_output(true);
		}
#else
#error "Unsupported AIRPUMP_DRIVE_MODE"
#endif
	}
	else
	{
#if (AIRPUMP_DRIVE_MODE == AIRPUMP_DRIVE_MODE_PWM_SOFTSTART)
		airpump_pwm_force_off();
#endif
		airpump_gpio_set_output(false);
		/* 停止输出时清零计数，下一次启动重新软启动。 */
		softstart_tick = 0;
	}
	
	airpump_enable_old = airpump_enable;
}

//判断当前是工作状态还是暂停状态（不包括停止状态），以及计算当前工作状态的剩余时间
aromaWorkStatus_t aroma_run_status(uint8_t num)
{
	if(aroma.startTime == 0) { airpump.cnt = 0; aroma.startTime = clockNow.totalSec; }
				
	if(clockNow.totalSec < aroma.startTime) //从第一天23:59跨到到第二天00:00时就会出现这种情况
	{
		aroma.startTime = clockNow.totalSec;
	}
	else if(clockNow.totalSec < (aroma.startTime + runEvent[num].workTime))
	{
		aroma.currentWorkStatusRemainingTime = aroma.startTime + runEvent[num].workTime - clockNow.totalSec;
		return WORK;
	}
	else if(clockNow.totalSec < (aroma.startTime + runEvent[num].workTime + runEvent[num].pauseTime))
	{
		aroma.currentWorkStatusRemainingTime = aroma.startTime + runEvent[num].workTime + runEvent[num].pauseTime - clockNow.totalSec;
//		aroma.currentPauseTime = aroma.startTime + runEvent[num].workTime + runEvent[num].pauseTime - currentClock_DEC.totalSec;
	}
	else
	{
		aroma.startTime = clockNow.totalSec;
	}
	
	return PAUSE;
}

//判断气泵运行计时功能是否正常，避免计时问题导致气泵持续运行
//通过定时器计时跟当前模式运行的工作时间和暂停时间比较，如果到了时间没有及时切换就判断为计时不正常，就关掉气泵。
operationCountS_t airpump_operation_count_status(uint16_t workTime, uint16_t pauseTime)
{
	static uint8_t cntA;

	if(airpump.SWOld == airpump.SW)
	{
		if(++cntA >= 10)
		{
			cntA = 0;
			airpump.cnt++;
		}
		if(airpump.SW)
		{
			if(airpump.cnt >= WORK_OVERTIME)
			{
				airpump.cnt = WORK_OVERTIME + 1;
				return S_ERROR;
			}
			else if(airpump.cnt >= (workTime+10))
			{
				airpump.cnt = (workTime+10) + 1;
				return S_ERROR;
			}
		}
		else
		{
			if(airpump.cnt >= PAUSE_OVERTIME)
			{
				airpump.cnt = PAUSE_OVERTIME + 1;
				return S_ERROR;
			}
			else if(airpump.cnt >= (pauseTime+10))
			{
				airpump.cnt = (pauseTime+10) + 1;
				return S_ERROR;
			}
		}
	}
	else
	{
		airpump.SWOld = airpump.SW;
		airpump.cnt = 0;
		cntA = 0;
	}
	
	return S_NORMAL;
}








