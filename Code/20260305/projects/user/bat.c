#include "bat.h"
#include "app_gpio.h"
#include "aroma.h"

bat_t bat;

void BAT_Init(void)
{	
	bat.volRange = VOLTAGE_RANGE1;
}

/**
 * @brief  Configures the different system clocks.
 */
void RCC_ADC_Configuration(void)
{
	/* Enable peripheral clocks */
    /* Enable GPIOB clocks */
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_GPIOB, ENABLE);
    /* Enable ADC clocks */
    RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, ENABLE);
        
    RCC_ConfigAdcClk(RCC_ADCCLK_SRC_AUDIOPLL);

    /* enable ADC 4M clock */
    RCC_Enable_ADC_CLK_SRC_AUDIOPLL(ENABLE);
}

uint16_t ADC_Converse(uint16_t ch, uint32_t sz, uint16_t *buf)
{
    uint32_t i;
    volatile uint8_t  flag;
    uint32_t total = 0;
	
	ADC_ConfigChannel(ADC, ch);
	
    
    /* Execute ADC converse and get the conversion result */
    for (i=0; i<sz; i++)
    {
        /* ADC start */
			ADC_Enable(ADC, ENABLE);

        /* waiting interrupt flag */
			while(ADC_GetFlagStatus(ADC,ADC_FLAG_DONE) == RESET);
			ADC_ClearFlag(ADC,ADC_FLAG_DONE); 

        /* Get the conversion result */
        *buf++ = ADC_GetDat(ADC);
        /* Calculate the total value of the conversion result */
        total += ADC_GetDat(ADC);
    } 
    
    return (total / sz); // return average value
}

void BAT_Task(void)
{
	static uint8_t cntA,cntB,cntC;
	uint16_t ResultBuf[10];
	float tempA;
	
	if((power.status != POWER_OFF) && (bat.status == BAT_DISCHARGE))
	{
		tempA =  ADC_ConverValueToVoltage(ADC_Converse(ADC_CTRL_CH_5, 10 ,ResultBuf), ADC_CTRL_CH_5);
		bat.volage = (uint16_t)(tempA * 1.13);
		
		switch(bat.volRange)
		{
			case VOLTAGE_RANGE1:
			{
				if(bat.volage < VOLTAGE_RANGE2_VALUE)
				{
					if(++cntC > 5)
					{
						bat.volRange = VOLTAGE_RANGE2;
						cntC=0;
					}
				}
				else { cntC=0; }
			}
			break;
			case VOLTAGE_RANGE2:
			{
				if(bat.volage < VOLTAGE_RANGE1_VALUE )
				{
					if(++cntC > 5)
					{
						bat.volRange = VOLTAGE_LOW;
						cntC=0;
					}
				}
				else { cntC=0; }
			}
			break;
			case VOLTAGE_LOW:
			{
				if(bat.volage < VOLTAGE_LOW_VALUE)
				{
					if(++cntC > 5)
					{
						bat.volRange = VOLTAGE_STOP;
						cntC=0;
					}
				}
				else { cntC=0; }
			}
			break;
			case VOLTAGE_STOP:
			{
				if(bat.volage < VOLTAGE_STOP_VALUE)
				{
					if(++cntC > 50)
					{
						power.status = POWER_OFF;
						cntC=0;
					}
				}
				else { cntC=0; }
			}
			break;
			default:  break;	
		}
	}

	
	if(DC_IN_GET)
	{
//		PowerOffTime = 0;
		cntB = 0;
		if(++cntA > 5)
		{
			cntA = 0;
			if(bat.status != BAT_CHARGE)
			{
				bat.status = BAT_CHARGE; EN_WSL2309_SET;
				bat.volRange = VOLTAGE_RANGE1;
			}
		}
	}
  else
	{
		cntA = 0;
		if(++cntB > 5)
		{
			cntB = 0;
			if(bat.status != BAT_DISCHARGE)
			{
//				PowerOffTime = 0;
				bat.status = BAT_DISCHARGE; EN_WSL2309_CLR;
				bat.chargeFull = 0;
				
				if(bat.fullCnt > 300){bat.fullCnt -= 300;}
				else {bat.fullCnt = 0;}
			}
		}
	}
	
  if(bat.status == BAT_CHARGE)
	{
		if(airpump.SW) 
		{
			EN_WSL2309_CLR;
		}
		else 
		{
			EN_WSL2309_SET;
		}
		
//		if((!LED3_MONITOR) && (LED1_MONITOR))
//		{
//			if(++bat.fullCnt > 100)
//			{
//				bat.fullCnt = 100;
//				bat.chargeFull = 1;
//			}
//		}
//		else 
			if(bat.volage > 4100)
		{
			if(++bat.fullCnt > 60000)
			{
				bat.chargeFull = 1;
			}
		}
		
		if(bat.fullCnt > 100000)
		{
			bat.fullCnt = 100000;
		}
	}
	else
	{
		if(bat.fullCnt){ bat.fullCnt--; }
	}	
	
}

void Bat_Monitor(void)
{
	static uint16_t Led1Cnt,CntC;
//	static bool LED1_MONITOR_OLD;
		
	if(bat.status == BAT_CHARGE)
	{
//		if(++Led1Cnt > 25000) Led1Cnt = 25000;
//		
//		if(LED1_MONITOR_OLD != LED1_MONITOR)
//		{
//			LED1_MONITOR_OLD = LED1_MONITOR;
//			if(Led1Cnt < 10)
//			{
//				if(++CntC > 20000) {CntC = 0; Charge_Full_Flag = 1;}
//			}
//			else
//			{
//				CntC = 0; 
//			}
//			
//			Led1Cnt = 0;
//		}
//		if((!LED3_MONITOR) && (LED1_MONITOR))
//		{
//			if(++CntC > 20000) {CntC = 0; bat.chargeFull = 1;}
//		}
//		else{CntC = 0;}
	}
	else
	{
		Led1Cnt = 0; CntC = 0; 
	}
}

void BAT_Sleep_Config(void)
{
//	SYS_SET_IOCFG(IOP01CFG,SYS_IOCFG_P01_GPIO);				/*设置为GPIO模式*/	
//	GPIO_CONFIG_IO_MODE(GPIO0,GPIO_PIN_1,GPIO_MODE_INPUT);	
//	
//	SYS_SET_IOCFG(IOP04CFG,SYS_IOCFG_P04_GPIO);				/*设置为GPIO模式*/	
//	GPIO_CONFIG_IO_MODE(GPIO0,GPIO_PIN_4,GPIO_MODE_INPUT);	
}

