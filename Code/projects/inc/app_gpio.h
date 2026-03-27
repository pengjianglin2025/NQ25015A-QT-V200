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
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAYS OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ****************************************************************************/

/**
 * @file app_gpio.h
 * @author Nations Firmware Team
 * @version v1.0.0
 *
 * @copyright Copyright (c) 2019, Nations Technologies Inc. All rights reserved.
 */
#ifndef __APP_GPIO_H__
#define __APP_GPIO_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "n32wb03x.h"
    
#define LED1_PORT GPIOB
#define LED1_PIN  GPIO_PIN_0
#define LED2_PORT GPIOA
#define LED2_PIN  GPIO_PIN_5


#define KEY_INPUT_PORT        GPIOB
#define KEY_INPUT_PIN         GPIO_PIN_1
#define KEY_INPUT_EXTI_LINE   EXTI_LINE1
#define KEY_INPUT_PORT_SOURCE GPIOB_PORT_SOURCE
#define KEY_INPUT_PIN_SOURCE  GPIO_PIN_SOURCE1
#define KEY_INPUT_IRQn        EXTI0_1_IRQn

#define POWER_KEY_LEVEL_GET     GPIO_ReadInputDataBit(GPIOB, GPIO_PIN_1)
/* PA6 当前语义为“正在充电”检测，高电平表示充电进行中。 */
#define CHARGE_ACTIVE_LEVEL_GET    GPIO_ReadInputDataBit(GPIOA, GPIO_PIN_6)


#define CHARGING_INPUT_PORT        GPIOA
#define CHARGING_INPUT_PIN         GPIO_PIN_6
#define CHARGING_INPUT_EXTI_LINE   EXTI_LINE2
#define CHARGING_INPUT_PORT_SOURCE GPIOA_PORT_SOURCE
#define CHARGING_INPUT_PIN_SOURCE  GPIO_PIN_SOURCE6
#define CHARGING_INPUT_IRQn        EXTI2_3_IRQn

/* 电源路径待机控制/充电状态检测脚 */
#define POWER_CTRL_STDBY_SET       GPIO_SetBits(GPIOA, GPIO_PIN_0)
#define POWER_CTRL_STDBY_CLR       GPIO_ResetBits(GPIOA, GPIO_PIN_0)
#define POWER_CHARGE_STATE_GET     GPIO_ReadInputDataBit(GPIOA, GPIO_PIN_1)
#define BOOST_6291_EN_SET          GPIO_SetBits(GPIOB, GPIO_PIN_12)  
#define BOOST_6291_EN_CLR          GPIO_ResetBits(GPIOB, GPIO_PIN_12)
#define CHARGE_CTRL_EN_SET         GPIO_SetBits(GPIOB, GPIO_PIN_0)  
#define CHARGE_CTRL_EN_CLR         GPIO_ResetBits(GPIOB, GPIO_PIN_0)

#define AIRPUMP_OUT_ON   GPIO_SetBits(GPIOB, GPIO_PIN_5)
#define AIRPUMP_OUT_OFF  GPIO_ResetBits(GPIOB, GPIO_PIN_5)


#define CHARGE_EN_SET    GPIO_SetBits(GPIOB, GPIO_PIN_0);  
#define CHARGE_EN_CLR    GPIO_ResetBits(GPIOB, GPIO_PIN_0)


#define LED_RED_SET     GPIO_SetBits(GPIOB, GPIO_PIN_2) 
#define LED_GREEN_SET   GPIO_SetBits(GPIOB, GPIO_PIN_3) 
#define LED_BLUE_SET    GPIO_SetBits(GPIOB, GPIO_PIN_4)  
#define LED_RED_CLR     GPIO_ResetBits(GPIOB, GPIO_PIN_2) 
#define LED_GREEN_CLR   GPIO_ResetBits(GPIOB, GPIO_PIN_3)
#define LED_BLUE_CLR    GPIO_ResetBits(GPIOB, GPIO_PIN_4)  

#define LED_COM2_SET    GPIO_SetBits(GPIOB, GPIO_PIN_10)  
#define LED_COM2_CLR    GPIO_ResetBits(GPIOB, GPIO_PIN_10) 
#define LED_COM1_SET    GPIO_SetBits(GPIOB, GPIO_PIN_7)  
#define LED_COM1_CLR    GPIO_ResetBits(GPIOB, GPIO_PIN_7) 

#define WF433_DATA_GET       GPIO_ReadInputDataBit(GPIOA, GPIO_PIN_2) //433数据引脚  
#define WF433_EN_SW_SET      GPIO_SetBits(GPIOA, GPIO_PIN_3) //433使能
#define WF433_EN_SW_CLR      GPIO_ResetBits(GPIOA, GPIO_PIN_3)


void gpio_init(void);
//void LedOn(GPIO_Module* GPIOx, uint16_t Pin);
//void LedOff(GPIO_Module* GPIOx, uint16_t Pin);
//void LedBlink(GPIO_Module* GPIOx, uint16_t Pin);
void KeyInputExtiInit(GPIO_Module* GPIOx, uint16_t Pin, FunctionalState Cmd);
void ChargingInputExtiInit(FunctionalState Cmd);

#ifdef __cplusplus
}
#endif

#endif /* __APP_GPIO_H__ */
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */



