#ifndef __RINGBUFFER_H__
#define __RINGBUFFER_H__

#include "protocol.h"
#include "main.h"


#define BUFF_SIZE_MAX 		(1024)
#define GET_MIN(x,y)  ((x) < (y) ? (x) : (y))

//struct ringbuffer
//{
//  uint8_t*  buffer;
//  uint16_t  buffer_size;
//  volatile uint16_t  head; 
//  volatile uint16_t  tail; 
//};

typedef struct 
{
	volatile uint16_t bufferSize;
  volatile uint16_t  head; 
  volatile uint16_t  tail; 
  uint8_t  buffer[BUFF_SIZE_MAX];
}RB_t;
extern RB_t rb;


void Ringbuffer_Init(void);
uint16_t ringbuffer_putstr(RB_t* rb, const uint8_t* data1, uint16_t data_length );
void ring_buffer_read(void);

#endif
