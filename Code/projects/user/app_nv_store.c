#include "app_nv_store.h"

#include <string.h>

#include "iap.h"
#include "app_user_config.h"
#include "n32wb03x_qflash.h"

#define APP_NV_STORE_MAGIC          (0x4150564EU)
#define APP_NV_STORE_VERSION        (0x0001U)
#define APP_NV_STORE_PRIMARY_ADDR   ((uint32_t)IAP_START_ADDR)   /* 新参数区地址 */
#define APP_NV_STORE_LEGACY_ADDR    (0x01020000UL)              /* 旧版本参数区地址 */
#define APP_NV_STORE_RETRY_MAX      (3U)

#pragma pack(1)
typedef struct
{
    uint32_t magic;
    uint16_t version;
    uint16_t payload_len;
    uint32_t checksum;
    uint32_t sequence;
}AppNvHeader_t;
#pragma pack()

typedef struct
{
    AppNvHeader_t header;
    uint8_t payload[IAP_DATA_MAX];
}AppNvRecord_t;

static uint32_t s_app_nv_sequence = 0U;

/* 轻量校验值，用于判断记录是否完整有效。 */
static uint32_t app_nv_store_checksum(const uint8_t* data, uint16_t len)
{
    uint32_t checksum = 2166136261UL;
    uint16_t index;

    for(index = 0; index < len; index++)
    {
        checksum ^= data[index];
        checksum *= 16777619UL;
    }

    return checksum;
}

static void app_nv_store_record_build(AppNvRecord_t* record, const uint8_t* payload, uint16_t payload_len)
{
    memset(record, 0, sizeof(AppNvRecord_t));
    record->header.magic = APP_NV_STORE_MAGIC;
    record->header.version = APP_NV_STORE_VERSION;
    record->header.payload_len = payload_len;
    record->header.sequence = ++s_app_nv_sequence;
    memcpy(record->payload, payload, payload_len);
    record->header.checksum = app_nv_store_checksum(record->payload, payload_len);
}

static bool app_nv_store_record_valid(const AppNvRecord_t* record, uint16_t payload_len)
{
    if(record->header.magic != APP_NV_STORE_MAGIC)
    {
        return false;
    }

    if(record->header.version != APP_NV_STORE_VERSION)
    {
        return false;
    }

    if(record->header.payload_len != payload_len)
    {
        return false;
    }

    if(record->header.checksum != app_nv_store_checksum(record->payload, payload_len))
    {
        return false;
    }

    return true;
}

static bool app_nv_store_read_record(AppNvRecord_t* record, uint16_t payload_len)
{
    memset(record, 0, sizeof(AppNvRecord_t));
    Qflash_Read(APP_NV_STORE_PRIMARY_ADDR, (uint8_t*)record, sizeof(AppNvHeader_t) + payload_len);
    if(!app_nv_store_record_valid(record, payload_len))
    {
        return false;
    }

    if(record->header.sequence > s_app_nv_sequence)
    {
        s_app_nv_sequence = record->header.sequence;
    }

    return true;
}

/* 兼容读取旧存储地址，便于升级后自动迁移参数。 */
static bool app_nv_store_read_legacy(uint8_t* payload, uint16_t payload_len)
{
    memset(payload, 0, payload_len);
    Qflash_Read(APP_NV_STORE_LEGACY_ADDR, payload, payload_len);
    return (payload[0] == IAP_WRITE_OK);
}

bool app_nv_store_read_payload(uint8_t* payload, uint16_t payload_len)
{
    AppNvRecord_t record;

    if(payload_len > IAP_DATA_MAX)
    {
        return false;
    }

    if(!app_nv_store_read_record(&record, payload_len))
    {
        memset(payload, 0, payload_len);
        return false;
    }

    memcpy(payload, record.payload, payload_len);
    return true;
}

/* 写入新格式记录，并做读回校验。 */
bool app_nv_store_write_payload(const uint8_t* payload, uint16_t payload_len)
{
    AppNvRecord_t write_record;
    AppNvRecord_t read_record;
    uint8_t retry;

    if(payload_len > IAP_DATA_MAX)
    {
        return false;
    }

    app_nv_store_record_build(&write_record, payload, payload_len);

    for(retry = 0; retry < APP_NV_STORE_RETRY_MAX; retry++)
    {
        Qflash_Erase_Sector(APP_NV_STORE_PRIMARY_ADDR);
        Qflash_Write(APP_NV_STORE_PRIMARY_ADDR, (uint8_t*)&write_record, sizeof(AppNvHeader_t) + payload_len);

        if(app_nv_store_read_record(&read_record, payload_len) &&
           (memcmp(&read_record, &write_record, sizeof(AppNvHeader_t) + payload_len) == 0))
        {
            return true;
        }
    }

    return false;
}

bool app_nv_store_load_legacy_compatible(uint8_t* payload, uint16_t payload_len)
{
    if(app_nv_store_read_payload(payload, payload_len))
    {
        return true;
    }

    if(app_nv_store_read_legacy(payload, payload_len))
    {
        (void)app_nv_store_write_payload(payload, payload_len);
        return true;
    }

    return false;
}

bool app_nv_store_is_payload_equal(const uint8_t* payload, uint16_t payload_len)
{
    uint8_t current_payload[IAP_DATA_MAX];

    if(payload_len > IAP_DATA_MAX)
    {
        return false;
    }

    if(!app_nv_store_read_payload(current_payload, payload_len))
    {
        return false;
    }

    return (memcmp(current_payload, payload, payload_len) == 0);
}


