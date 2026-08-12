/*
 * keyboard.c
 *
 * Created: 1/17/2017 10:35:53 AM
 *  Author: Janis Zaharans
 */ 

#include "main.h"



void Keyboard_Reset(uint8_t *keyboard)
{
	uint8_t i;
	
	for(i=0; i<16; i++)
		keyboard[i] = 0;	
}

void Keyboard_Press(uint8_t *keyboard, uint8_t key)
{
	uint8_t byte_idx = key>>3;
	uint8_t bit_idx = key & 0x7;
	
	keyboard[byte_idx] |= (1<<bit_idx);
}


void	Keyboard_Release(uint8_t *keyboard, uint8_t key)
{
	uint8_t byte_idx = key>>3;
	uint8_t bit_idx = key & 0x7;
	
	keyboard[byte_idx] &= ~(1<<bit_idx);
}


int8_t Keyboard_GetHighest(uint8_t *keyboard)
{
	int8_t byte_idx;
	int8_t bit_idx;
	uint8_t bit_mask;
	uint8_t	found;
	
	// Find highest byte
	found = 0;
	for(byte_idx = 15; byte_idx >= 0; byte_idx--) {
		if(keyboard[byte_idx] != 0) {
			found = 1;
			break;
		}
	}
	
	if(!found)
		return -1;
		
	
	// Find highest bit
	found = 0;
	bit_mask = 0x80;
	for(bit_idx = 7; bit_idx >= 0; bit_idx--) {
		if((keyboard[byte_idx] & bit_mask) != 0) {
			found = 1;
			break;
		}
			
		bit_mask >>= 1;
	}
	
	if(!found)
		return -1;
	else
		return ((byte_idx<<3) | bit_idx);
}


int8_t	Keyboard_GetLowest(uint8_t *keyboard)
{
	uint8_t byte_idx;
	uint8_t bit_idx;
	uint8_t bit_mask;
	uint8_t found;
	
	// Find lowest byte
	found = 0;
	for(byte_idx = 0; byte_idx < 16; byte_idx++) {
		if(keyboard[byte_idx] != 0) {
			found = 1;
			break;
		}
	}
	
	if(!found)
		return -1;
	
	// Find lowest bit
	found = 0;
	bit_mask = 0x01;
	for(bit_idx = 0; bit_idx < 8; bit_idx++) {
		if((keyboard[byte_idx] & bit_mask) != 0) {
			found = 1;
			break;
		}
		
		bit_mask <<= 1;
	}
	
	if(!found)
		return -1;
	else
		return ((byte_idx<<3) | bit_idx);
}