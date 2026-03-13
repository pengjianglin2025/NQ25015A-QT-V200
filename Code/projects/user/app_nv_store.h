#ifndef __APP_NV_STORE_H__
#define __APP_NV_STORE_H__

#include <stdbool.h>
#include <stdint.h>

bool app_nv_store_load_legacy_compatible(uint8_t* payload, uint16_t payload_len);
bool app_nv_store_read_payload(uint8_t* payload, uint16_t payload_len);
bool app_nv_store_write_payload(const uint8_t* payload, uint16_t payload_len);
bool app_nv_store_is_payload_equal(const uint8_t* payload, uint16_t payload_len);

#endif
