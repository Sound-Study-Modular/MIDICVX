#ifndef TRANSPORT_H_
#define TRANSPORT_H_

#include <stdint.h>

/*
 * External transport input on PC1 / PCINT9.
 * The PCB net and front-panel jack were originally CLOCK OUT.
 */
void Transport_Init(void);
void Transport_Process(void);
uint32_t Transport_GetTickCount(void);
uint8_t Transport_ConsumeTick(void);

#endif /* TRANSPORT_H_ */
