#include "iap.h"

Iap_t IapRead,IapWrite;
uint8_t IapWriteEn = 0;


#define FLASH_TEST_ADDRESS       0x1020000
#define BUFFER_SIZE              IAP_DATA_MAX


void Iap_Read(void)
{
	memset(IapRead.Buffer,0,sizeof(IapRead.Buffer));
	
	Qflash_Read(FLASH_TEST_ADDRESS, IapRead.Buffer, BUFFER_SIZE);

	if(IapRead.writeOk == IAP_WRITE_OK)
	{
		for(uint8_t i=0; i<5; i++)
		{
			runEvent[i].startTimeHour = IapRead.EventData[i].startTimeHour;
			runEvent[i].startTimeMinutes = IapRead.EventData[i].startTimeMinutes;
			runEvent[i].stopTimeHour = IapRead.EventData[i].stopTimeHour;
			runEvent[i].stopTimeMinutes = IapRead.EventData[i].stopTimeMinutes;
			runEvent[i].workTime = IapRead.EventData[i].workTime.WORD;
			runEvent[i].pauseTime = IapRead.EventData[i].pauseTime.WORD;
			runEvent[i].workPer = IapRead.EventData[i].workGear;
			runEvent[i].weekEn.BYTE = IapRead.EventData[i].workWeek;
			runEvent[i].en = IapRead.EventData[i].eventEN;
		}
		fan.en = IapRead.FanEN;
		aroma.en = IapRead.workState;
		key.lockStatus = IapRead.keyLockState;
		oil.curretVolume = IapRead.curretVolume;
		oil.actualConsumeSpeed = IapRead.consumeSpeed;
		RollingCode1 = IapRead.rollingCode1;
		RollingCode2 = IapRead.rollingCode2;
	}
	else
	{
		event_date_init();
		oil_reset();
		fan.en = ON;
		aroma.en = ON;
		key.lockStatus = UNLOCK;
		Iap_Write();
	}
}
void Iap_Write(void)
{
	uint8_t i,k;
	
	memset(IapWrite.Buffer,0,sizeof(IapWrite.Buffer));
	
	IapWrite.writeOk = IAP_WRITE_OK;
	for(i=0; i<5; i++)
	{
		IapWrite.EventData[i].startTimeHour = runEvent[i].startTimeHour;
		IapWrite.EventData[i].startTimeMinutes = runEvent[i].startTimeMinutes;
		IapWrite.EventData[i].stopTimeHour = runEvent[i].stopTimeHour;
		IapWrite.EventData[i].stopTimeMinutes = runEvent[i].stopTimeMinutes;
		IapWrite.EventData[i].workTime.WORD =	runEvent[i].workTime;
		IapWrite.EventData[i].pauseTime.WORD = runEvent[i].pauseTime;
		IapWrite.EventData[i].workGear = runEvent[i].workPer;
		IapWrite.EventData[i].workWeek = runEvent[i].weekEn.BYTE;
		IapWrite.EventData[i].eventEN = runEvent[i].en;
	}	
	IapWrite.FanEN = fan.en;
	IapWrite.workState = aroma.en;
	IapWrite.keyLockState = key.lockStatus;
	IapWrite.curretVolume = oil.curretVolume;
	IapWrite.consumeSpeed =	oil.actualConsumeSpeed;
	IapWrite.rollingCode1 = RollingCode1;
	IapWrite.rollingCode2 = RollingCode2;
	
	/* 擦写Flash后做读回比对，最多重试3次，提升断电存储可靠性。 */
	for(k=0; k<3; k++)
	{
		Qflash_Erase_Sector(FLASH_TEST_ADDRESS);
		
    Qflash_Write(FLASH_TEST_ADDRESS, IapWrite.Buffer, BUFFER_SIZE); 
		
		memset(IapRead.Buffer,0,sizeof(IapRead.Buffer));
		Qflash_Read(FLASH_TEST_ADDRESS, IapRead.Buffer, BUFFER_SIZE);
		
		IapWriteEn = 0;
		/* 从索引1开始比较：索引0为写入标志位，避免擦写瞬态导致误判。 */
		for(i=1; i<BUFFER_SIZE; i++) { if(IapRead.Buffer[i] != IapWrite.Buffer[i]) { IapWriteEn = 1; break; } }
		
		if(!IapWriteEn) { break; }
	}
	
	IapWriteEn = 0;
}
/**************************************************************************
** 功能	@brief : 比较运行的参数和flash里的值，如果不一样就更新flash的值
** 输入	@param : 
** 输出	@retval: 
***************************************************************************/
void Iap_Data_Comparison(void)  
{
	uint8_t i;
	
	if(globalWorkState == FULL_WORKING)
	{
		memset(IapRead.Buffer, 0, sizeof(IapRead.Buffer));
		Qflash_Read(FLASH_TEST_ADDRESS, IapRead.Buffer, BUFFER_SIZE);
		
		memset(IapWrite.Buffer,0,sizeof(IapWrite.Buffer));
		IapWrite.writeOk = IAP_WRITE_OK;
		for(i=0; i<5; i++)
		{
			IapWrite.EventData[i].startTimeHour = runEvent[i].startTimeHour;
			IapWrite.EventData[i].startTimeMinutes = runEvent[i].startTimeMinutes;
			IapWrite.EventData[i].stopTimeHour = runEvent[i].stopTimeHour;
			IapWrite.EventData[i].stopTimeMinutes = runEvent[i].stopTimeMinutes;
			IapWrite.EventData[i].workTime.WORD =	runEvent[i].workTime;
			IapWrite.EventData[i].pauseTime.WORD = runEvent[i].pauseTime;
			IapWrite.EventData[i].workGear = runEvent[i].workPer;
			IapWrite.EventData[i].workWeek = runEvent[i].weekEn.BYTE;
			IapWrite.EventData[i].eventEN = runEvent[i].en;
		}	
		IapWrite.FanEN = fan.en;
		IapWrite.workState = aroma.en;
		IapWrite.keyLockState = key.lockStatus;
		IapWrite.curretVolume = oil.curretVolume;
		IapWrite.consumeSpeed =	oil.actualConsumeSpeed;
		IapWrite.rollingCode1 = RollingCode1;
		IapWrite.rollingCode2 = RollingCode2;
		
		for(i=1; i<BUFFER_SIZE; i++)
		{
			if(IapRead.Buffer[i] != IapWrite.Buffer[i])
			{
				IapWriteEn = 1;
				if(!upData.DPID018Back) upData.DPID018Back = 1;
				if(i < 51) aroma.startTime = 0;
				break;
			}
		}
		if(IapRead.FanEN != IapWrite.FanEN) {IapWriteEn = 1; if(!upData.DPID004Back) upData.DPID004Back = 1;}
		if(IapRead.workState != IapWrite.workState) {IapWriteEn = 1; if(!upData.DPID001Back) upData.DPID001Back = 1;}
		if(IapRead.keyLockState != IapWrite.keyLockState) {IapWriteEn = 1; if(!upData.DPID005Back) upData.DPID005Back = 1;}
		if(IapRead.curretVolume != IapWrite.curretVolume) {IapWriteEn = 1;if(!upData.DPID020Back) upData.DPID020Back = 1;}
		if(IapRead.consumeSpeed != IapWrite.consumeSpeed) {IapWriteEn = 1;if(!upData.DPID020Back) upData.DPID020Back = 1;}
		
		if(IapWriteEn) //&& (!airpump.SW))
		{
			IapWriteEn = 0;
//			AlarmMode = 3;
			IapWrite.writeOk = IAP_WRITE_OK;
			Qflash_Erase_Sector(FLASH_TEST_ADDRESS);
			
			Qflash_Write(FLASH_TEST_ADDRESS, IapWrite.Buffer, BUFFER_SIZE); 
		}
	
	}
	else
	{
		IapWriteEn = 0;
	}
	
		
}
void Iap_Data_Rest(void)
{
}


