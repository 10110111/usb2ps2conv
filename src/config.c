#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "syscalls.h"
#include "config.h"
#include "led.h"

void SystemClock_Config(void)
{
    /* Enable Power Control clock */
    __PWR_CLK_ENABLE();

    /* The voltage scaling allows optimizing the power consumption when the device is
       clocked below the maximum system frequency, to update the voltage scaling value
       regarding system frequency refer to product datasheet.  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

    /* Enable HSE Oscillator and activate PLL with HSI as source */
    RCC_OscInitTypeDef RCC_OscInitStruct =
    {
        .OscillatorType = RCC_OSCILLATORTYPE_HSI,
        .HSIState = RCC_HSI_ON,
        .HSICalibrationValue = 16,
        .PLL.PLLState = RCC_PLL_ON,
        .PLL.PLLSource = RCC_PLLSOURCE_HSI,
        .PLL.PLLM = 16,
        .PLL.PLLN = 336,
        .PLL.PLLP = RCC_PLLP_DIV4,
        .PLL.PLLQ = 7,
    };
    if(HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
        abort();

    /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
    RCC_ClkInitTypeDef RCC_ClkInitStruct =
    {
        .ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2),
        .SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK,
        .AHBCLKDivider = RCC_SYSCLK_DIV1,
        .APB1CLKDivider = RCC_HCLK_DIV2,
        .APB2CLKDivider = RCC_HCLK_DIV1,
    };
    if(HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
        abort();
}
