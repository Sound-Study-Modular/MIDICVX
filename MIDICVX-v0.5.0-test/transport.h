#ifndef TRANSPORT_H_
#define TRANSPORT_H_

#include <stdint.h>

void Transport_Init(void);
void Transport_Process(void);
uint32_t Transport_GetTickCount(void);
uint8_t Transport_ConsumeTick(void);

#endif /* TRANSPORT_H_ */
