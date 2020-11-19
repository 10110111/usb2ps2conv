#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

void PS2_Init(void);
void PS2_Process(void);
void passByteToPS2(uint8_t data);
extern volatile uint32_t autorepeatTickCounter;
extern uint32_t autorepeatPeriodInTicks;
extern uint32_t autorepeatDelayInTicks;

#ifdef __cplusplus
}
#endif
