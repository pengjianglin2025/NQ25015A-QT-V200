
#include "aromaevent.h"
#include "aroma.h"
#include "clock.h"

const eventParameter_t initEvent[5] = {{0,0,23,59,WORK_TIME_DEFAULT,PAUSE_TIME_DEFAULT,60,0xff,1,0},
																			 {0,0,0,0,WORK_TIME_DEFAULT,PAUSE_TIME_DEFAULT,60,0xff,1,0},
																			 {0,0,0,0,WORK_TIME_DEFAULT,PAUSE_TIME_DEFAULT,60,0xff,1,0},
																			 {0,0,0,0,WORK_TIME_DEFAULT,PAUSE_TIME_DEFAULT,60,0xff,1,0},
																			 {0,0,0,0,WORK_TIME_DEFAULT,PAUSE_TIME_DEFAULT,60,0xff,1,0},};
eventParameter_t runEvent[5];
event_t event;
uint16_t runTime[7][5];
														 
void event_date_init(void)
{
	uint8_t i;
	
	airpump.status = S_NORMAL;
	fan.status = S_NORMAL;
	aroma.en = ON;
	fan.en = ON;
	event.dataComparisonResult = 0;
	for(i=0; i<5; i++)
	{
		runEvent[i] = initEvent[i];
	}
}

void event_Task(void)
{
	uint32_t StartTime,StopTime;
//	uint32_t TempA;//,TempB,TempC;
	uint32_t TempD,TempE;
	uint8_t i,CurrentEventNumTemp,WeekTemp;
	
	if(globalWorkState == FULL_WORKING)
	{
		//currentClock_DEC.totalSec = hourToSec[currentClock_DEC.Hour] + minToSec[currentClock_DEC.Minute] + currentClock_DEC.Second;
		
		CurrentEventNumTemp = 0;
		aroma.currentEventWorkTimeInTotal = 0;
		aroma.currentEventPauseTimeInTotal = 0;
		aroma.runTimeInTotal = 0;
		for(i=0; i<5; i++)
		{
			if(runEvent[i].en != 0)
			{
				switch(RTC_DateStructure.WeekDay)
				{
					case CLOCK_MON: WeekTemp = 0; break;
					case CLOCK_TUE: WeekTemp = 1; break;
					case CLOCK_WED: WeekTemp = 2; break;
					case CLOCK_THU: WeekTemp = 3; break;
					case CLOCK_FRI: WeekTemp = 4; break;
					case CLOCK_SAT: WeekTemp = 5; break;
					case CLOCK_SUN: WeekTemp = 6; break;
				}
				if(runEvent[i].weekEn.BYTE & 0x01<<(WeekTemp))
				{
					StartTime = hourToSec[runEvent[i].startTimeHour] + minToSec[runEvent[i].startTimeMinutes];
					StopTime = hourToSec[runEvent[i].stopTimeHour] + minToSec[runEvent[i].stopTimeMinutes];
						
					if((StartTime < StopTime) && (clockNow.totalSec >= StartTime) && (clockNow.totalSec < StopTime))
					{
//						TempA = runEvent[i].workTime;
//						TempB = runEvent[i].pauseTime;
//						TempC = runEvent[i].workTime + runEvent[i].pauseTime;
						TempD = runEvent[i].workTime*100/(runEvent[i].workTime + runEvent[i].pauseTime);
						aroma.runTimeInTotal += ((StopTime - StartTime) * TempD)/100;
						
						if(CurrentEventNumTemp == 0) 
						{
							aroma.currentEventWorkTimeInTotal = ((StopTime - StartTime) * TempD)/100;
							TempE = runEvent[i].pauseTime*100/(runEvent[i].workTime + runEvent[i].pauseTime);
							aroma.currentEventPauseTimeInTotal = ((StopTime - StartTime) * TempE)/100;
							CurrentEventNumTemp = i+1;
						}
					}
				}
			}
		}
		event.num = CurrentEventNumTemp;
		if(event.num) 
		{
			event.status = 1;
			event.num -= 1;  
		}
		else 
		{
			event.status = 0;
			event.num = 0; 
		}
	}
	else 
	{
		event.status = 0;
	}
	
	aroma_work_task();
}

void Calculate_TotalRunTime_Aweek(void)  //计算的是有效运行时间，重复设置和不合法设置的会排除
{
	uint32_t tempA,tempB;
	static uint8_t week,mode;
	uint16_t minute;
	
	for(mode=0; mode<5; mode++) 
	 runTime[week][mode] = 0; 
	
	for(minute=0; minute<1440; minute++)
	{
		for(mode=0; mode<5; mode++)
		{
			if((runEvent[mode].en) && (runEvent[mode].weekEn.BYTE & (0x01<<week)))
			{
				tempA = hourToMin[runEvent[mode].startTimeHour] + runEvent[mode].startTimeMinutes;
				tempB = hourToMin[runEvent[mode].stopTimeHour] + runEvent[mode].stopTimeMinutes;
				if((tempA < tempB) && (minute >= tempA) && (minute < tempB))
				{
					runTime[week][mode] += 1; break;
				}
			}
		}
	}
	if(++week > 6)
	{
		aroma.runTimeInTotal_Aweek = 0;
		for(mode=0; mode<5; mode++)
		{
			tempA = (runTime[0][mode] + runTime[1][mode] + runTime[2][mode] + runTime[3][mode] + runTime[4][mode] + runTime[5][mode] + runTime[6][mode]);
			aroma.runTimeInTotal_Aweek += tempA*60*runEvent[mode].workTime/(runEvent[mode].workTime+runEvent[mode].pauseTime);
		}
		week = 0;
	}
}

