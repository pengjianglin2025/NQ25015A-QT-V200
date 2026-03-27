# Protocol Reference

This file summarizes the firmware protocol used by the generated Android app.

## Transport

- BLE service UUID: `0xFFF0`
- Notify characteristic UUID: `0xFFF1`
- Write characteristic UUID: `0xFFF2`

## Frame

```text
55 AA VER CMD LEN_H LEN_L PAYLOAD... CHECKSUM
```

- Checksum: sum of bytes from `HEAD1` to end of payload, then `% 256`

## Command words

- `0x00`: heartbeat / dida
- `0x01`: networking mode string
- `0x03`: link status
- `0x04`: module reset
- `0x06`: write DP
- `0x07`: report DP
- `0x08`: request all DP states
- `0x1C`: time sync / time request

## Known DPs from firmware

- `1`: aroma enable
- `3`: concentration
- `4`: fan enable
- `5`: key lock
- `9`: light enable
- `15`: motor enable
- `18`: schedule table
- `20`: oil info
- `22`: feature bitmap / capabilities
- `23`: min/max parameter limits
- `24`: current runtime status
- `25`: parameter mode
- `26`: concentration count
- `27`: custom timing

