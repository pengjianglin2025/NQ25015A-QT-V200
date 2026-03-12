
#include "aroma.h"

//整个香氛运行包含气泵，风扇两个物理层支撑单元。
//香氛运行的控制为事件，有事件发生香氛就开始。

moto_t moto;

//uint32_t currentClock_DEC.totalSec;    //实时时间 单位：S 
aroma_t aroma;	
fan_t fan;	
airpump_t airpump;				

void WorkInit(void)
{
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
	
	if(power.status == POWER_ON)
	{
		EN_6291_SET;
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
		EN_6291_CLR;
		airpump.SW = 0;
		fan.SW = OFF;
		aroma.startTime = 0;
//		aroma.currentPauseTime = 60;
	}
	
}

void airpump_gpio_out(void)
{
	static uint16_t duty;
	
	if(airpump.SW)
	{
		if((airpump.status == S_ERROR) || (clockNow.rtcStatus == S_ERROR))  //气泵运行超时保护（可能由于RTC原因造成时间不跑）
		{
			AIRPUMP_OUT_OFF;
//			duty = AIRPUMP_DUTY_INITIAL_VALUE;
		}
		else
		{
			AIRPUMP_OUT_ON;
//			duty += 50;
//			if(duty > AIRPUMP_DUTY_MAX_VALUE) duty = AIRPUMP_DUTY_MAX_VALUE;
		}
	}
	else 
	{
		AIRPUMP_OUT_OFF;
//		duty = AIRPUMP_DUTY_INITIAL_VALUE;
	}
	
//	TM40->TDR01 = duty;
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
