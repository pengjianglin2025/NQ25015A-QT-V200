#include "clock.h"

const uint32_t hourToSec[24]={0,3600,7200,10800,14400,18000,21600,25200,28800,32400,
                                 36000,39600,43200,46800,50400,54000,57600,61200,64800,68400,
                                 72000,75600,79200,82800};
const uint16_t minToSec[60]={0,60,120,180,240,300,360,420,480,540,
	                                 600,660,720,780,840,900,960,1020,1080,1140,
	                                 1200,1260,1320,1380,1440,1500,1560,1620,1680,1740,
	                                 1800,1860,1920,1980,2040,2100,2160,2220,2280,2340,
	                                 2400,2460,2520,2580,2640,2700,2760,2820,2880,2940,
	                                 3000,3060,3120,3180,3240,3300,3360,3420,3480,3540
	                                 };
const uint16_t hourToMin[24] = {0,60,120,180,240,300,360,420,480,540,600,
                                   660,720,780,840,900,960,1020,1080,1140,1200,
                                   1260,1320,1380,};
clock_t clockNow;

RTC_DateType RTC_DateStructure;
RTC_DateType RTC_DateDefault;
RTC_TimeType RTC_TimeStructure;
RTC_TimeType RTC_TimeDefault;
RTC_InitType RTC_InitStructure;
uint32_t SynchPrediv, AsynchPrediv;

/**
 * @brief  RTC initalize default value.
 */
void RTC_DateAndTimeDefaultVale(void)
{ // Date
    RTC_DateDefault.WeekDay = 3;
    RTC_DateDefault.Date    = 20;
    RTC_DateDefault.Month   = 11;
    RTC_DateDefault.Year    = 19;
    // Time
    RTC_TimeDefault.H12     = RTC_AM_H12;
    RTC_TimeDefault.Hours   = 4;
    RTC_TimeDefault.Minutes = 5;
    RTC_TimeDefault.Seconds = 1;
}

/**
 * @brief  Configures RTC.
 *   Configure the RTC peripheral by selecting the clock source
 */

void RTC_CLKSourceConfig(uint8_t ClkSrc)
{
    assert_param(IS_CLKSRC_VALUE(ClkSrc));

    /* Enable the PWR clock */
    RCC_EnableAPB1PeriphClk(RCC_APB1_PERIPH_PWR, ENABLE);
    RCC_EnableAPB2PeriphClk(RCC_APB2_PERIPH_AFIO, ENABLE);

    /* Disable RTC clock */
    RCC_EnableRtcClk(DISABLE);

    if (ClkSrc == 0x01)
    {
        NS_LOG_INFO("\r\n RTC_ClkSrc Is Set LSE! \r\n");

#if (_TEST_LSE_BYPASS_)
        RCC_ConfigLse(RCC_LSE_BYPASS);
#else
        RCC_ConfigLse(RCC_LSE_ENABLE);
#endif

        while (RCC_GetFlagStatus(RCC_LSCTRL_FLAG_LSERD) == RESET)
        {
        }
        
        RCC_ConfigLSXSEL(RCC_RTCCLK_SRC_LSE);
        RCC_EnableLsi(DISABLE);

        SynchPrediv  = 0xFF; // 32.768KHz
        AsynchPrediv = 0x7F; // value range: 0-7F
    }
    else if (ClkSrc == 0x02)
    {
        NS_LOG_INFO("\r\n RTC_ClkSrc Is Set LSI! \r\n");

        /* Enable the LSI OSC */
        RCC_EnableLsi(ENABLE);
        while (RCC_GetFlagStatus(RCC_LSCTRL_FLAG_LSIRD) == RESET)
        {
        }

        RCC_ConfigLSXSEL(RCC_RTCCLK_SRC_LSI);

        SynchPrediv  = 0xFF; // 32.768KHz
        AsynchPrediv = 0x7F;  // value range: 0-7F
    }
    else
    {
        NS_LOG_INFO("\r\n RTC_ClkSrc Value is error! \r\n");
    }

    /* Enable the RTC Clock */
    RCC_EnableRtcClk(ENABLE);
    RTC_WaitForSynchro();
}

/**
 * @brief  RTC prescaler config.
 */
void RTC_PrescalerConfig(void)
{
    /* Configure the RTC data register and RTC prescaler */
    RTC_InitStructure.RTC_AsynchPrediv = AsynchPrediv;
    RTC_InitStructure.RTC_SynchPrediv  = SynchPrediv;
    RTC_InitStructure.RTC_HourFormat   = RTC_24HOUR_FORMAT;

    /* Check on RTC init */
    if (RTC_Init(&RTC_InitStructure) == ERROR)
    {
        NS_LOG_INFO("\r\n //******* RTC Prescaler Config failed **********// \r\n");
    }
}

/**
 * @brief  Display the current Date on the Hyperterminal.
 */
void RTC_DateShow(void)
{
    /* Get the current Date */
    RTC_GetDate(RTC_FORMAT_BIN, &RTC_DateStructure);
    //NS_LOG_INFO("\n\r //=========== Current Date Display ==============// \n\r");
//    NS_LOG_INFO("\n\r The current date (WeekDay-Date-Month-Year) is :  %0.2d-%0.2d-%0.2d-%0.2d ",
//             RTC_DateStructure.WeekDay,
//             RTC_DateStructure.Date,
//             RTC_DateStructure.Month,
//             RTC_DateStructure.Year);
}

/**
 * @brief  Display the current time on the Hyperterminal.
 */
void RTC_TimeShow(void)
{
	static uint8_t SecondOld,cntA;
	
    /* Get the current Time and Date */
    RTC_GetTime(RTC_FORMAT_BIN, &RTC_TimeStructure);
	
	clockNow.totalSec = hourToSec[RTC_TimeStructure.Hours] + minToSec[RTC_TimeStructure.Minutes] + RTC_TimeStructure.Seconds;
	
	if(SecondOld != RTC_TimeStructure.Seconds)
	{
		cntA = 0;
		clockNow.rtcStatus = S_NORMAL;
		SecondOld = RTC_TimeStructure.Seconds;
//		PowerOnTime++;
	}
	else
	{
		if(++cntA > 30)
		{
			cntA = 0;
			clockNow.rtcStatus = S_ERROR;
		}
	}
	
    //NS_LOG_INFO("\n\r //============ Current Time Display ===============// \n\r");
//    NS_LOG_INFO("\n\r The current time (Hour-Minute-Second) is :  %0.2d:%0.2d:%0.2d \n\r",
//             RTC_TimeStructure.Hours,
//             RTC_TimeStructure.Minutes,
//             RTC_TimeStructure.Seconds);
    /* Unfreeze the RTC DAT Register */
    (void)RTC->DATE;
}
/**
 * @brief  RTC date regulate with the default value.
 */
ErrorStatus RTC_DateRegulate(void)
{
    uint32_t tmp_hh = 0xFF, tmp_mm = 0xFF, tmp_ss = 0xFF;
    NS_LOG_INFO("\n\r //=============Date Settings================// \n\r");

    NS_LOG_INFO("\n\r Please Set WeekDay (01-07) \n\r");
    tmp_hh = RTC_DateDefault.WeekDay;
    if (tmp_hh == 0xff)
    {
        return ERROR;
    }
    else
    {
        RTC_DateStructure.WeekDay = tmp_hh;
    }
    NS_LOG_INFO(": %0.2d\n\r", tmp_hh);

    tmp_hh = 0xFF;
    NS_LOG_INFO("\n\r Please Set Date (01-31) \n\r");
    tmp_hh = RTC_DateDefault.Date;
    if (tmp_hh == 0xff)
    {
        return ERROR;
    }
    else
    {
        RTC_DateStructure.Date = tmp_hh;
    }
    NS_LOG_INFO(": %0.2d\n\r", tmp_hh);

    NS_LOG_INFO("\n\r Please Set Month (01-12)\n\r");
    tmp_mm = RTC_DateDefault.Month;
    if (tmp_mm == 0xff)
    {
        return ERROR;
    }
    else
    {
        RTC_DateStructure.Month = tmp_mm;
    }
    NS_LOG_INFO(": %0.2d\n\r", tmp_mm);

    NS_LOG_INFO("\n\r Please Set Year (00-99)\n\r");
    tmp_ss = RTC_DateDefault.Year;
    if (tmp_ss == 0xff)
    {
        return ERROR;
    }
    else
    {
        RTC_DateStructure.Year = tmp_ss;
    }
    NS_LOG_INFO(": %0.2d\n\r", tmp_ss);

    /* Configure the RTC date register */
    if (RTC_SetDate(RTC_FORMAT_BIN, &RTC_DateStructure) == ERROR)
    {
        NS_LOG_INFO("\n\r>> !! RTC Set Date failed. !! <<\n\r");
        return ERROR;
    }
    else
    {
        NS_LOG_INFO("\n\r>> !! RTC Set Date success. !! <<\n\r");
        RTC_DateShow();
        return SUCCESS;
    }
}
/**
 * @brief  RTC time regulate with the default value.
 */
ErrorStatus RTC_TimeRegulate(void)
{
    uint32_t tmp_hh = 0xFF, tmp_mm = 0xFF, tmp_ss = 0xFF;
    NS_LOG_INFO("\n\r //==============Time Settings=================// \n\r");

    RTC_TimeStructure.H12 = RTC_TimeDefault.H12;

    NS_LOG_INFO("\n\r Please Set Hours \n\r");
    tmp_hh = RTC_TimeDefault.Hours;
    if (tmp_hh == 0xff)
    {
        return ERROR;
    }
    else
    {
        RTC_TimeStructure.Hours = tmp_hh;
    }
    NS_LOG_INFO(": %0.2d\n\r", tmp_hh);

    NS_LOG_INFO("\n\r Please Set Minutes \n\r");
    tmp_mm = RTC_TimeDefault.Minutes;
    if (tmp_mm == 0xff)
    {
        return ERROR;
    }
    else
    {
        RTC_TimeStructure.Minutes = tmp_mm;
    }
    NS_LOG_INFO(": %0.2d\n\r", tmp_mm);

    NS_LOG_INFO("\n\r Please Set Seconds \n\r");
    tmp_ss = RTC_TimeDefault.Seconds;
    if (tmp_ss == 0xff)
    {
        return ERROR;
    }
    else
    {
        RTC_TimeStructure.Seconds = tmp_ss;
    }
    NS_LOG_INFO(": %0.2d\n\r", tmp_ss);

    /* Configure the RTC time register */
    if (RTC_ConfigTime(RTC_FORMAT_BIN, &RTC_TimeStructure) == ERROR)
    {
        NS_LOG_INFO("\n\r>> !! RTC Set Time failed. !! <<\n\r");
        return ERROR;
    }
    else
    {
        NS_LOG_INFO("\n\r>> !! RTC Set Time success. !! <<\n\r");
        RTC_TimeShow();
        return SUCCESS;
    }
}

