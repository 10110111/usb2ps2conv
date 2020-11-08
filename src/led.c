#include "stm32f4xx_hal_conf.h"
#include "led.h"

#ifdef ENABLE_LED
# define	LED_GPIO_PORT           GPIOD
# define	LED_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOD_CLK_ENABLE()

static const uint16_t ledPins[] =
{
    GPIO_PIN_12,
    GPIO_PIN_13,
    GPIO_PIN_14,
    GPIO_PIN_15,
};
#endif

void ledOn(const LED_ID led)
{
#ifdef ENABLE_LED
    LED_GPIO_PORT->BSRR = ledPins[led];
#endif
}

void ledOff(const LED_ID led)
{
#ifdef ENABLE_LED
    LED_GPIO_PORT->BSRR = (uint32_t)ledPins[led] << 16;
#endif
}

void ledsOn(void)
{
#ifdef ENABLE_LED
    LED_GPIO_PORT->BSRR = ledPins[0]|ledPins[1]|ledPins[2]|ledPins[3];
#endif
}

void ledsOff(void)
{
#ifdef ENABLE_LED
    LED_GPIO_PORT->BSRR = (uint32_t)(ledPins[0]|ledPins[1]|ledPins[2]|ledPins[3]) << 16;
#endif
}

void ledInit(void)
{
#ifdef ENABLE_LED
    LED_GPIO_CLK_ENABLE();

    GPIO_InitTypeDef init =
    {
        .Pin = ledPins[0]|ledPins[1]|ledPins[2]|ledPins[3],
        .Mode = GPIO_MODE_OUTPUT_PP,
        .Pull = GPIO_NOPULL,
        .Speed = GPIO_SPEED_LOW,
    };
    HAL_GPIO_Init(LED_GPIO_PORT, &init);
#endif
}
