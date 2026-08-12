/*
 * blinker.c
 *
 * Created: 2/17/2017 10:31:01 AM
 *  Author: Janis Zaharans
 */ 
#include "main.h"

void Blinker_Process(uint32_t* pattern, uint8_t length)
{
	static uint32_t time;
	static uint8_t step;
	
	uint32_t now = g_time;
	
	if((time - now) > pattern[step]) {
		
		if(step & 0x01) {
			MIDI_LED_HIGH();
		} else {
			MIDI_LED_LOW();
		}
		
		step++;
		if(step == length)
			step = 0;
		
		time = now + pattern[step];
	}
}