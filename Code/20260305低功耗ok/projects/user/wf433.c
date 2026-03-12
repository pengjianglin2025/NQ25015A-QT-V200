#include "wf433.h"
#include "app_gpio.h"
#include "aroma.h"
#include "aromaevent.h"
#include "clock.h"
#include "led.h"


uint8_t ReceiveOverFlag;  ////接收完数据标志
uint8_t wf433DataRightFlag = 0;
uint8_t CodeMatchingFlag;  //  对码完成标志
uint8_t RollingCode1, RollingCode2;
uint8_t Receive_Data[5];

void Wf433_Pin_Config(void)
{

}

void wf433_task(void)
{
	static uint8_t LowLevelCount = 0;
	static uint8_t HighLevelFlag = 0;
	static uint8_t SyncFlag = 0;   //同步标志
	static uint16_t RFinH_Count = 0,RFinL_Count = 0;
//	static uint8_t Receive_Total_Long = 0;
	static uint8_t DataBitFlag = 0;
	static uint8_t Receive_CountA = 0;
	
	if(power.status == POWER_ON)
	{
		WF433_EN_SW_CLR;
	
		if(!ReceiveOverFlag)		//关机或者接收完数据没有处理
		{
			if(SyncFlag == 1)	   //  确认已经同步过
			{
	//		PORT_ToggleBit(PORT7,PIN5); 
				if(!WF433_DATA_GET)
				{
					LowLevelCount++;
					HighLevelFlag = 0;
				}
				else
				{
					if(!HighLevelFlag)  //433M接收脚IN发生跳变(检测到上升沿判定前一个低电平时间长度)
					{
						if((LowLevelCount > 3)&&(LowLevelCount <= 7))	    //   接收到“1”   低电平长度    //300us   700us										
						{
							//PORT_SetBit(PORT7,PIN5);
							Receive_Data[Receive_CountA] = (Receive_Data[Receive_CountA] >> 1) | 0x80;      //433M 数据
							
							if(DataBitFlag == 0x80)
							{
								DataBitFlag = 0x01;
	//								if(Receive_CountA == 3)
	//								{
	//									Receive_Total_Long = Total_LongA;
	//									Check_Data = Receive_Data[3];
	//								}
	//								else if(Receive_CountA > 3)
	//								{
	//									Check_Data ^= Receive_Data[Receive_CountA];
	//								}
								
								Receive_CountA++;
								if(Receive_CountA >= MAX_BYTE_LONG)
								{
									Receive_CountA = 0;
									SyncFlag = 0;
									ReceiveOverFlag = 1;//接收结束标志	
								}
							}
							else {DataBitFlag <<= 1;}					
						}	
						else if((LowLevelCount > 7)&&(LowLevelCount < 15))   //    接收到“0”   
						{
							//PORT_ClrBit(PORT7,PIN5);
							Receive_Data[Receive_CountA] = (Receive_Data[Receive_CountA] >> 1) & 0x7f;
							
							if(DataBitFlag == 0x80)
							{
								DataBitFlag = 0x01;
	//								if(Receive_CountA == 3)
	//								{
	//									Receive_Total_Long = Total_LongA;
	//									Check_Data = Receive_Data[3];
	//								}
	//								else if(Receive_CountA > 3)
	//								{
	//									Check_Data ^= Receive_Data[Receive_CountA];
	//								}
								
								Receive_CountA++;
								if(Receive_CountA >= MAX_BYTE_LONG)
								{
									Receive_CountA = 0;
									SyncFlag = 0;
									ReceiveOverFlag = 1;//接收结束标志	
								}
							}
							else {DataBitFlag <<= 1;}						
						}
						else			//错误
						{
							SyncFlag = 0;
							DataBitFlag = 0x01;Receive_CountA = 0;
							ReceiveOverFlag = 0;	//接收结束标志										
						}											
					}
					LowLevelCount  = 0;
					HighLevelFlag = 1;
				}
			}
			
			if(SyncFlag == 0)			//没有同步过初值为0
			{
				if(WF433_DATA_GET)
				{
					//PORT_SetBit(PORT7,PIN5);
					RFinH_Count++;
					if((RFinL_Count > 80)&&(RFinL_Count < 160))	// 8000us < LL_w < 16000us 
					{
						SyncFlag = 1;
						DataBitFlag = 0x01;Receive_CountA = 0;
	//					Receive_Total_Long = 40;
						RFinH_Count = 0; RFinL_Count = 0; LowLevelCount = 0;
	//					High_Level_Flag = 1;
					}	
					RFinL_Count = 0;
				}
				else if(!WF433_DATA_GET)
				{
					//PORT_ClrBit(PORT7,PIN5);
					RFinL_Count++;
					if((RFinH_Count > 80)&&(RFinH_Count < 1600))	// 8000us < LL_w < 16000us 
					{
						SyncFlag = 1;
						DataBitFlag = 0x01;Receive_CountA = 0;
	//					Receive_Total_Long = 4;
						RFinH_Count = 0; RFinL_Count = 0; LowLevelCount = 0;
	//					High_Level_Flag = 1;
					}	
					RFinH_Count = 0;
				}						
			}
		}
	}
	else
	{
		WF433_EN_SW_SET;
		HighLevelFlag = 0;
		SyncFlag = 0;
		RFinH_Count = 0;RFinL_Count = 0;
		ReceiveOverFlag = 0; 
		wf433DataRightFlag = 0;
		DataBitFlag = 0;
		Receive_CountA = 0;
	}
}

uint16_t RemoteControlIdleTime;
uint16_t remotePairingTime;

void wf433_data_task(void)
{
	static uint16_t RemoteControlIdleTime;
	static uint8_t i,DataOld;
	
	if(++RemoteControlIdleTime > 2) RemoteControlIdleTime = 20;
	if(ReceiveOverFlag)
	{
		if(remotePairingTime <= 20000)
		{
			if(Receive_Data[2] == WF_ON)
			{
				RollingCode1 = Receive_Data[0];
				RollingCode2 = Receive_Data[1];
				CodeMatchingFlag = 1;
				remotePairingTime = 21000;
			}
			ReceiveOverFlag = 0;RemoteControlIdleTime = 0;return;
		}
		else if((Receive_Data[0]!=RollingCode1) || (Receive_Data[1]!=RollingCode2))//  
		{ 
			ReceiveOverFlag = 0;return;
		}
		else if(RemoteControlIdleTime <= 2)
		{
			ReceiveOverFlag = 0; RemoteControlIdleTime = 0;return;
		}	
		
		aroma.parameterMode = SIMPLE;
		
		switch(Receive_Data[2])
		{
			case WF_ON:
			{
				if(aroma.en == OFF)
				{
					aroma.en = ON;
					#if DUAL_MODE
					aroma.timeingParameterUpdate = 1;
					aroma.timeingGear = TIMEING_OFF;
					aroma.concentration = 0;
					#else
					event_date_init();
					
					for(i=0; i<5; i++)
					{
						runEvent[i].startTimeHour = 0;     
						runEvent[i].startTimeMinutes = 0;
						runEvent[i].stopTimeHour = 0;     
						runEvent[i].stopTimeMinutes = 0;
					}
					runEvent[0].startTimeHour = 0;     
					runEvent[0].startTimeMinutes = 0;
					runEvent[0].stopTimeHour = 23;     
					runEvent[0].stopTimeMinutes = 59; 
					for(i=0; i<5; i++)
					{
						runEvent[i].workTime = WORK_TIME_DEFAULT;     
						runEvent[i].pauseTime = PAUSE_TIME_DEFAULT;
					}
					#endif
				}
				wf433DataRightFlag = 1;
			}break;
			case WF_OFF:
			{
				if(aroma.en == ON)
				{
					aroma.en = OFF;
					wf433DataRightFlag = 1;
				}
			}break;
			case WF_L6:
			{
				if(aroma.en == ON)
				{
					#if DUAL_MODE
					aroma.concentration = 5;
					aroma.simpleModeCnt = 0;
					#else
					Gear_Set(30,90);
					aroma.startTime = 0;
					#endif
					wf433DataRightFlag = 1;
//					Set_Event_Charge_Flag = 1;
				}
			}break;
			case WF_L5:
			{
				if(aroma.en == ON)
				{
					#if DUAL_MODE
					aroma.concentration = 4;
					aroma.simpleModeCnt = 0;
					#else
					Gear_Set(30,120);
					aroma.startTime = 0;
					#endif
					wf433DataRightFlag = 1;
//					Set_Event_Charge_Flag = 1;
				}
			}break;
			case WF_L4:
			{
				if(aroma.en == ON)
				{
					#if DUAL_MODE
					aroma.concentration = 3;
					aroma.simpleModeCnt = 0;
					#else
					Gear_Set(30,180);
					aroma.startTime = 0;
					#endif
					wf433DataRightFlag = 1;
//					Set_Event_Charge_Flag = 1;
				}
			}break;
			
			case WF_L3:
			{
				if(aroma.en == ON)
				{
					#if DUAL_MODE
					aroma.concentration = 2;
					aroma.simpleModeCnt = 0;
					#else
					Gear_Set(15,120);
					aroma.startTime = 0;
					#endif
					wf433DataRightFlag = 1;
//					Set_Event_Charge_Flag = 1;
				}
			}break;
			case WF_L2:
			{
				if(aroma.en == ON)
				{
					#if DUAL_MODE
					aroma.concentration = 1;
					aroma.simpleModeCnt = 0;
					#else
					Gear_Set(15,150);
					aroma.startTime = 0;
					#endif
					wf433DataRightFlag = 1;
//					Set_Event_Charge_Flag = 1;
				}
			}break;
			
			case WF_L1:
			{
				if(aroma.en == ON)
				{
					#if DUAL_MODE
					aroma.concentration = 0;
					aroma.simpleModeCnt = 0;
					#else
					Gear_Set(15,180);
					aroma.startTime = 0;
					#endif
					wf433DataRightFlag = 1;
//					Set_Event_Charge_Flag = 1;
				}
			}break;
			case WF_1H:
			{
				if(aroma.en == ON)
				{
					#if DUAL_MODE
					aroma.timeingGear = TIMEING_1H;
					aroma.simpleModeCnt = 0;
					#else
					Timeing_Set(1);
					aroma.startTime = 0;
					#endif
					wf433DataRightFlag = 1;
//					Set_Event_Charge_Flag = 1;
				}
			}break;
			
			case WF_HIGH:
			{
				if(aroma.en == ON)
				{
					#if DUAL_MODE
					aroma.concentration = 0;
					aroma.simpleModeCnt = 0;
					#else
					Gear_Set(30,90);
					aroma.startTime = 0;
					#endif
					wf433DataRightFlag = 1;
//					Set_Event_Charge_Flag = 1;
				}
			}break;
			case WF_MED:
			{
				if(aroma.en == ON)
				{
					#if DUAL_MODE
					aroma.concentration = 0;
					aroma.simpleModeCnt = 0;
					#else
					Gear_Set(30,120);
					aroma.startTime = 0;
					#endif
					wf433DataRightFlag = 1;
//					Set_Event_Charge_Flag = 1;
				}
			}break;
			case WF_LOW:
			{
				if(aroma.en == ON)
				{
					#if DUAL_MODE
					aroma.concentration = 0;
					aroma.simpleModeCnt = 0;
					#else
					Gear_Set(30,180);
					aroma.startTime = 0;
					#endif
					wf433DataRightFlag = 1;
//					Set_Event_Charge_Flag = 1;
				}
			}break;
			case WF_4H:
			{
				if(aroma.en == ON)
				{
					#if DUAL_MODE
					aroma.timeingGear = TIMEING_4H;
					aroma.simpleModeCnt = 0;
					#else
					Timeing_Set(4);
					aroma.startTime = 0;
					#endif
					wf433DataRightFlag = 1;
//					Set_Event_Charge_Flag = 1;
				}
			}break;
			case WF_6H:
			{
				if(aroma.en == ON)
				{
					#if DUAL_MODE
					aroma.timeingGear = TIMEING_6H;
					aroma.simpleModeCnt = 0;
					#else
					Timeing_Set(6);
					aroma.startTime = 0;
					#endif
					wf433DataRightFlag = 1;
				}
			}break;
			case WF_8H:
			{
				if(aroma.en == ON)
				{
					#if DUAL_MODE
					aroma.timeingGear = TIMEING_8H;
					aroma.simpleModeCnt = 0;
					#else
					Timeing_Set(8);
					aroma.startTime = 0;
					#endif
					wf433DataRightFlag = 1;
//					Set_Event_Charge_Flag = 1;
				}
			}break;
			case WF_12H:
			{
				if(aroma.en == ON)
				{
					#if DUAL_MODE
					aroma.timeingGear = TIMEING_12H;
					aroma.simpleModeCnt = 0;
					#else
					Timeing_Set(12);
					aroma.startTime = 0;
					#endif
					wf433DataRightFlag = 1;
				}
			}break;
		}
		if(DataOld != Receive_Data[2])
		{
			DataOld = Receive_Data[2];
			LedCntA = 0;
		}
		ReceiveOverFlag = 0;
		RemoteControlIdleTime = 0;
	}
}

void Gear_Set(uint16_t workTime, uint16_t pauseTime)
{
	uint8_t i;
	
	for(i=2; i<5; i++)
	{
		runEvent[i].en = 0;
	}
	runEvent[0].en = 1;
	runEvent[0].weekEn.BYTE = 0xff;
	runEvent[0].workTime = workTime;     
	runEvent[0].pauseTime = pauseTime;
	if(runEvent[1].en)
	{
		runEvent[1].weekEn.BYTE = 0xff;
		runEvent[1].workTime = workTime;     
		runEvent[1].pauseTime = pauseTime;
	}
}
void Timeing_Set(uint8_t hour)
{
	uint8_t i;
	uint32_t tempA;
	
	if(hour > 24) return;
	
	for(i=0; i<5; i++)
	{
		runEvent[i].en = 0;
	}
	
	tempA = hourToMin[RTC_TimeStructure.Hours] + RTC_TimeStructure.Minutes + hourToMin[hour];
					
	if(tempA > (hourToMin[23]+60))
	{
		runEvent[0].en = 1;
		runEvent[0].weekEn.BYTE = 0xff;
		runEvent[0].startTimeHour = RTC_TimeStructure.Hours;     
		runEvent[0].startTimeMinutes= RTC_TimeStructure.Minutes;
		runEvent[0].stopTimeHour = 23;     
		runEvent[0].stopTimeMinutes= 59; 
		runEvent[1].en = 1;
		runEvent[1].weekEn.BYTE = 0xff;
		runEvent[1].workTime = runEvent[0].workTime;     
		runEvent[1].pauseTime = runEvent[0].pauseTime;
		runEvent[1].startTimeHour = 0;     
		runEvent[1].startTimeMinutes= 0;
		runEvent[1].stopTimeHour = (tempA - (hourToMin[23]+60))/60;     
		runEvent[1].stopTimeMinutes= (tempA - (hourToMin[23]+60))%60; 
	}
	else
	{
		runEvent[0].en = 1;
		runEvent[0].weekEn.BYTE = 0xff;
		runEvent[0].startTimeHour = RTC_TimeStructure.Hours;     
		runEvent[0].startTimeMinutes= RTC_TimeStructure.Minutes;
		runEvent[0].stopTimeHour = RTC_TimeStructure.Hours + hour;     
		runEvent[0].stopTimeMinutes= RTC_TimeStructure.Minutes;   
	}
}


