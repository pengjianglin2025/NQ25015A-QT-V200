
#include "ringbuffer.h"

RB_t rb;

void Ringbuffer_Init(void)
{
	memset(rb.buffer,0,BUFF_SIZE_MAX);
	rb.bufferSize = BUFF_SIZE_MAX - 1;   //“-1”是保证数组有一定的余量
	rb.head = 0;
	rb.tail = 0;
}

/**
 * @description:  使用的rb大小的值
 * @param 			rb  ringbuffer
 * @return 执行结果码
 */
uint16_t ringbuffer_data_len(RB_t* rb)
{
//    return ((rb->buffer_size - rb->head + rb->tail) % rb->buffer_size);
	if (rb->head == rb->tail) return 0;
  if (rb->head < rb->tail) return rb->tail - rb->head;
  return rb->bufferSize - (rb->head - rb->tail);

}
/**
 * @description: 发送数据到rb中
 * @param 			rb  			ringbuffer
 * 					data			待发送数据buffer
 * 					data_length		发送的数据长度
 * @return 执行结果码
 */
uint16_t ringbuffer_putstr(RB_t* rb, const uint8_t* data1, uint16_t data_length )
{
    uint16_t space_len = rb->bufferSize - 1 - rb->tail;

    uint16_t put_data_len = GET_MIN(data_length, (space_len + rb->head) % rb->bufferSize);

    space_len++;

    memcpy(&rb->buffer[rb->tail], data1, GET_MIN(put_data_len, space_len));

    if ( space_len < put_data_len )
    {
        memcpy( &rb->buffer[ 0 ], data1 + space_len, put_data_len - space_len );
    }

    rb->tail = (rb->tail + put_data_len) % rb->bufferSize;

	
    return put_data_len;
}

/**
 * @description: 从rb中读取数据
 * @param 			rb  			ringbuffer
 * 					data			待读取buffer
 * 					data_length		读取的数据长度
 * @return 执行结果码
 */
int ringbuffer_getstr(RB_t* rb, uint8_t* data1, uint16_t data_length)
{
    uint16_t i, used_space, max_read_len, head;

    head = rb->head;

    used_space = ringbuffer_data_len(rb);

    max_read_len = GET_MIN(data_length, used_space);

    if ( max_read_len != 0)
    {
        for ( i = 0; i != max_read_len; i++, ( head = ( head + 1 ) % rb->bufferSize ) )
        {
            data1[ i ] = rb->buffer[ head ];
        }
		rb->head = (rb->head + max_read_len) % rb->bufferSize;
    }
    return 0;
}


void ring_buffer_read(void)
{
	uint16_t usedSpace, readLen, headTemp;
	uint16To2_t lengthTemp;
	
	if(net.HaveNewRxData == 0)// && (ReceiveIdleCount > INTERVAL_TIME))
	{
		headTemp = rb.head;
		usedSpace = ringbuffer_data_len(&rb);
		
		if(usedSpace >= 6)  //协议的最低字节数要求大于6
		{
			lengthTemp.BYTE1 = rb.buffer[headTemp+4];
			lengthTemp.BYTE0 = rb.buffer[headTemp+5];
			if((rb.buffer[headTemp] == HEAD1) && (rb.buffer[headTemp+1] == HEAD2) && \
				(usedSpace >= (lengthTemp.WORD + 7)))
			{
				readLen = (lengthTemp.WORD + 7);
				
				//memcpy(Rx.Buffer, &rb.buffer[headTemp], readLen);
				ringbuffer_getstr(&rb, Rx.Buffer, readLen);
				net.HaveNewRxData = 1;
				
//				rb.head = (rb.head + readLen) % rb.bufferSize;
				
				memset(&rb.buffer[headTemp],0,readLen);
			}
			else
			{
				if(++rb.head >= rb.bufferSize) rb.head = 0;
			}
		}
	}
}

