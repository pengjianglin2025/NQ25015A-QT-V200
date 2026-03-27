#include "app_ota_feature.h"

#if (APP_OTA_FEATURE_ENABLE)

#include "app_user_config.h"
#include "ns_dfu_boot.h"
#include "ns_ble.h"
#include "ns_delay.h"

#define APP_OTA_PROTOCOL_VERSION            (0x01U)
#define APP_OTA_STATUS_OK                   (0x00U)
#define APP_OTA_STATUS_UNSUPPORTED          (0x01U)
#define APP_OTA_STATUS_INVALID_PARAM        (0x02U)
#define APP_OTA_STATUS_BUSY                 (0x03U)
#define APP_OTA_JUMP_DELAY_TICKS            (3U)

static uint8_t s_ota_rsp_buffer[20];
static uint16_t s_ota_rsp_length = 0;
static bool s_ota_rsp_pending = false;
static bool s_ota_jump_pending = false;
static uint8_t s_ota_jump_delay_ticks = 0;

static void app_ota_feature_queue_simple_rsp(uint8_t subcmd, uint8_t status)
{
    s_ota_rsp_buffer[0] = subcmd;
    s_ota_rsp_buffer[1] = status;
    s_ota_rsp_length = 2U;
    s_ota_rsp_pending = true;
}

static void app_ota_feature_u32_to_buf(uint8_t* buffer, uint32_t value)
{
    buffer[0] = (uint8_t)((value >> 24) & 0xFFU);
    buffer[1] = (uint8_t)((value >> 16) & 0xFFU);
    buffer[2] = (uint8_t)((value >> 8) & 0xFFU);
    buffer[3] = (uint8_t)(value & 0xFFU);
}

static void app_ota_feature_queue_info_rsp(void)
{
    const NS_Bank_t* current_bank;
    uint8_t current_bank_id;

    if (CURRENT_APP_START_ADDRESS == NS_APP2_START_ADDRESS)
    {
        current_bank = &ns_bootsetting.app2;
        current_bank_id = 2U;
    }
    else
    {
        current_bank = &ns_bootsetting.app1;
        current_bank_id = 1U;
    }

    s_ota_rsp_buffer[0] = APP_OTA_SUBCMD_QUERY_INFO;
    s_ota_rsp_buffer[1] = APP_OTA_STATUS_OK;
    s_ota_rsp_buffer[2] = APP_OTA_PROTOCOL_VERSION;
    s_ota_rsp_buffer[3] = current_bank_id;
    app_ota_feature_u32_to_buf(&s_ota_rsp_buffer[4], CURRENT_APP_START_ADDRESS);
    app_ota_feature_u32_to_buf(&s_ota_rsp_buffer[8], current_bank->version);
    app_ota_feature_u32_to_buf(&s_ota_rsp_buffer[12], ns_bootsetting.ImageUpdate.version);
    s_ota_rsp_buffer[16] = (ns_bootsetting.ImageUpdate.activation == NS_BOOTSETTING_ACTIVATION_YES) ? 1U : 0U;
    s_ota_rsp_buffer[17] = (uint8_t)(APP_OTA_FEATURE_ENABLE ? 1U : 0U);
    s_ota_rsp_length = 18U;
    s_ota_rsp_pending = true;
}

void app_ota_feature_init(void)
{
    s_ota_rsp_length = 0U;
    s_ota_rsp_pending = false;
    s_ota_jump_pending = false;
    s_ota_jump_delay_ticks = 0U;
}

bool app_ota_feature_handle_command(const Message_t* rx_msg)
{
    uint8_t subcmd;

    if (rx_msg == 0)
    {
        return false;
    }

    if (rx_msg->CommandWord != APP_OTA_CMDWORD)
    {
        return false;
    }

    subcmd = rx_msg->FunctionalData[0];

    switch (subcmd)
    {
        case APP_OTA_SUBCMD_QUERY_INFO:
            app_ota_feature_queue_info_rsp();
            break;

        case APP_OTA_SUBCMD_ENTER_BOOT:
            if (s_ota_jump_pending)
            {
                app_ota_feature_queue_simple_rsp(subcmd, APP_OTA_STATUS_BUSY);
            }
            else
            {
                s_ota_jump_pending = true;
                s_ota_jump_delay_ticks = APP_OTA_JUMP_DELAY_TICKS;
                app_ota_feature_queue_simple_rsp(subcmd, APP_OTA_STATUS_OK);
            }
            break;

        default:
            app_ota_feature_queue_simple_rsp(subcmd, APP_OTA_STATUS_UNSUPPORTED);
            break;
    }

    return true;
}

bool app_ota_feature_prepare_tx(Message_t* tx_msg, uint16_t* tx_length)
{
    if ((!s_ota_rsp_pending) || (tx_msg == 0) || (tx_length == 0))
    {
        return false;
    }

    memset(tx_msg->Buffer, 0, sizeof(tx_msg->Buffer));
    tx_msg->CommandWord = APP_OTA_CMDWORD;
    memcpy(tx_msg->FunctionalData, s_ota_rsp_buffer, s_ota_rsp_length);
    *tx_length = s_ota_rsp_length;
    s_ota_rsp_pending = false;

    return true;
}

void app_ota_feature_poll(void)
{
    if (!s_ota_jump_pending)
    {
        return;
    }

    if (s_ota_jump_delay_ticks > 0U)
    {
        s_ota_jump_delay_ticks--;
        return;
    }

    ns_ble_disconnect();
    delay_n_10us(10000U);
    ns_ble_adv_stop();
    delay_n_10us(10000U);
    ns_dfu_boot_jump(NS_MASTERBOOT_START_ADDRESS);
}

#else

void app_ota_feature_init(void)
{
}

bool app_ota_feature_handle_command(const Message_t* rx_msg)
{
    (void)rx_msg;
    return false;
}

bool app_ota_feature_prepare_tx(Message_t* tx_msg, uint16_t* tx_length)
{
    (void)tx_msg;
    (void)tx_length;
    return false;
}

void app_ota_feature_poll(void)
{
}

#endif
