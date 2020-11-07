#include "stm32f4xx_hal_conf.h"
#include "led.h"

#define	LED_GPIO_PORT           GPIOD
#define	LED_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()

static const uint16_t ledPins[] =
{
    GPIO_PIN_12,
    GPIO_PIN_13,
    GPIO_PIN_14,
    GPIO_PIN_15,
};

void ledOn(const LED_ID led)
{
    LED_GPIO_PORT->BSRR = ledPins[led];
}

void ledOff(const LED_ID led)
{
    LED_GPIO_PORT->BSRR = (uint32_t)ledPins[led] << 16;
}

void ledsOn(void)
{
    LED_GPIO_PORT->BSRR = ledPins[0]|ledPins[1]|ledPins[2]|ledPins[3];
}

void ledsOff(void)
{
    LED_GPIO_PORT->BSRR = (uint32_t)(ledPins[0]|ledPins[1]|ledPins[2]|ledPins[3]) << 16;
}

void ledInit(void)
{
    LED_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef init =
    {
        .Pin = ledPins[0]|ledPins[1]|ledPins[2]|ledPins[3],
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_LOW,
    };
    HAL_GPIO_Init(LED_GPIO_PORT, &init);
}
