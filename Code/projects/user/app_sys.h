#ifndef __APP_SYS_H__
#define __APP_SYS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* 系统层统一的电源状态切换入口。 */
void app_sys_power_set(powerS_t new_status);

/* 统一应用关机态硬件输出，供睡眠前复用。 */
void app_sys_power_apply_off_outputs(void);

/* 统一应用开机态关键硬件恢复。 */
void app_sys_power_apply_on_outputs(void);

#ifdef __cplusplus
}
#endif

#endif
