#include "ringbuffer.h"

RB_t rb;

static void ringbuffer_reset(RB_t* ring)
{
    memset(ring->buffer, 0, sizeof(ring->buffer));
    ring->head = 0;
    ring->tail = 0;
    ring->stats.resetCount++;
}

static uint16_t ringbuffer_free_len(RB_t* ring)
{
    return (uint16_t)(ring->bufferSize - 1U - ringbuffer_data_len(ring));
}

static uint8_t ringbuffer_peek_byte(RB_t* ring, uint16_t offset)
{
    return ring->buffer[(ring->head + offset) % ring->bufferSize];
}

void Ringbuffer_Init(void)
{
    memset(&rb, 0, sizeof(rb));
    rb.bufferSize = BUFF_SIZE_MAX;
    ringbuffer_reset(&rb);
    rb.stats.resetCount = 0;
}

uint16_t ringbuffer_data_len(RB_t* ring)
{
    if(ring->tail >= ring->head)
    {
        return (uint16_t)(ring->tail - ring->head);
    }

    return (uint16_t)(ring->bufferSize - ring->head + ring->tail);
}

uint16_t ringbuffer_putstr(RB_t* ring, const uint8_t* data1, uint16_t data_length)
{
    uint16_t put_data_len;
    uint16_t first_copy_len;
    uint16_t remain_len;

    /* 收到新字节流后立刻清接收空闲计时，避免被上层误判为超时。 */
    ReceiveIdleCount = 0;

    put_data_len = GET_MIN(data_length, ringbuffer_free_len(ring));
    if(put_data_len == 0U)
    {
        ring->stats.overflowCount++;
        return 0U;
    }
    if(put_data_len < data_length)
    {
        ring->stats.overflowCount++;
    }

    first_copy_len = GET_MIN(put_data_len, (uint16_t)(ring->bufferSize - ring->tail));
    memcpy(&ring->buffer[ring->tail], data1, first_copy_len);

    remain_len = (uint16_t)(put_data_len - first_copy_len);
    if(remain_len > 0U)
    {
        memcpy(&ring->buffer[0], data1 + first_copy_len, remain_len);
    }

    ring->tail = (uint16_t)((ring->tail + put_data_len) % ring->bufferSize);
    ring->stats.putCount += put_data_len;
    return put_data_len;
}

int ringbuffer_getstr(RB_t* ring, uint8_t* data1, uint16_t data_length)
{
    uint16_t max_read_len;
    uint16_t first_copy_len;
    uint16_t remain_len;

    max_read_len = GET_MIN(data_length, ringbuffer_data_len(ring));
    if(max_read_len == 0U)
    {
        return 0;
    }

    first_copy_len = GET_MIN(max_read_len, (uint16_t)(ring->bufferSize - ring->head));
    memcpy(data1, &ring->buffer[ring->head], first_copy_len);

    remain_len = (uint16_t)(max_read_len - first_copy_len);
    if(remain_len > 0U)
    {
        memcpy(data1 + first_copy_len, &ring->buffer[0], remain_len);
    }

    ring->head = (uint16_t)((ring->head + max_read_len) % ring->bufferSize);
    ring->stats.getCount += max_read_len;
    return max_read_len;
}

uint8_t ring_buffer_read(void)
{
    uint16_t usedSpace;
    uint16_t readLen;
    uint16To2_t lengthTemp;

    if(net.HaveNewRxData)
    {
        return 0U;
    }

    while((usedSpace = ringbuffer_data_len(&rb)) >= 7U)
    {
        if((ringbuffer_peek_byte(&rb, 0U) != HEAD1) || (ringbuffer_peek_byte(&rb, 1U) != HEAD2))
        {
            rb.head = (uint16_t)((rb.head + 1U) % rb.bufferSize);
            rb.stats.badHeaderCount++;
            continue;
        }

        if(ringbuffer_peek_byte(&rb, 2U) != RECEIVE_VERSION)
        {
            rb.head = (uint16_t)((rb.head + 1U) % rb.bufferSize);
            rb.stats.badVersionCount++;
            continue;
        }

        lengthTemp.BYTE1 = ringbuffer_peek_byte(&rb, 4U);
        lengthTemp.BYTE0 = ringbuffer_peek_byte(&rb, 5U);
        if(lengthTemp.WORD > (PROTOCOL_DATA_MAX - 7U))
        {
            rb.head = (uint16_t)((rb.head + 1U) % rb.bufferSize);
            rb.stats.badLengthCount++;
            continue;
        }

        readLen = (uint16_t)(lengthTemp.WORD + 7U);
        if(usedSpace < readLen)
        {
            return 0U;
        }

        /* 这里只负责提取完整帧，校验和业务处理仍然留在 protocol.c。 */
        memset(protocol_rx_buffer_get(), 0, protocol_rx_buffer_size_get());
        if(ringbuffer_getstr(&rb, protocol_rx_buffer_get(), readLen) != readLen)
        {
            ringbuffer_reset(&rb);
            return 0U;
        }

        net.HaveNewRxData = 1;
        RxCnt = readLen;
        rb.stats.frameCount++;
        return 1U;
    }

    return 0U;
}