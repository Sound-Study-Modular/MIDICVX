/*
 * ring_buffer.h
 *
 * Created: 1/16/2017 4:24:51 PM
 *  Author: Janis Zaharans
 */ 


#ifndef RING_BUFFER_H_
#define RING_BUFFER_H_


#define RING_BUFFER_SIZE	128

typedef struct  
{
	uint8_t* r_head;
	uint8_t* w_head;
	uint8_t	buf[RING_BUFFER_SIZE];
} ring_buffer_t;


void	RingBuffer_Init(ring_buffer_t *ring_buf);
void	RingBuffer_Write(ring_buffer_t *ring_buf, uint8_t data);
uint8_t	RingBuffer_Read(ring_buffer_t *ring_buf);
uint8_t	RingBuffer_GetLength(ring_buffer_t *ring_buf);
uint8_t RingBuffer_IsEmpty(ring_buffer_t *ring_buf);
void	RingBuffer_Clear(ring_buffer_t *ring_buf);


#endif /* RING_BUFFER_H_ */