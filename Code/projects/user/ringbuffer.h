#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include "protocol.h"
#include "main.h"

#define BUFF_SIZE_MAX (1024)
#define GET_MIN(x,y)  ((x) < (y) ? (x) : (y))

typedef struct
{
    uint32_t putCount;
    uint32_t getCount;
    uint32_t frameCount;
    uint32_t overflowCount;
    uint32_t badHeaderCount;
    uint32_t badVersionCount;
    uint32_t badLengthCount;
    uint32_t resetCount;
} RbStats_t;

typedef struct
{
    volatile uint16_t bufferSize;
    volatile uint16_t head;
    volatile uint16_t tail;
    uint8_t buffer[BUFF_SIZE_MAX];
    RbStats_t stats;
} RB_t;
extern RB_t rb;

void Ringbuffer_Init(void);
uint16_t ringbuffer_data_len(RB_t* rb);
uint16_t ringbuffer_putstr(RB_t* rb, const uint8_t* data1, uint16_t data_length);
int ringbuffer_getstr(RB_t* rb, uint8_t* data1, uint16_t data_length);
uint8_t ring_buffer_read(void);

#endif