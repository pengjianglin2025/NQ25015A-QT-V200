#include "iap.h"
#include "app_user_config.h"
#include "app_nv_store.h"

Iap_t iapRead, IapWrite;
uint8_t IapWriteEn = 0;

#define FLASH_TEST_ADDRESS       ((uint32_t)IAP_START_ADDR)
#define BUFFER_SIZE              IAP_DATA_MAX


/* 将当前运行参数整理成一份完整镜像，便于统一存盘。 */
static void iap_build_runtime_image(Iap_t* image)
{
    uint8_t index;

    memset(image->Buffer, 0, sizeof(image->Buffer));
    image->writeOk = IAP_WRITE_OK;

    for(index = 0; index < 5; index++)
    {
        image->EventData[index].startTimeHour = runEvent[index].startTimeHour;
        image->EventData[index].startTimeMinutes = runEvent[index].startTimeMinutes;
        image->EventData[index].stopTimeHour = runEvent[index].stopTimeHour;
        image->EventData[index].stopTimeMinutes = runEvent[index].stopTimeMinutes;
        image->EventData[index].workTime.WORD = runEvent[index].workTime;
        image->EventData[index].pauseTime.WORD = runEvent[index].pauseTime;
        image->EventData[index].workGear = runEvent[index].workPer;
        image->EventData[index].workWeek = runEvent[index].weekEn.BYTE;
        image->EventData[index].eventEN = runEvent[index].en;
    }

    image->FanEN = fan.en;
    image->workState = aroma.en;
    image->keyLockState = key.lockStatus;
    image->rollingCode1 = RollingCode1;
    image->rollingCode2 = RollingCode2;
    image->oilCurretVolume.WORD = oil.curretVolume;
    image->oilConsumeSpeed.WORD = oil.actualConsumeSpeed;
}

/* 将 Flash 中保存的参数恢复到运行变量。 */
static void iap_restore_runtime_image(const Iap_t* image)
{
    uint8_t index;

    for(index = 0; index < 5; index++)
    {
        runEvent[index].startTimeHour = image->EventData[index].startTimeHour;
        runEvent[index].startTimeMinutes = image->EventData[index].startTimeMinutes;
        runEvent[index].stopTimeHour = image->EventData[index].stopTimeHour;
        runEvent[index].stopTimeMinutes = image->EventData[index].stopTimeMinutes;
        runEvent[index].workTime = image->EventData[index].workTime.WORD;
        runEvent[index].pauseTime = image->EventData[index].pauseTime.WORD;
        runEvent[index].workPer = image->EventData[index].workGear;
        runEvent[index].weekEn.BYTE = image->EventData[index].workWeek;
        runEvent[index].en = image->EventData[index].eventEN;
    }

    fan.en = image->FanEN;
    aroma.en = image->workState;
    key.lockStatus = image->keyLockState;
    RollingCode1 = image->rollingCode1;
    RollingCode2 = image->rollingCode2;

    if((image->oilCurretVolume.WORD == 0U) && (image->oilConsumeSpeed.WORD == 0U))
    {
        oil.curretVolume = oil.totalVolume;
        oil.actualConsumeSpeed = oil.defaultConsumeSpeed;
    }
    else
    {
        oil.curretVolume = image->oilCurretVolume.WORD;
        oil.actualConsumeSpeed = image->oilConsumeSpeed.WORD;
    }

    if(oil.curretVolume > oil.totalVolume)
    {
        oil.curretVolume = oil.totalVolume;
    }
    if(oil.actualConsumeSpeed == 0U)
    {
        oil.actualConsumeSpeed = oil.defaultConsumeSpeed;
    }
}

/* 启动时优先读取新参数区；若新参数区为空，则兼容旧地址并自动迁移。 */
static uint8_t iap_storage_load(uint8_t* buffer, uint16_t size)
{
#if (APP_NV_STORE_ENABLE)
    return app_nv_store_load_legacy_compatible(buffer, size) ? 1U : 0U;
#else
    memset(buffer, 0, size);
    Qflash_Read(FLASH_TEST_ADDRESS, buffer, size);
    return (buffer[0] == IAP_WRITE_OK) ? 1U : 0U;
#endif
}

/* 读取当前正在使用的参数区，用于比较是否需要更新。 */
static uint8_t iap_storage_read_current(uint8_t* buffer, uint16_t size)
{
#if (APP_NV_STORE_ENABLE)
    return app_nv_store_read_payload(buffer, size) ? 1U : 0U;
#else
    memset(buffer, 0, size);
    Qflash_Read(FLASH_TEST_ADDRESS, buffer, size);
    return (buffer[0] == IAP_WRITE_OK) ? 1U : 0U;
#endif
}

/* 统一写入入口：默认写新参数区，关闭宏时回退旧实现。 */
static uint8_t iap_storage_write(const uint8_t* buffer, uint16_t size)
{
#if (APP_NV_STORE_ENABLE)
    return app_nv_store_write_payload(buffer, size) ? 1U : 0U;
#else
    uint8_t retry;
    for(retry = 0; retry < 3; retry++)
    {
        Qflash_Erase_Sector(FLASH_TEST_ADDRESS);
        Qflash_Write(FLASH_TEST_ADDRESS, (uint8_t*)buffer, size);
        memset(iapRead.Buffer, 0, sizeof(iapRead.Buffer));
        Qflash_Read(FLASH_TEST_ADDRESS, iapRead.Buffer, size);
        if(memcmp(iapRead.Buffer, buffer, size) == 0)
        {
            return 1U;
        }
    }
    return 0U;
#endif
}

void Iap_Read(void)
{
    memset(iapRead.Buffer, 0, sizeof(iapRead.Buffer));

    if(iap_storage_load(iapRead.Buffer, BUFFER_SIZE) && (iapRead.writeOk == IAP_WRITE_OK))
    {
        iap_restore_runtime_image(&iapRead);
    }
    else
    {
        event_date_init();
        oil_reset();
        fan.en = ON;
        aroma.en = ON;
        key.lockStatus = UNLOCK;
        Iap_Write();
    }
}

void Iap_Write(void)
{
    uint8_t storageValid;

    iap_build_runtime_image(&IapWrite);
    storageValid = iap_storage_write(IapWrite.Buffer, BUFFER_SIZE);
    if(storageValid)
    {
        (void)iap_storage_read_current(iapRead.Buffer, BUFFER_SIZE);
    }
    else
    {
        memset(iapRead.Buffer, 0, sizeof(iapRead.Buffer));
    }
    IapWriteEn = 0;
}
/**************************************************************************
** 功能    @brief : 比较运行的参数和flash里的值，如果不一样就更新flash的值
** 输入    @param : 
** 输出    @retval: 
***************************************************************************/
void Iap_Data_Comparison(void)
{
    uint8_t index;
    uint8_t eventChanged;
    uint8_t diffIndex;
    uint8_t storageValid;

    if(globalWorkState == FULL_WORKING)
    {
        memset(iapRead.Buffer, 0, sizeof(iapRead.Buffer));
        storageValid = iap_storage_read_current(iapRead.Buffer, BUFFER_SIZE);

        iap_build_runtime_image(&IapWrite);
        if(!storageValid)
        {
            IapWriteEn = 1;
        }

        eventChanged = 0;
        diffIndex = 0;
        /* DP18 只对应 5 组事件表，不能把油量等运行值变化也当成事件表变化去上报。 */
        for(index = 1; storageValid && (index < (1 + sizeof(iapRead.EventData))); index++)
        {
            if(iapRead.Buffer[index] != IapWrite.Buffer[index])
            {
                eventChanged = 1;
                diffIndex = index;
                break;
            }
        }
        if(eventChanged)
        {
            IapWriteEn = 1;
            if(!upData.DPID018Back) upData.DPID018Back = 1;
            aroma.startTime = 0;
        }
        if(iapRead.FanEN != IapWrite.FanEN) {IapWriteEn = 1; if(!upData.DPID004Back) upData.DPID004Back = 1;}
        if(iapRead.workState != IapWrite.workState) {IapWriteEn = 1; if(!upData.DPID001Back) upData.DPID001Back = 1;}
        if(iapRead.keyLockState != IapWrite.keyLockState) {IapWriteEn = 1; if(!upData.DPID005Back) upData.DPID005Back = 1;}
        if(iapRead.oilCurretVolume.WORD != IapWrite.oilCurretVolume.WORD) {IapWriteEn = 1; if(!upData.DPID020Back) upData.DPID020Back = 1;}
        if(iapRead.oilConsumeSpeed.WORD != IapWrite.oilConsumeSpeed.WORD) {IapWriteEn = 1; if(!upData.DPID020Back) upData.DPID020Back = 1;}

        if(IapWriteEn)
        {
            IapWriteEn = 0;
            (void)iap_storage_write(IapWrite.Buffer, BUFFER_SIZE);
        }
    }
    else
    {
        IapWriteEn = 0;
    }
}
void Iap_Data_Rest(void)
{
}


