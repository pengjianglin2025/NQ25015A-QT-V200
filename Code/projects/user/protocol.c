#include "protocol.h"
#include "mkey.h"
#include "led.h"
#include "app_ota_feature.h"
#include "iap.h"


const eventParameter_t NetDateInit[5] = {{0,0,0,0,0,0,0,0x7f,1,0},
																 {0,0,0,0,0,0,0,0x7f,1,0},
																 {0,0,0,0,0,0,0,0x7f,1,0},
																 {0,0,0,0,0,0,0,0x7f,1,0},
																 {0,0,0,0,0,0,0,0x7f,1,0},};

const char ConncetMode[70] = "{\"p\":\"p11r26_MVZ4NjdLR3F0VUUv\",\"v\":\"1.0.0\",\"tslid\":0,\"m\":0,\"mt\":3}"; 

uint16_t RxCnt = 0,TxCnt = 0;
uint16_t TxTotalLength=0;  //�����ַ������ܳ���
uint16_t ReceiveIdleCount = 0,SendIdleCount = 0;  //���ڽ���,��������ʱ��
uint16_t	LocalCheckSum = 0;

upData_t upData;
net_t net;
/* ���ջ���ֻ�� protocol.c �ڲ�ά�����ⲿͨ���ӿ��� Buffer ָ�롣 */
static Message_t Rx;
/* ���ͻ���ֻ�� protocol.c �ڲ�ʹ�ã������տڿɼ�����д���ա� */
static Message_t Tx;
Link_t NetLinkStatus;

uint8_t* protocol_rx_buffer_get(void)
{
	return Rx.Buffer;
}

uint16_t protocol_rx_buffer_size_get(void)
{
	return (uint16_t)sizeof(Rx.Buffer);
}
//volatile eventParameter_t NetEvent[5];
																 
											
void Module_Config(void)
{
	net.WORD = 0;
	upData.DWORD = 0;
	app_ota_feature_init();
}		
void Module_Reset(void)
{
	net.WORD = 0;
	upData.DWORD = 0;
	upData.ModuleReset = 1;
}			 

/* ����ǰЭ���ʽ�����ͨ�� BLE notify ���� App�� */
void UARTTxData(uint16_t Length)
{
	uint16_t i;
	uint16_t LocalCheckSum;
	uint16To2_t len;
	len.WORD = Length;
	
	LocalCheckSum = 0;
	Tx.Head1 = HEAD1;            
	Tx.Head2 = HEAD2;          
	Tx.Version = BACK_VERSION;     
	Tx.Length_H = len.BYTE1;  
	Tx.Length_L = len.BYTE0; 
	
	/**************֡������****************/
	for (i=0; i<(len.WORD+6); i++) { LocalCheckSum += Tx.Buffer[i]; }
	Tx.Buffer[len.WORD+6] = LocalCheckSum%256;
	
	TxTotalLength	= len.WORD+7;
	TxCnt = 0;
	
	rdtss_16bit_send_notify((uint8_t*)Tx.Buffer, TxTotalLength);
}

/* �� Rx.Buffer �н���һ֡����Э�飬���ѽ���ַ����� DP ҵ������� */
void app_data_parse_task(void)
{
	RTC_DateType colckDataTemp;
	RTC_TimeType colckTimeTemp;
	uint16_t DataLenght;
	uint16_t DpLength;
	uint8_t i,j,n,CheckSumTemp;
	uint16To2_t TempA;
	
	/* net.HaveNewRxData �ɻ��ζ�����ȡ������֡����λ�� */
	if(net.HaveNewRxData)// && (ReceiveIdleCount > INTERVAL_TIME))
	{
		
		LocalCheckSum = 0;
		CheckSumTemp = 0;
		/* ����֡ͷ�Ͱ汾У�飬��������������У����жϡ� */
		if((Rx.Head1 == HEAD1) && (Rx.Head2 == HEAD2) && (Rx.Version == RECEIVE_VERSION))
		{
			/* Length ��ʾ�� Command/DP ��ʼ�� payload ���ܳ��ȣ�����ͷ��У���ֽڡ� */
			DataLenght = Rx.Length_H*256 + Rx.Length_L;
			if(DataLenght > (PROTOCOL_DATA_MAX - 7))
			{
				TxCnt = 0;
				net.HaveNewRxData = 0;
				RxCnt = 0;
				memset(Rx.Buffer,0,sizeof(Rx.Buffer));
				return;
			}
			
      for(i=0; i<DataLenght+6; i++) { LocalCheckSum += Rx.Buffer[i]; }
			CheckSumTemp = LocalCheckSum%256;
			
			
			/* У��ͨ����Ž��������ַַ���������/�����дҵ��״̬�� */
			if(CheckSumTemp == Rx.Buffer[DataLenght+6])
			{
				net.dataReceiveFlag = 1;
				
				switch(Rx.CommandWord)
				{
					case 0x00: { upData.DiDaBack = 1; }break;
					case 0x01: { upData.NetworkingModeSetBack = 1; }break;
					case 0x02: { }break;
					case 0x03: 
					{ 
						upData.LinkStatusBack = 1;
						
						net.LinkStatus = Rx.FunctionalData[0];
						if(Rx.FunctionalData[0] == 0x05)
						{
							upData.ModuleReset = 1;
						}
					}break;
					case 0x04: { upData.ModuleReset = 0; }break;
					case 0x1c: 
					{ 
						if(Rx.FunctionalData[0])
						{
							colckDataTemp.Year = Rx.FunctionalData[1];
							colckDataTemp.Month = Rx.FunctionalData[2];
							colckDataTemp.Date = Rx.FunctionalData[3];
							colckTimeTemp.Hours = Rx.FunctionalData[4];
							colckTimeTemp.Minutes = Rx.FunctionalData[5];
							colckTimeTemp.Seconds = Rx.FunctionalData[6];
							switch(Rx.FunctionalData[7])
							{
								case 1: colckDataTemp.WeekDay = CLOCK_MON; break;
								case 2: colckDataTemp.WeekDay = CLOCK_TUE; break;
								case 3: colckDataTemp.WeekDay = CLOCK_WED; break;
								case 4: colckDataTemp.WeekDay = CLOCK_THU; break;
								case 5: colckDataTemp.WeekDay = CLOCK_FRI; break;
								case 6: colckDataTemp.WeekDay = CLOCK_SAT; break;
								case 7: colckDataTemp.WeekDay = CLOCK_SUN; break;
								default: colckDataTemp.WeekDay = Rx.FunctionalData[7]; break;
							}
							if((colckTimeTemp.Seconds < 60) && (colckTimeTemp.Minutes < 60) && (colckTimeTemp.Hours < 24)\
								&& (colckDataTemp.WeekDay >= CLOCK_WEEK_MIN) && (colckDataTemp.WeekDay <= CLOCK_WEEK_MAX))
							{
								RTC_SetDate(RTC_FORMAT_BIN, &colckDataTemp);
								RTC_ConfigTime(RTC_FORMAT_BIN, &colckTimeTemp);
								net.GetTimeAlready = 1;
							}
						}
					}break;
					case 0x06:
					{
						/* 0x06 �� DP �·���ڣ����� DP ���ݳ��ȣ��ٰ� DP_ID ��ҵ������ */
						DpLength = Rx.DP_Len_H*256 + Rx.DP_Len_L;
						switch(Rx.DP_ID)
						{
							case 1: { 
							aroma.en = Rx.DP_Data[0];  
							/*upData.DPID001Back = 1;*/
							}break;
							case 3: { 
							aroma.concentration = Rx.DP_Data[0];  
								upData.DPID003Back = 1;  
							}break;
							case 4: { 
							fan.en = Rx.DP_Data[0];  /*upData.DPID004Back = 1; */ 
							}break;
							case 5: { 
								if(Rx.DP_Data[0])
								{
									keyLock_status_set(LOCK);
								}
								else
								{
									keyLock_status_set(UNLOCK);
								}								/*upData.DPID005Back = 1; */ 
							}break;
							case 9: { 
							Light.en = Rx.DP_Data[0];  /*upData.DPID009Back = 1;*/  
							}break;
							case 15: { 
							moto.En = Rx.DP_Data[0];  upData.DPID015Back = 1;  
							}break;
							case 18: 
							{ 
								/* רҵģʽ 5 ���¼������̶� 55 �ֽڣ�������˳������ָ��� */
								if(DpLength != 55) { break; }
								n = 0;
								for(i=0; i<5; i++)
								{
									if(Rx.DP_Data[n] < 5)
									{
										j = Rx.DP_Data[n++];
										runEvent[j].weekEn.BYTE = Rx.DP_Data[n++];
										runEvent[j].startTimeHour = Rx.DP_Data[n++];
										runEvent[j].startTimeMinutes = Rx.DP_Data[n++];
										runEvent[j].stopTimeHour = Rx.DP_Data[n++];
										runEvent[j].stopTimeMinutes = Rx.DP_Data[n++];
										runEvent[j].en = Rx.DP_Data[n++];
										TempA.BYTE1 = Rx.DP_Data[n++];
										TempA.BYTE0 = Rx.DP_Data[n++];
										runEvent[j].workTime =  TempA.WORD;
										TempA.BYTE1 = Rx.DP_Data[n++];
										TempA.BYTE0 = Rx.DP_Data[n++];
										runEvent[j].pauseTime =  TempA.WORD;
									}
									else { break; }
								}
							}break;
							case 20: 
							{ 
								/* ��������������/��ǰ��/Ĭ���ͺ�/ʣ������/ʵ���ͺġ� */
								if(DpLength != 8) { break; }
								n = 0;
								TempA.BYTE1 = Rx.DP_Data[n++];
								TempA.BYTE0 = Rx.DP_Data[n++];
								oil.totalVolume =  TempA.WORD;
								TempA.BYTE1 = Rx.DP_Data[n++];
								TempA.BYTE0 = Rx.DP_Data[n++];
								oil.curretVolume =  TempA.WORD;
								oil.defaultConsumeSpeed =  Rx.DP_Data[n++];
								TempA.BYTE1 = Rx.DP_Data[n++];
								TempA.BYTE0 = Rx.DP_Data[n++];
								oil.surplusDay =  TempA.WORD;
								oil.actualConsumeSpeed =  Rx.DP_Data[n++];
								upData.DPID020Back = 1;  
							}break;
							case 24: { upData.DPID024Back = 1;  }break;
							case 25: { 
							aroma.parameterMode = Rx.DP_Data[0];
							upData.DPID025Back = 1;
							}break;
							case 26: { upData.DPID026Back = 1;  }break;
							case 27: 
							{ 
								/* ��ģʽ��ʱ��ʣ������� + ��ǰ��λ�� */
								if(DpLength != 3) { break; }
								n = 0;
								TempA.BYTE1 = Rx.DP_Data[n++];
								TempA.BYTE0 = Rx.DP_Data[n++];
								aroma.timeLeft =  TempA.WORD;
								aroma.concentration = Rx.DP_Data[n++];
								if(aroma.concentration)
									aroma.concentration -= 1;
								else
									aroma.concentration = 0;
								aroma.timeingGear = TIMEING_CUSTOMIZE;
							}break;
						}
//						alarmMode = 1;
					}break;
					case 0x08: 
					{ 
						upData.DPID001Back = 1;
						upData.DPID003Back = 1;
						upData.DPID004Back = 1;
						upData.DPID005Back = 1;
						upData.DPID009Back = 1;
						upData.DPID015Back = 1;
						upData.DPID017Back = 1;
						upData.DPID018Back = 1;
						upData.DPID020Back = 1;
						upData.DPID022Back = 1;
						upData.DPID023Back = 1;
						upData.DPID024Back = 1;
						upData.DPID025Back = 1;
						upData.DPID026Back = 1;
						upData.DPID027Back = 1;
					}break;
					default:
					{
						(void)app_ota_feature_handle_command(&Rx);
					}break;
				}
//				alarmMode = 1;
			}
			TxCnt = 0;
		}
		
		net.HaveNewRxData = 0;
		RxCnt = 0;
		memset(Rx.Buffer,0,sizeof(Rx.Buffer));
	}
	else if(ReceiveIdleCount > INTERVAL_TIME)
	{
		RxCnt = 0;
		memset(Rx.Buffer,0,sizeof(Rx.Buffer));
	}
	
}
/* �� upData ��־λ���豸״̬�������ϱ��� App�� */
void app_data_up_task(void)
{
	static uint8_t CntA,CntB;
	uint8_t i,n;
	uint16To2_t Length;
	uint16To2_t TempA;
	
	if(net.LinkStatusOld != net.LinkStatus)
	{
		net.LinkStatusOld = net.LinkStatus;
		if(net.LinkStatus != 0x04){net.GetTimeAlready = 0;}
	}
	/* �豸һ����������ûУʱ���Ͱ��̶������� App ����ʱ�䡣 */
	if(!net.GetTimeAlready)
	{
		if(net.LinkStatus == 0x04) 
		{
			if(CntB == 0) upData.GetTime = 1; 
			if(++CntB > 100) CntB = 0;
		}
		else
		{
			CntB = 0;
		}
	}
	
//	if(SendIdleCount > (INTERVAL_TIME*5))
//	{
		memset(Tx.Buffer,0,sizeof(Tx.Buffer));
	
		if(app_ota_feature_prepare_tx(&Tx, &Length.WORD))
		{
			UARTTxData(Length.WORD);
		}
		else if(upData.DiDaBack)
		{
			Tx.CommandWord = 0x00;
      if(net.DiDaFirst){Tx.FunctionalData[0] = 1;}
			else {Tx.FunctionalData[0] = 0; net.DiDaFirst = 1;}
			UARTTxData(1);
			upData.DiDaBack = 0;
//			if(!net.BIT.GetTimeAlready) upData.BIT.GetTime = 1;
		}
		else if(upData.NetworkingModeSetBack)
		{
			Tx.CommandWord = 0x01;
			memcpy(Tx.Buffer + 6, ConncetMode, strlen(ConncetMode));
			UARTTxData(strlen(ConncetMode));
			upData.NetworkingModeSetBack = 0;
		}		
		else if(upData.LinkStatusBack)
		{
			Tx.CommandWord = 0x03;
			UARTTxData(0);
			upData.LinkStatusBack = 0;
		}
		else if(upData.ModuleReset)
		{
			if(CntA == 0)
			{
				Tx.CommandWord = 0x04;
				UARTTxData(0);
//			upData.BIT.ModuleReset = 0;
			}
			if(++CntA > 30) CntA = 0;
		}
		else if(upData.GetTime)
		{
			Tx.CommandWord = 0x1c;
			UARTTxData(0);
			upData.GetTime = 0;
		}
		
		else if(upData.DPID001Back)
		{
			Tx.CommandWord = 0x07; Tx.DP_ID = 1; Tx.DP_Type = 0x01; Length.WORD = 1; Tx.DP_Len_H = Length.BYTE1;    Tx.DP_Len_L = Length.BYTE0; 
			Tx.DP_Data[0] = aroma.en;
			
			UARTTxData(5);
			upData.DPID001Back = 0;
		}
		else if(upData.DPID003Back)
		{
			Tx.CommandWord = 0x07; Tx.DP_ID = 3; Tx.DP_Type = 0x01; Length.WORD = 1; Tx.DP_Len_H = Length.BYTE1;    Tx.DP_Len_L = Length.BYTE0; 
			Tx.DP_Data[0] = aroma.concentration; 
			
			UARTTxData(5);
			upData.DPID003Back = 0;
		}
		else if(upData.DPID004Back)
		{
			Tx.CommandWord = 0x07; Tx.DP_ID = 4; Tx.DP_Type = 0x01; Length.WORD = 1; Tx.DP_Len_H = Length.BYTE1;    Tx.DP_Len_L = Length.BYTE0; 
			Tx.DP_Data[0] = fan.en;
			
			UARTTxData(5);
			upData.DPID004Back = 0;
		}
		else if(upData.DPID005Back)
		{
			Tx.CommandWord = 0x07; Tx.DP_ID = 5; Tx.DP_Type = 0x01; Length.WORD = 1; Tx.DP_Len_H = Length.BYTE1;    Tx.DP_Len_L = Length.BYTE0; 
			Tx.DP_Data[0] = key.lockStatus;
			
			UARTTxData(5);
			upData.DPID005Back = 0;
		}
		else if(upData.DPID009Back)
		{
			Tx.CommandWord = 0x07; Tx.DP_ID = 9; Tx.DP_Type = 0x01; Length.WORD = 1; Tx.DP_Len_H = Length.BYTE1;    Tx.DP_Len_L = Length.BYTE0; 
			Tx.DP_Data[0] = Light.en;
			
			UARTTxData(5);
			upData.DPID009Back = 0;
		}
		else if(upData.DPID015Back)
		{
			Tx.CommandWord = 0x07; Tx.DP_ID = 15; Tx.DP_Type = 0x01; Length.WORD = 1; Tx.DP_Len_H = Length.BYTE1; Tx.DP_Len_L = Length.BYTE0; 
			Tx.DP_Data[0] = moto.En; 
			
			UARTTxData(5);
			upData.DPID015Back = 0;
		}
		else if(upData.DPID017Back)
		{
			Tx.CommandWord = 0x07; Tx.DP_ID = 17; Tx.DP_Type = 0x01; Length.WORD = 1; Tx.DP_Len_H = Length.BYTE1; Tx.DP_Len_L = Length.BYTE0;
			Tx.DP_Data[0] = oil.armalFlag;

			UARTTxData(5);
			upData.DPID017Back = 0;
		}
		else if(upData.DPID020Back)
		{
			/* DP20 �ϱ�������ز������� App ��ҳָ�꿨������ҳ��ʹ�á� */
			Tx.CommandWord = 0x07; Tx.DP_ID = 20; Tx.DP_Type = 0x00; Length.WORD = 8; Tx.DP_Len_H = Length.BYTE1; Tx.DP_Len_L = Length.BYTE0; 
			
			n = 0;
			TempA.WORD = oil.totalVolume;
			Tx.DP_Data[n++] = TempA.BYTE1;
			Tx.DP_Data[n++] = TempA.BYTE0;
			TempA.WORD = oil.curretVolume;
			Tx.DP_Data[n++] = TempA.BYTE1;
			Tx.DP_Data[n++] = TempA.BYTE0;
			Tx.DP_Data[n++] = oil.defaultConsumeSpeed;
			TempA.WORD = oil.surplusDay;
			Tx.DP_Data[n++] = TempA.BYTE1;
			Tx.DP_Data[n++] = TempA.BYTE0;
			Tx.DP_Data[n++] = oil.actualConsumeSpeed;
			
			UARTTxData(12);
			upData.DPID020Back = 0;
		}
		else if(upData.DPID022Back)
		{
			/* DP22 �ϱ������������ã�App ��ݴ˾�����ʾ��Щ������ڡ� */
			Tx.CommandWord = 0x07; Tx.DP_ID = 22; Tx.DP_Type = 0x00; Length.WORD = 7; Tx.DP_Len_H = Length.BYTE1; Tx.DP_Len_L = Length.BYTE0; 
			
			n = 0;
			Tx.DP_Data[n++] = function.CheckStyle;
			Tx.DP_Data[n++] = function.moto;
			Tx.DP_Data[n++] = function.atmosphereLight;
			Tx.DP_Data[n++] = function.quantityOfElectricity;
			Tx.DP_Data[n++] = function.humanBodyInduction;
			Tx.DP_Data[n++] = function.fan;
			Tx.DP_Data[n++] = function.keyLock;
			
			UARTTxData(11);
			upData.DPID022Back = 0;
		}
		else if(upData.DPID023Back)
		{
			Tx.CommandWord = 0x07; Tx.DP_ID = 23; Tx.DP_Type = 0x00; Length.WORD = 9; Tx.DP_Len_H = Length.BYTE1; Tx.DP_Len_L = Length.BYTE0; 
			
			n = 0;
			TempA.WORD = PAUSE_TIME_MIN;
			Tx.DP_Data[n++] = TempA.BYTE1;
			Tx.DP_Data[n++] = TempA.BYTE0;
			TempA.WORD = PAUSE_TIME_MAX;
			Tx.DP_Data[n++] = TempA.BYTE1;
			Tx.DP_Data[n++] = TempA.BYTE0;
			TempA.WORD = WORK_TIME_MIN;
			Tx.DP_Data[n++] = TempA.BYTE1;
			Tx.DP_Data[n++] = TempA.BYTE0;
			TempA.WORD = WORK_TIME_MAX;
			Tx.DP_Data[n++] = TempA.BYTE1;
			Tx.DP_Data[n++] = TempA.BYTE0;
			Tx.DP_Data[n++] = SET_TIME_STEP;
			
			UARTTxData(13);
			upData.DPID023Back = 0;
		}
		else if(upData.DPID024Back)
		{
			Tx.CommandWord = 0x07; Tx.DP_ID = 24; Tx.DP_Type = 0x00; Length.WORD = 7; Tx.DP_Len_H = Length.BYTE1; Tx.DP_Len_L = Length.BYTE0; 
			
			n = 0;
			Tx.DP_Data[n++] = aroma.workStatus;
			TempA.WORD = aroma.currentWorkStatusRemainingTime;
			Tx.DP_Data[n++] = TempA.BYTE1;
			Tx.DP_Data[n++] = TempA.BYTE0;
			TempA.WORD = aroma.currentWorkSec;
			Tx.DP_Data[n++] = TempA.BYTE1;
			Tx.DP_Data[n++] = TempA.BYTE0;
			TempA.WORD = aroma.currentPauseSec;
			Tx.DP_Data[n++] = TempA.BYTE1;
			Tx.DP_Data[n++] = TempA.BYTE0;
			
			UARTTxData(11);
			upData.DPID024Back = 0;
		}
		else if(upData.DPID025Back)
		{
			Tx.CommandWord = 0x07; Tx.DP_ID = 25; Tx.DP_Type = 0x01; Length.WORD = 1; Tx.DP_Len_H = Length.BYTE1; Tx.DP_Len_L = Length.BYTE0; 
			Tx.DP_Data[0] = aroma.parameterMode;
			
			UARTTxData(5);
			upData.DPID025Back = 0;
		}
		else if(upData.DPID026Back)
		{
			Tx.CommandWord = 0x07; Tx.DP_ID = 26; Tx.DP_Type = 0x00; Length.WORD = 1; Tx.DP_Len_H = Length.BYTE1; Tx.DP_Len_L = Length.BYTE0; 
			Tx.DP_Data[0] = function.concentrationNum;
			
			UARTTxData(5);
			upData.DPID026Back = 0;
		}
		else if(upData.DPID027Back)
		{
			Tx.CommandWord = 0x07; Tx.DP_ID = 27; Tx.DP_Type = 0x00; Length.WORD = 3; Tx.DP_Len_H = Length.BYTE1; Tx.DP_Len_L = Length.BYTE0;

			n = 0;
			TempA.WORD = aroma.timeLeft;
			Tx.DP_Data[n++] = TempA.BYTE1;
			Tx.DP_Data[n++] = TempA.BYTE0;
			Tx.DP_Data[n++] = aroma.concentration + 1;
			UARTTxData(7);
			upData.DPID027Back = 0;
		}
		else if(upData.DPID018Back)
		{
			/* DPID018 payload=66 bytes, needs MTU>=69. Moved last so DPID020-027
			   are always sent first. ble_att_mtu is updated by notification confirms
			   and equals (negotiated_mtu - 3); default=20, after setBLEMTU(256)=253. */
			extern uint16_t ble_att_mtu;
			if(ble_att_mtu >= 66)
			{
				Tx.CommandWord = 0x07; Tx.DP_ID = 18; Tx.DP_Type = 0x00; Length.WORD = 55; Tx.DP_Len_H = Length.BYTE1; Tx.DP_Len_L = Length.BYTE0;

				n = 0;
				for(i=0; i<5; i++)
				{
					Tx.DP_Data[n++] = i;
					Tx.DP_Data[n++] = runEvent[i].weekEn.BYTE;
					Tx.DP_Data[n++] = runEvent[i].startTimeHour;
					Tx.DP_Data[n++] = runEvent[i].startTimeMinutes;
					Tx.DP_Data[n++] = runEvent[i].stopTimeHour;
					Tx.DP_Data[n++] = runEvent[i].stopTimeMinutes;
					Tx.DP_Data[n++] = runEvent[i].en;
					TempA.WORD = runEvent[i].workTime;
					Tx.DP_Data[n++] = TempA.BYTE1;
					Tx.DP_Data[n++] = TempA.BYTE0;
					TempA.WORD = runEvent[i].pauseTime;
					Tx.DP_Data[n++] = TempA.BYTE1;
					Tx.DP_Data[n++] = TempA.BYTE0;
				}

				UARTTxData(59);
			}
			upData.DPID018Back = 0;
		}

//	}
}

