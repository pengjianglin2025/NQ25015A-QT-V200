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
	
	if((power.status == POWER_ON) || (!POWER_KEY_GET) || (power.offTime < 20)) return;
		
	ke_timer_clear(APP_5MS_EVT, TASK_APP);
	ke_timer_clear(APP_20MS_EVT, TASK_APP);
	ke_timer_clear(APP_100MS_EVT, TASK_APP);
	ke_timer_clear(APP_500MS_EVT, TASK_APP);
	ke_timer_clear(APP_1S_EVT, TASK_APP);
//	LedOff(LED1_PORT,LED1_PIN);
//	LedOff(LED2_PORT,LED2_PIN);
	RCC_Enable_ADC_CLK_SRC_AUDIOPLL(DISABLE);
	
	/* Configure PB.6 (ADC Channe5) as analog input --------*/
	GPIO_InitStructure.Pin       = GPIO_PIN_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_MODE_INPUT;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
	
	 SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);	
	  SysTick->LOAD = 0;
      SysTick->VAL  = 0;
	TIM_Enable(TIM3, DISABLE);
	
	ns_ble_disconnect();
	for(uint8_t i=0; i<10; i++)
	{
		rwip_schedule();
	}
	
	ns_ble_adv_stop();
	for(uint8_t i=0; i<10; i++)
	{
		rwip_schedule();
	}
	
	
	/*Initialize key as external line interrupt*/
	KeyInputExtiInit(KEY_INPUT_PORT, KEY_INPUT_PIN, ENABLE);
	
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
	if(!power.sleepAllow) return;
	
	ke_timer_set(APP_5MS_EVT, TASK_APP, 5);
	ke_timer_set(APP_20MS_EVT, TASK_APP, 20);
	ke_timer_set(APP_100MS_EVT, TASK_APP, 100);
	ke_timer_set(APP_500MS_EVT, TASK_APP, 500);
	ke_timer_set(APP_1S_EVT, TASK_APP, 1000);
	
//    /* enable ADC 4M clock */
//    RCC_Enable_ADC_CLK_SRC_AUDIOPLL(ENABLE);
	
	/* Configure PB.6 (ADC Channe5) as analog input --------*/
	GPIO_InitStructure.Pin       = GPIO_PIN_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_MODE_ANALOG;
	GPIO_InitPeripheral(GPIOB, &GPIO_InitStructure);
	
	
	KeyInputExtiInit(KEY_INPUT_PORT, KEY_INPUT_PIN, DISABLE);
	
  SysTick_Config(SystemCoreClock/SYSTICK_100US);
	SysTick->CTRL |= (SysTick_CTRL_ENABLE_Msk | SysTick_CTRL_TICKINT_Msk);
	TIM_Enable(TIM3, ENABLE);

	
	power.sleepAllow = 0;
	power.offTime = 0;
	
	ns_ble_adv_start();
	for(uint8_t i=0; i<10; i++)
	{
		rwip_schedule();
	}
	
}



/**
 * @}
 */

