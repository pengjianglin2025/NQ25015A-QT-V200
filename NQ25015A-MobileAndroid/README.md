# NQ25015A Mobile Android

Android app skeleton generated from the firmware protocol in `Code/projects/user/protocol.c`
and the BLE RDTSS 16-bit profile in `Code/projects/inc/app_profile/app_rdtss_16bit.h`.

## What is included

- Native Android Studio project using Kotlin + Jetpack Compose
- BLE transport wired to the firmware's real UUIDs
- Protocol frame encoder/decoder for:
  - `0x06` DP write
  - `0x07` DP report
  - `0x08` status request
  - `0x1C` time sync
- First-page controls for:
  - Aroma power
  - Concentration
  - Fan
  - Light
  - Motor
  - Key lock
  - Simple/professional mode
  - Custom timing
- Live hex log window for debugging

## BLE profile used by this app

From the firmware:

- Service UUID: `0xFFF0`
- Notify characteristic: `0xFFF1`
- Write characteristic: `0xFFF2`
- Default BLE name in sample readme: `NS_RDTSS`

The app uses the standard 16-bit Bluetooth base UUID:

- `0000FFF0-0000-1000-8000-00805F9B34FB`
- `0000FFF1-0000-1000-8000-00805F9B34FB`
- `0000FFF2-0000-1000-8000-00805F9B34FB`

## Notes

- This repository does not currently include a Gradle wrapper jar.
- Open the folder in Android Studio and let it sync/download the Android toolchain.
- BLE runtime permissions are handled in code for Android 12+.

