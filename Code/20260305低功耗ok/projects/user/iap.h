#ifndef _IAP_H_
#define _IAP_H_

#include "main.h"

#define IAP_WRITE_OK   0x55  //Êı¾İÒÑĞ´½øflashÇøÓò±êÖ¾
#define IAP_DATA_MAX   70

#define IAP_START_ADDR 0x500200   //µØÖ··

extern uint32_t Iap_Read_Temp[10];

typedef struct {
	uint8_t startTimeHour;        //¹¤×÷¿ªÊ¼Ê±¼äĞ¡Ê±
	uint8_t startTimeMinutes;     //¹¤×÷¿ªÊ¼Ê±¼ä·ÖÖÓ
	uint8_t stopTimeHour;         //¹¤×÷½áÊøÊ±¼äĞ¡Ê±
	uint8_t stopTimeMinutes;      //¹¤×÷½áÊøÊ±¼ä·ÖÖÓ
  uint16To2_t workTime;            //¹¤×÷Ê±¼ä
  uint16To2_t pauseTime;           //ÔİÍ£Ê±¼ä
	uint8_t workGear;             //¹¤×÷Å¨¶È
	uint8_t workWeek;             //¹¤×÷ÈÕ µÚ0Î»´ú±íĞÇÆÚÒ»....ÒÀ´ÎµÚ6Î»´ú±íĞÇÆÚÌì
	uint8_t eventEN;             //ÊÂ¼şÊ¹ÄÜ
}EventIap_t;

typedef union
{
	uint8_t Buffer[IAP_DATA_MAX];
	struct
	{
		uint8_t writeOk;
		EventIap_t EventData[5];
		uint8_t FanEN;
		uint8_t workState;
		uint8_t keyLockState;
		uint8_t lightEn;
		uint8_t totalVolume;
		uint8_t curretVolume;
		uint8_t consumeSpeed;
		uint8_t rollingCode1;
		uint8_t rollingCode2;
	};
}Iap_t;

void Iap_Read(void);
void Iap_Write(void);
void Iap_Data_Rest(void);
void Iap_Data_Comparison(void);  

#endif
