/*
 * ring_buffer.c
 *
 * Created: 1/16/2017 4:24:35 PM
 *  Author: Janis Zaharans
 */ 

#include "main.h"

void	RingBuffer_Init(ring_buffer_t *ring_buf)
{
	ring_buf->w_head = ring_buf->buf;
	ring_buf->r_head = ring_buf->buf;
}

void	RingBuffer_Write(ring_buffer_t *ring_buf, uint8_t data)
{
	*(ring_buf->w_head++) = data;
	if(ring_buf->w_head >= (ring_buf->buf + RING_BUFFER_SIZE)) {
		ring_buf->w_head = ring_buf->buf;
	}
}

uint8_t	RingBuffer_Read(ring_buffer_t *ring_buf)
{
	uint8_t data = *(ring_buf->r_head++);
	if(ring_buf->r_head >= (ring_buf->buf + RING_BUFFER_SIZE)) {
		ring_buf->r_head = ring_buf->buf;
	}
	return data;
}

uint8_t	RingBuffer_GetLength(ring_buffer_t *ring_buf)
{
	if(ring_buf->r_head == ring_buf->w_head)
		return 0;
	else if (ring_buf->w_head > ring_buf->r_head)
		return (ring_buf->w_head - ring_buf->r_head);
	else
		return (ring_buf->w_head - ring_buf->r_head + RING_BUFFER_SIZE);
}

uint8_t RingBuffer_IsEmpty(ring_buffer_t *ring_buf)
{
	return (ring_buf->w_head == ring_buf->r_head);
}


void	RingBuffer_Clear(ring_buffer_t *ring_buf) 
{
	ring_buf->w_head = ring_buf->buf;
	ring_buf->r_head = ring_buf->buf;
}