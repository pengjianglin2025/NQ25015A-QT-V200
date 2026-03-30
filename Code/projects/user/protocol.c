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
uint16_t TxTotalLength=0;  //锟斤拷锟斤拷锟街凤拷锟斤拷锟斤拷锟杰筹拷锟斤拷
uint16_t ReceiveIdleCount = 0,SendIdleCount = 0;  //锟斤拷锟节斤拷锟斤拷,锟斤拷锟斤拷锟斤拷锟斤拷时锟斤拷
uint16_t	LocalCheckSum = 0;

upData_t upData;
net_t net;
/* 锟斤拷锟秸伙拷锟斤拷只锟斤拷 protocol.c 锟节诧拷维锟斤拷锟斤拷锟解部通锟斤拷锟接匡拷锟斤拷 Buffer 指锟诫。 */
static Message_t Rx;
/* 锟斤拷锟酵伙拷锟斤拷只锟斤拷 protocol.c 锟节诧拷使锟矫ｏ拷锟斤拷锟斤拷锟秸口可硷拷锟斤拷锟斤拷写锟斤拷锟秸★拷 */
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

/* 锟斤拷锟斤拷前协锟斤拷锟绞斤拷锟斤拷锟斤拷通锟斤拷 BLE notify 锟斤拷锟斤拷 App锟斤拷 */
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
	
	/**************帧锟斤拷锟斤拷锟斤拷****************/
	for (i=0; i<(len.WORD+6); i++) { LocalCheckSum += Tx.Buffer[i]; }
	Tx.Buffer[len.WORD+6] = LocalCheckSum%256;
	
	TxTotalLength	= len.WORD+7;
	TxCnt = 0;
	
	rdtss_16bit_send_notify((uint8_t*)Tx.Buffer, TxTotalLength);
}

/* 锟斤拷 Rx.Buffer 锟叫斤拷锟斤拷一帧锟斤拷锟斤拷协锟介，锟斤拷锟窖斤拷锟斤拷址锟斤拷锟斤拷锟?DP 业锟斤拷锟斤拷锟斤拷锟?*/
void app_data_parse_task(void)
{
	RTC_DateType colckDataTemp;
	RTC_TimeType colckTimeTemp;
	uint16_t DataLenght;
	uint16_t DpLength;
	uint8_t i,j,n,CheckSumTemp;
	uint16To2_t TempA;
	
	/* net.HaveNewRxData 锟缴伙拷锟轿讹拷锟斤拷锟斤拷取锟斤拷锟斤拷锟斤拷帧锟斤拷锟斤拷位锟斤拷 */
	if(net.HaveNewRxData)// && (ReceiveIdleCount > INTERVAL_TIME))
	{
		
		LocalCheckSum = 0;
		CheckSumTemp = 0;
		/* 锟斤拷锟斤拷帧头锟酵版本校锟介，锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷校锟斤拷锟斤拷卸稀锟?*/
		if((Rx.Head1 == HEAD1) && (Rx.Head2 == HEAD2) && (Rx.Version == RECEIVE_VERSION))
		{
			/* Length 锟斤拷示锟斤拷 Command/DP 锟斤拷始锟斤拷 payload 锟斤拷锟杰筹拷锟饺ｏ拷锟斤拷锟斤拷头锟斤拷校锟斤拷锟街节★拷 */
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
			
			
			/* 校锟斤拷通锟斤拷锟斤拷沤锟斤拷锟斤拷锟斤拷锟斤拷址址锟斤拷锟斤拷锟斤拷锟斤拷锟?锟斤拷锟斤拷锟叫匆碉拷锟阶刺拷锟?*/
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
						/* 0x06 锟斤拷 DP 锟铰凤拷锟斤拷冢锟斤拷锟斤拷锟?DP 锟斤拷锟捷筹拷锟饺ｏ拷锟劫帮拷 DP_ID 锟斤拷业锟斤拷锟斤拷锟斤拷 */
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
								/* 专业模式 5 锟斤拷锟铰硷拷锟斤拷锟斤拷锟教讹拷 55 锟街节ｏ拷锟斤拷锟斤拷锟斤拷顺锟斤拷锟斤拷锟斤拷指锟斤拷锟?*/
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
								/* 锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷/锟斤拷前锟斤拷/默锟斤拷锟酵猴拷/剩锟斤拷锟斤拷锟斤拷/实锟斤拷锟酵耗★拷 */
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
								/* 锟斤拷模式锟斤拷时锟斤拷剩锟斤拷锟斤拷锟斤拷锟?+ 锟斤拷前锟斤拷位锟斤拷 */
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
/* 锟斤拷 upData 锟斤拷志位锟斤拷锟借备状态锟斤拷锟斤拷锟斤拷锟较憋拷锟斤拷 App锟斤拷 */
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
	/* 锟借备一锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷没校时锟斤拷锟酵帮拷锟教讹拷锟斤拷锟斤拷锟斤拷 App 锟斤拷锟斤拷时锟戒。 */
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
			/* DP20 锟较憋拷锟斤拷锟斤拷锟斤拷夭锟斤拷锟斤拷锟斤拷锟?App 锟斤拷页指锟疥卡锟斤拷锟斤拷锟斤拷页锟斤拷使锟矫★拷 */
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
			/* DP22 锟较憋拷锟斤拷锟斤拷锟斤拷锟斤拷锟斤拷锟矫ｏ拷App 锟斤拷荽司锟斤拷锟斤拷锟绞撅拷锟叫╋拷锟斤拷锟斤拷锟节★拷 */
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
			/* DPID018 payload is 59 bytes on-air, so keep it pending until the
			   negotiated notify payload is large enough. app_env.max_mtu is updated
			   by the BLE stack before app_usart refreshes ble_att_mtu. */
			extern uint16_t ble_att_mtu;
			uint16_t notify_payload_mtu = ble_att_mtu;
			if((app_env.max_mtu > 3) && ((app_env.max_mtu - 3) > notify_payload_mtu))
			{
				notify_payload_mtu = (app_env.max_mtu - 3);
				ble_att_mtu = notify_payload_mtu;
			}
			if(notify_payload_mtu >= 59)
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
				upData.DPID018Back = 0;
			}
		}

//	}
}


