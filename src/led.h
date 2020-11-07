#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
	LED_GREEN,
	LED_ORANGE,
	LED_RED,
	LED_BLUE,
} LED_ID;

void ledInit(void);
void ledOn(LED_ID led);
void ledOff(LED_ID led);
void ledsOn(void);
void ledsOff(void);

#ifdef __cplusplus
}
#endif
