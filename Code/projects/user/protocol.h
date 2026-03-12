#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include "main.h"
#include "app_usart.h"
#include "ns_delay.h"
#include "app_rdtss_16bit.h" 
#include "aroma.h"
#include "aromaevent.h"
#include "clock.h"

#define INTERVAL_TIME  10


#define HEAD1    	0x55    	
#define HEAD2    	0xaa   
#define RECEIVE_VERSION    	0x00 	
#define BACK_VERSION    	0x03   

#define TAIL1    	0x5a   	  

#define YEAR_SECOND    	0xaa   


//HBYTE 是 4-BIT, BYTE 是 8-BIT 数据型态，WORD 为 16 BIT， EWORD 为 24 BIT，DWORD 为 32 BIT。
typedef union
{
	uint32_t DWORD;
	struct
	{
		uint32_t ReportDeviceOperationStatus:	          1; 
		uint32_t DiDaBack:	                  1; 
		uint32_t NetworkingModeSetBack:	1; 
		uint32_t LinkStatusBack:	  1; 
		uint32_t ModuleReset:	    1; 
		uint32_t GetTime:	        1; 
		uint32_t ResetWifi:       1;
		uint32_t DPID001Back:	        1; 
		uint32_t DPID003Back:	        1; 
		uint32_t DPID004Back:	        1; 
		uint32_t DPID005Back:	        1; 
		uint32_t DPID007Back:	        1; 
		uint32_t DPID009Back:	        1; 
		uint32_t DPID010Back:	        1; 
		uint32_t DPID011Back:	        1; 
		uint32_t DPID012Back:	        1; 
		uint32_t DPID014Back:	        1;
		uint32_t DPID015Back:	        1; 
		uint32_t DPID016Back:	        1; 
		uint32_t DPID017Back:	        1;  
		uint32_t DPID018Back:	        1; 
		uint32_t DPID019Back:	        1; 
		uint32_t DPID020Back:	        1; 
		uint32_t DPID021Back:	        1; 
		uint32_t DPID022Back:	        1; 
		uint32_t DPID023Back:	        1; 
		uint32_t DPID024Back:	        1; 
		uint32_t DPID025Back:	        1; 
		uint32_t DPID026Back:	        1; 
		uint32_t DPID027Back:	        1; 
		uint32_t Reserverd:	          2; //最高位
	};
}upData_t;
extern upData_t upData;

typedef union
{
	uint16_t WORD;
	struct
	{
		uint16_t Reserverd:	              5; 
		uint16_t dataReceiveFlag:	        1; 
		uint16_t GetTimeAlready:	        1; 
		uint16_t HaveNewNetEventData:	        1; 
		uint16_t DiDaFirst:	        1; 
		uint16_t HaveNewRxData:	              1;  
		uint16_t LinkStatusOld:	               3; 
		uint16_t LinkStatus:	               3; //最高位
	};
}net_t;
extern net_t net;

typedef enum {
	NO_LINK,
	AT_LINKING,
	COMPLETED_LINK,
}Link_t;
extern Link_t NetLinkStatus;


#define PROTOCOL_DATA_MAX    128
typedef union
{
	uint8_t Buffer[PROTOCOL_DATA_MAX];
	struct
	{
		uint8_t Head1;   //帧头1
		uint8_t Head2;   //帧头2
		uint8_t Version;   //版本
		uint8_t CommandWord;    //命令字
		uint8_t Length_H;    //数据长度高八位
		uint8_t Length_L;    //数据长度低八位
		union
		{
			uint8_t FunctionalData[PROTOCOL_DATA_MAX-6];  //功能指令
			struct
			{
				uint8_t DP_ID;        //dpID
				uint8_t DP_Type;     //数据类型
				uint8_t DP_Len_H; 	//状态数据长度高八位
				uint8_t DP_Len_L; 	//状态数据长度低八位
				uint8_t DP_Data[PROTOCOL_DATA_MAX-10];  //状态数据
			};
		};
	};
}Message_t;
extern Message_t Rx,Tx;

extern uint16_t ReceiveIdleCount,SendIdleCount;  //串口接收闲置时间
extern uint16_t RxCnt,TxCnt;
extern uint16_t TxTotalLength;  //发送字符串的总长度
extern uint16_t RxTotalLength;  //接收字符串的总长度
extern uint16_t LocalCheckSum;

extern uint8_t RXLength;            //接收数据长度
extern bool RXFinishFlag;       //接收完成标志
extern bool RXStartFlag;        //接收起始标志
extern uint16_t CrcCheck;


void Module_Config(void);
void Module_Reset(void);
void app_data_parse_task(void);
void app_data_up_task(void);

#endif
