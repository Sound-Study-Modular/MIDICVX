/*
 * button.h
 *
 * Created: 1/17/2017 6:58:44 PM
 *  Author: Janis Zaharans
 */ 


#ifndef BUTTON_H_
#define BUTTON_H_


typedef struct
{
	volatile uint8_t*	port;
	uint8_t				pin;

	uint16_t	state_press;
	uint16_t	state_release;

	uint8_t		press;
	uint8_t		release;
	uint32_t	hold_time;
} button_t;


void Button_Process(button_t *btn);


#endif /* BUTTON_H_ */