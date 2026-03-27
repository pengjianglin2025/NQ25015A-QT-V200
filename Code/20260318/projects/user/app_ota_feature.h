#ifndef __APP_OTA_FEATURE_H__
#define __APP_OTA_FEATURE_H__

#include "protocol.h"

#define APP_OTA_CMDWORD                 (0x30U)
#define APP_OTA_SUBCMD_QUERY_INFO       (0x00U)
#define APP_OTA_SUBCMD_ENTER_BOOT       (0x01U)

void app_ota_feature_init(void);
bool app_ota_feature_handle_command(const Message_t* rx_msg);
bool app_ota_feature_prepare_tx(Message_t* tx_msg, uint16_t* tx_length);
void app_ota_feature_poll(void);

#endif
