/*****************************************************************************
 * Copyright (c) 2019, Nations Technologies Inc.
 *
 * All rights reserved.
 * ****************************************************************************
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Nations' name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY NATIONS "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL NATIONS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ****************************************************************************/

/**
 * @file main.c
 * @author Nations Firmware Team
 * @version v1.0.1
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */

/** @addtogroup 
 * @{
 */
 
/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "app_ota_feature.h"
#include "app_sys.h"

#if  (CFG_APP_NS_IUS)
#include "ns_dfu_boot.h"
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define DEMO_STRING  "\r\n Nations raw data transfer server(16bit UUID) demo \r\n"

/* Private constants ---------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

power_t power;
WStatus_t globalWorkState = FULL_WORKING;
function_t function;

/**
 * @brief  main function
 * @param   
 * @return 
 * @note   Note
 */
int main(void)
{
	//for hold the SWD before sleep
	delay_n_10us(200*1000);
	
	power.status = POWER_ON;
	
	NS_LOG_INIT();
	gpio_init();
	
    /* Get SystemCoreClock */
    SystemCoreClockUpdate();
    /* Config 100us SysTick  */
    SysTick_Config(SystemCoreClock/SYSTICK_100US);
	
	RCC_ADC_Configuration();
	ADC_EnableBypassFilter(ADC, ENABLE);
	
	/* RTC date time alarm default value*/
	RTC_DateAndTimeDefaultVale();

	/* RTC clock source select 1:LSE 2:LSI*/
	RTC_CLKSourceConfig(1);
	RTC_PrescalerConfig();

	/* Adjust time by values entered by the user on the hyperterminal */
	/* Disable the RTC Wakeup Interrupt and delay more than 100us before set data and time */
	RTC_ConfigInt(RTC_INT_WUT, DISABLE);
	RTC_EnableWakeUp(DISABLE);
	RTC_DateRegulate();
	RTC_TimeRegulate();
	
	
	WorkInit();
	Key_Init();
	oil_init(OIL_150ml, DEFAULT_OIL_CONSUME_SPEED_8);

    #if  (CFG_APP_NS_IUS)
    if(CURRENT_APP_START_ADDRESS == NS_APP1_START_ADDRESS){
        NS_LOG_INFO("application 1 start new ...\r\n");
    }else if(CURRENT_APP_START_ADDRESS == NS_APP2_START_ADDRESS){
        NS_LOG_INFO("application 2 start new ...\r\n");
    }
    #endif
    app_ble_init();
    NS_LOG_INFO(DEMO_STRING);

    // periph init 
//    LedInit(LED1_PORT,LED1_PIN);  // power led
//    LedInit(LED2_PORT,LED2_PIN);  //connection state
//    LedOn(LED1_PORT,LED1_PIN);    
//    app_usart_dma_enable(ENABLE);
//    //init text
//    usart_tx_dma_send((uint8_t*)DEMO_STRING, sizeof(DEMO_STRING)); 

//    delay_n_10us(500);
//    //disable usart for enter sleep
//    app_usart_dma_enable(DISABLE);

		Qflash_Init();
		Iap_Read();
		app_ota_feature_init();
		/* 需求：每次上电后默认进入运行状态，不沿用上次关机时的开关态。 */
		aroma.en = ON;
	
		function.CheckStyle = OIL_CALCULATE;
		function.moto = 0;
		function.atmosphereLight = 0;
		function.quantityOfElectricity = 0;
		function.humanBodyInduction = 0;
		function.fan = 0;
		function.keyLock = 0;
		function.concentrationNum = 3;
    
    while (1)
    {
        /*schedule all pending events*/
        rwip_schedule();
        ns_sleep();

    }
}

/**
 * @brief  user handle before enter sleep mode
 * @param  
 * @return 
 * @note   
 */
void app_sleep_prepare_proc(void)
{
	
	GPIO_InitType GPIO_InitStructure;
	
	/* 仅在逻辑关机状态下准备深睡，避免误触发休眠流程。 */
	if((power.status == POWER_ON) || (!POWER_KEY_GET) || (power.offTime < 20) || (CHARGING_GET)) return;
		
	ke_timer_clear(APP_5MS_EVT, TASK_APP);
	ke_timer_clear(APP_20MS_EVT, TASK_APP);
	ke_timer_clear(APP_100MS_EVT, TASK_APP);
	ke_timer_clear(APP_500MS_EVT, TASK_APP);
	ke_timer_clear(APP_1S_EVT, TASK_APP);
//	LedOff(LED1_PORT,LED1_PIN);
//	LedOff(LED2_PORT,LED2_PIN);

	/* 睡前统一收口关机态外设输出。 */
	app_sys_power_apply_off_outputs();

	ADC_Enable(ADC, DISABLE);
	RCC_EnableAHBPeriphClk(RCC_AHB_PERIPH_ADC, DISABLE);
	RCC_Enable_ADC_CLK_SRC_AUDIOPLL(DISABLE);
	
	/* Configure PB.6 (ADC Channe5) as analog input --------*/
	GPIO_InitStructure.Pin       = GPIO_PIN_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_MODE_ANALOG;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
	
	 SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);	
	  SysTick->LOAD = 0;
      SysTick->VAL  = 0;
	/* 睡前关闭 TIM3，避免 PWM 定时器继续运行带来额外电流。 */
	TIM_Enable(TIM3, DISABLE);
	
	/* 睡眠入口只保留入睡相关动作，BLE断连/停广播在关机状态切换时一次性完成。 */

	
	/*Initialize key as external line interrupt*/
	KeyInputExtiInit(KEY_INPUT_PORT, KEY_INPUT_PIN, ENABLE);
	ChargingInputExtiInit(ENABLE);
	
	power.sleepAllow = 1;
}

/**
 * @brief  user handle after wake up from sleep mode
 * @param  
 * @return 
 * @note   
 */
void app_sleep_resume_proc(void)
{
	
	GPIO_InitType GPIO_InitStructure;
	bool need_runtime_recover;
	if(!power.sleepAllow) return;
	
	/* 根据唤醒源决定是否恢复系统运行态与软件定时器。 */
		/* 直接按 DC_IN 判断是否恢复到运行态或关机充电显示态。 */
	/* PA6 当前按“正在充电”语义参与恢复判断，而非单纯插电存在。 */
	need_runtime_recover = (power.status == POWER_ON) || (!POWER_KEY_GET) || (CHARGING_GET);
	
	/* Configure PB.6 (ADC Channe5) as analog input --------*/
	GPIO_InitStructure.Pin       = GPIO_PIN_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_MODE_ANALOG;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
	
		if(need_runtime_recover)
	{
		/* 需要恢复运行态时释放 STDBY，并恢复外设工作。 */
		NU1680_STDBY_CLR;
		/* 唤醒后恢复软件定时器。 */
		ke_timer_set(APP_5MS_EVT, TASK_APP, 5);
		ke_timer_set(APP_20MS_EVT, TASK_APP, 20);
		ke_timer_set(APP_100MS_EVT, TASK_APP, 100);
		ke_timer_set(APP_500MS_EVT, TASK_APP, 500);
		ke_timer_set(APP_1S_EVT, TASK_APP, 1000);
		
		KeyInputExtiInit(KEY_INPUT_PORT, KEY_INPUT_PIN, DISABLE);
		ChargingInputExtiInit(DISABLE);
		
		SystemCoreClockUpdate();
    SysTick_Config(SystemCoreClock/SYSTICK_100US);
		SysTick->CTRL |= (SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
		TIM_Enable(TIM3, ENABLE);
	}
	else
	{
		/* 不需要恢复运行态时，保持 NU1680 待机控制。 */
		NU1680_STDBY_SET;
		/* 保持最小运行状态时，不恢复系统时钟节拍。 */
		SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
		SysTick->LOAD = 0;
		SysTick->VAL  = 0;
		/* 睡前关闭 TIM3，避免 PWM 定时器继续运行带来额外电流。 */
	TIM_Enable(TIM3, DISABLE);
	}

	
	power.sleepAllow = 0;
	power.offTime = 0;
	
	/* 广播只应在开机状态下恢复。 */
	if(power.status == POWER_ON)
	{
		ns_ble_adv_start();
		for(uint8_t i=0; i<10; i++)
		{
			rwip_schedule();
		}
	}
	
}



/**
 * @}
 */










