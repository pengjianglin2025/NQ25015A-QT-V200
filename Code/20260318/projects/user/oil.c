
#include "oil.h"

oil_t oil;

void oil_init(oilVolumeType_t oilVolumeType , oilSpeedType_t oilSpeedType)
{
	oil.volumeType = oilVolumeType;
	
  if(oil.volumeType == OIL_150ml) 
	{
		oil.totalVolume = 150; 
	}
  else if(oil.volumeType == OIL_180ml) 
	{
		oil.totalVolume = 180; 
	}
  else if(oil.volumeType == OIL_200ml) 
	{
		oil.totalVolume = 200; 
	}
  else if(oil.volumeType == OIL_240ml) 
	{
		oil.totalVolume = 240; 
	}
  else if(oil.volumeType == OIL_250ml) 
	{
		oil.totalVolume = 250; 
	}
	else  
	{
		oil.totalVolume = 500; 
	}
	
	oil.defaultConsumeSpeed = oilSpeedType;
	oil.actualConsumeSpeedMax = (oil.defaultConsumeSpeed + oilSpeedType);
	if(oil.defaultConsumeSpeed > oilSpeedType)
		oil.actualConsumeSpeedMin = oil.defaultConsumeSpeed - oilSpeedType;
	else
		oil.actualConsumeSpeedMin = 0;
}


void oil_reset(void)
{
	oil.surplusDay = 100;
	oil.curretVolume = oil.totalVolume;
	
	oil.actualConsumeSpeed = oil.defaultConsumeSpeed;
	
	oil.warnThreshold = 10;
	oil.curretUsedTime = 0;
	oil.armalFlag = 0;
}

void oil_surplusDay_task(void)
{
	if((oil.curretVolumeOld != oil.curretVolume) || (oil.actualConsumeSpeedOld != oil.actualConsumeSpeed) || (aroma.runTimeInTotal_AweekOld != aroma.runTimeInTotal_Aweek))
	{
		oil.curretVolumeOld = oil.curretVolume;
		oil.actualConsumeSpeedOld = oil.actualConsumeSpeed;
		aroma.runTimeInTotal_AweekOld = aroma.runTimeInTotal_Aweek;
		if((oil.actualConsumeSpeed == 0) || (aroma.runTimeInTotal_Aweek == 0))
		{
			oil.surplusDay = 65500;
		}
		else
		{
			oil.surplusDay = oil.curretVolume*252000/(oil.actualConsumeSpeed*aroma.runTimeInTotal_Aweek);//oil.curretVolume*3600/(oil.consumeSpeed/10)*7/aroma.runTimeInTotal_Aweek;//精油可用天数
		}
	
		if(oil.totalVolume > 0)
			oil.curretVolumePercent = oil.curretVolume*100/oil.totalVolume;
		else
			oil.curretVolumePercent = 0;
	}
	
	if(oil.surplusDay > 65500) { oil.surplusDay = 65500; }
	
	if(oil.surplusDayOld != oil.surplusDay)
	{
		oil.surplusDayOld = oil.surplusDay;
		upData.DPID020Back = 1;
	}
	
	if(oil.curretVolumePercentOld != oil.curretVolumePercent)
	{
		oil.curretVolumePercentOld = oil.curretVolumePercent;
//		upData.Bit.DPID105 = 1;
	}
}

void oil_currentVolume_calculate_task(void)
{
	static uint16_t CntA,CntB,CntC;
	
//	EN_3V3_SET;
//	PORT_Init(PORT1,PIN5,PULLDOWN_INPUT);
//	m0_delay_ms(3);
	
	if(airpump.SW)
	{
		oil.curretUsedTime++;
		if(oil.actualConsumeSpeed*oil.curretUsedTime >= 360000)
		{
			if(oil.curretVolume > 0) 
				oil.curretVolume -= 1;//当前精油容量 
			oil.curretUsedTime = 0;
		}
	}
	
//	if(OIL_CHECK_IN_GET)
//	{ 
//		CntB = 0; CntC = 0;
//		if(CntA%300 == 0) { upData.DPID017Back = 1; oil.armalFlag = 1; oil.curretVolume = 0;}
//		if(++CntA > 1000) CntA = 1000;
//	}
//	else 
		if(oil.curretVolumePercent < 10)
	{
		CntA = 0; CntB = 0;
		if(CntC%300 == 0) { upData.DPID017Back = 1; oil.armalFlag = 1;}
		if(++CntC > 1000) CntC = 1000;
	}
	else 
	{ 
		CntA = 0; CntC = 0;
		if(CntB%300 == 0) { upData.DPID017Back = 1; oil.armalFlag = 0;}
		if(++CntB > 1000) CntB = 1000;
		
		if(oil.curretVolume > oil.totalVolume) { oil.curretVolume = oil.totalVolume; }
	}
	
//	EN_3V3_CLR;
//	PORT_Init(PORT1,PIN5,INPUT);
}


