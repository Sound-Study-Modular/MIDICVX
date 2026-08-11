/*
 * keyboard.h
 *
 * Created: 1/17/2017 10:37:01 AM
 *  Author: Janis Zaharans
 */ 


#ifndef KEYBOARD_H_
#define KEYBOARD_H_


void	Keyboard_Reset(uint8_t *keyboard);
void	Keyboard_Press(uint8_t *keyboard, uint8_t key);
void	Keyboard_Release(uint8_t *keyboard, uint8_t key);
int8_t	Keyboard_GetHighest(uint8_t *keyboard);
int8_t	Keyboard_GetLowest(uint8_t *keyboard);



#endif /* KEYBOARD_H_ */