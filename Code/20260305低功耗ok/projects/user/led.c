#include "led.h"
#include "app_gpio.h"
#include "protocol.h"
#include "mkey.h"
#include "bat.h"
#include "wf433.h"


uint16_t LedCnt = 0,LedCntA = 0;

ledStatus_t ledS;
Light_t Light;

void Led_Pin_Init(void)
{
}


void Led_Scan(void)
{
	static bool stage;
	
	LED_RED_SET; LED_GREEN_SET; LED_BLUE_SET;
	LED_COM2_CLR; LED_COM1_CLR;
	
	if(stage)
	{
		if(ledS.RED1) LED_RED_CLR;
		if(ledS.GREEN1) LED_GREEN_CLR;
		if(ledS.BLUE1) LED_BLUE_CLR;
		LED_COM1_SET;  
	}
	else
	{
		if(ledS.RED2) LED_RED_CLR;
		if(ledS.GREEN2) LED_GREEN_CLR;
		if(ledS.BLUE2) LED_BLUE_CLR;
		LED_COM2_SET;  
	}
	stage = !stage;
}
void Led_Task(void) 
{
	static uint16_t cntA;
	
	if(power.status == POWER_ON)
	{
//		ledS.RED1 = LEDS_ON; ledS.GREEN1 = LEDS_OFF; ledS.BLUE1 = LEDS_OFF;
//		ledS.RED2 = LEDS_ON; ledS.GREEN2 = LEDS_OFF; ledS.BLUE2 = LEDS_OFF;
	//	/*
		if(globalWorkState == MODE_RESET)
		{
			ledS.RED2 = LEDS_OFF; ledS.GREEN2 = LEDS_OFF; 
			ledS.RED1 = LEDS_OFF; ledS.GREEN1 = LEDS_OFF; 
			if(cntA%10 == 0)
			{
				ledS.BLUE1 = !ledS.BLUE1;
				ledS.BLUE2 = ledS.BLUE1;
			}
			if(++cntA > 200) 
			{
				cntA = 0;
					event_date_init();
//					oil_reset();
					fan.en = ON;
					aroma.en = ON;
					key.lockStatus = OFF;
				globalWorkState = FULL_WORKING;
			}
		}
		else
		{
			if(bat.status == BAT_DISCHARGE)
			{
				if(aroma.en)
				{
					switch(bat.volRange)
					{
						case VOLTAGE_RANGE1: { ledS.RED2 = LEDS_OFF; ledS.GREEN2 = LEDS_ON; ledS.BLUE2 = LEDS_OFF; }break;
						case VOLTAGE_RANGE2: { ledS.RED2 = LEDS_ON; ledS.GREEN2 = LEDS_ON; ledS.BLUE2 = LEDS_OFF; }break;
						case VOLTAGE_LOW: { ledS.RED2 = LEDS_ON; ledS.GREEN2 = LEDS_OFF; ledS.BLUE2 = LEDS_OFF; }break;
						case VOLTAGE_STOP: { ledS.RED2 = LEDS_ON; ledS.GREEN2 = LEDS_OFF; ledS.BLUE2 = LEDS_OFF; }break;
						default:  break;	
					}
				}
				else
				{
					ledS.RED2 = LEDS_OFF; ledS.GREEN2 = LEDS_OFF; ledS.BLUE2 = LEDS_OFF;
				}
			}
			else
			{
				if(bat.chargeFull)
				{
					ledS.GREEN2 = LEDS_ON;
					ledS.RED2 = LEDS_OFF;
				}
				else
				{
					if(LedCntA < 50) ledS.RED2 = LEDS_ON;
					else ledS.RED2 = LEDS_OFF;
					if(++LedCntA > 100) LedCntA = 0;
					ledS.GREEN2 = LEDS_OFF;
				}
				ledS.BLUE2 = LEDS_OFF;
			}
			
			if(CodeMatchingFlag)
			{
				if(LedCnt%10 == 0)
				{
					ledS.BLUE1 = !ledS.BLUE1;
				}
				if(++LedCnt > 100)
				{
					LedCnt = 0;
					wf433DataRightFlag = 0;
					net.dataReceiveFlag = 0;
					CodeMatchingFlag = 0;
				}
				ledS.RED1 = LEDS_OFF;
				ledS.GREEN1 = LEDS_OFF;
			}
			else if((airpump.status == S_ERROR) || (clockNow.rtcStatus == S_ERROR))
			{
				if(cntA < 20)
					ledS.RED1 = LEDS_OFF;
				else 
					ledS.RED1 = LEDS_ON;
				
				if(++cntA >= 40) cntA = 0;
				ledS.GREEN1 = LEDS_OFF;
				ledS.BLUE1 = LEDS_OFF;
			}
			else if(aroma.en == OFF)
			{
				ledS.RED1 = LEDS_ON;
				ledS.GREEN1 = LEDS_OFF;
				ledS.BLUE1 = LEDS_OFF;
			}
			else if((wf433DataRightFlag) | (net.dataReceiveFlag))
			{
				if(LedCnt%10 == 0)
				{
					ledS.BLUE1 = !ledS.BLUE1;
				}
				if(++LedCnt > 100)
				{
					LedCnt = 0;
					wf433DataRightFlag = 0;
					net.dataReceiveFlag = 0;
				}
				ledS.RED1 = LEDS_OFF;
				ledS.GREEN1 = LEDS_OFF;
			}
			else
			{
	//				ledS.RED1 = LEDS_OFF;
	//				ledS.GREEN1 = LEDS_ON;
	//				ledS.BLUE1 = LEDS_ON;
				if(event.status)
				{
					ledS.RED1 = LEDS_OFF;
					ledS.GREEN1 = LEDS_ON;
					ledS.BLUE1 = LEDS_ON;
				}
				else
				{
					ledS.RED1 = LEDS_ON;
					ledS.GREEN1 = LEDS_ON;
					ledS.BLUE1 = LEDS_OFF;
				}
			}
		}
		//*/
	}
	else
	{
		if(bat.status == BAT_CHARGE)
		{
			if(bat.chargeFull)
			{
				ledS.GREEN2 = LEDS_ON;
				ledS.RED2 = LEDS_OFF;
			}
			else
			{
				if(LedCntA < 50) ledS.RED2 = LEDS_ON;
				else ledS.RED2 = LEDS_OFF;
				if(++LedCntA > 100) LedCntA = 0;
				ledS.GREEN2 = LEDS_OFF;
			}
			ledS.BLUE2 = LEDS_OFF;
		}
		else
		{
			ledS.RED2 = LEDS_OFF; ledS.GREEN2 = LEDS_OFF; ledS.BLUE2 = LEDS_OFF;
		}
		ledS.RED1 = LEDS_OFF; ledS.GREEN1 = LEDS_OFF; ledS.BLUE1 = LEDS_OFF;
		
	}
	
}



