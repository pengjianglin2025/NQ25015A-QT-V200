#ifndef _IAP_H_
#define _IAP_H_

#include "main.h"

/* 断电参数结构采用 1 字节对齐，避免结构变化导致存储布局错位。 */
#pragma pack(1)

#define IAP_WRITE_OK   0x55  //数据已写进flash区域标志
#define IAP_DATA_MAX   70

#define IAP_START_ADDR 0x01003000   //应用参数安全存储区地址

extern uint32_t Iap_Read_Temp[10];

typedef struct {
    uint8_t startTimeHour;        //工作开始时间小时
    uint8_t startTimeMinutes;     //工作开始时间分钟
    uint8_t stopTimeHour;         //工作结束时间小时
    uint8_t stopTimeMinutes;      //工作结束时间分钟
    uint16To2_t workTime;         //工作时间
    uint16To2_t pauseTime;        //暂停时间
    uint8_t workGear;             //工作浓度
    uint8_t workWeek;             //工作日 第0位代表星期一....依次第6位代表星期天
    uint8_t eventEN;              //事件使能
}EventIap_t;

typedef union
{
    uint8_t Buffer[IAP_DATA_MAX];
    struct
    {
        uint8_t writeOk;
        EventIap_t EventData[5];
        uint8_t FanEN;
        uint8_t workState;
        uint8_t keyLockState;
        uint8_t lightEn;
        uint8_t totalVolume;
        uint8_t curretVolume;
        uint8_t consumeSpeed;
        uint8_t rollingCode1;
        uint8_t rollingCode2;
    };
}Iap_t;

#pragma pack()

/* IAP 布局大小校验：变化后需同步调整 IAP_DATA_MAX。 */
typedef char iap_layout_size_check[(sizeof(Iap_t) == IAP_DATA_MAX) ? 1 : -1];

void Iap_Read(void);
void Iap_Write(void);
void Iap_Data_Rest(void);
void Iap_Data_Comparison(void);

#endif
