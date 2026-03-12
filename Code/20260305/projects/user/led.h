#ifndef _LED_H_
#define _LED_H_

#include "main.h"


enum
{
	LEDS_OFF,
	LEDS_ON,
};

typedef union
{
	uint8_t BYTE;
	struct
	{
		uint8_t RED1:	          1; 
		uint8_t GREEN1:	                  1; 
		uint8_t BLUE1:	1; 
		uint8_t RED2:	  1; 
		uint8_t GREEN2:	    1; 
		uint8_t BLUE2:	        1; 
		uint8_t Reserverd:	          2; //×î¸ßÎ»
	};
}ledStatus_t;
extern uint16_t LedCnt,LedCntA;

typedef struct
{
	bool en;
	uint8_t mode;
	uint8_t modeOld;
	uint8_t redValue;
	uint8_t greenValue;
	uint8_t blueValue;
}Light_t;
extern Light_t Light;


void Led_Pin_Init(void);
void Led_Scan(void);
void Led_Task(void);


#endif

