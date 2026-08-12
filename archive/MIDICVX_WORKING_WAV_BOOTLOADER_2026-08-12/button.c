/*
 * button.c
 *
 * Created: 1/17/2017 6:58:10 PM
 *  Author: Janis Zaharans
 */ 

#include "main.h"

void Button_Process(button_t *btn)
{
	btn->state_press = (btn->state_press<<1) | (((*btn->port) & btn->pin) != 0) | 0xE000;
	if(btn->state_press == 0xF000) btn->press = 1;

	btn->state_release = (btn->state_release<<1) | (((*btn->port) & btn->pin) == 0) | 0xE000;
	if(btn->state_release == 0xF000) btn->release = 1;
}