#include "stm32f4xx.h"
#include "stm32f4xx_hal.h"
#include "syscalls.h"
#include "util.h"
#include "dbg-out.h"

#ifdef ENABLE_DEBUG_OUTPUT
# define DBG_USART_NUM 6
# define DBG_USART_TX_GPIO_LETTER C
# define DBG_USART_TX_PIN_NUM 6
# define DBG_USART_RX_GPIO_LETTER C
# define DBG_USART_RX_PIN_NUM 7

# define USART_DBG                       CONCAT(USART,DBG_USART_NUM,)
# define USART_DBG_CLK_ENABLE()          CONCAT(__HAL_RCC_USART,DBG_USART_NUM,_CLK_ENABLE())
# define USART_DBG_RX_GPIO_CLK_ENABLE()  CONCAT(__HAL_RCC_GPIO,DBG_USART_RX_GPIO_LETTER,_CLK_ENABLE())
# define USART_DBG_TX_GPIO_CLK_ENABLE()  CONCAT(__HAL_RCC_GPIO,DBG_USART_TX_GPIO_LETTER,_CLK_ENABLE())
# define USART_DBG_FORCE_RESET()         CONCAT(__HAL_RCC_USART,DBG_USART_NUM,_FORCE_RESET())
# define USART_DBG_RELEASE_RESET()       CONCAT(__HAL_RCC_USART,DBG_USART_NUM,_RELEASE_RESET())
# define USART_DBG_TX_PIN                CONCAT(GPIO_PIN_,DBG_USART_TX_PIN_NUM,)
# define USART_DBG_TX_GPIO_PORT          CONCAT(GPIO,DBG_USART_TX_GPIO_LETTER,)
# define USART_DBG_TX_AF                 CONCAT(GPIO_AF8_USART,DBG_USART_NUM,)
# define USART_DBG_RX_PIN                CONCAT(GPIO_PIN_,DBG_USART_RX_PIN_NUM,)
# define USART_DBG_RX_GPIO_PORT          CONCAT(GPIO,DBG_USART_RX_GPIO_LETTER,)
# define USART_DBG_RX_AF                 CONCAT(GPIO_AF8_USART,DBG_USART_NUM,)

UART_HandleTypeDef uartDbgHandle;

static void rccConfig(void)
{
    USART_DBG_RX_GPIO_CLK_ENABLE();
    USART_DBG_TX_GPIO_CLK_ENABLE();
    USART_DBG_CLK_ENABLE();
}

static void gpioConfig(void)
{
    GPIO_InitTypeDef init;

    init.Pin = USART_DBG_TX_PIN;
    init.Mode = GPIO_MODE_AF_PP;
    init.Pull = GPIO_NOPULL;
    init.Speed = GPIO_SPEED_FAST;
    init.Alternate = USART_DBG_TX_AF;
    HAL_GPIO_Init(USART_DBG_TX_GPIO_PORT, &init);
    init.Pin = USART_DBG_RX_PIN;
    init.Alternate = USART_DBG_RX_AF;
    HAL_GPIO_Init(USART_DBG_RX_GPIO_PORT, &init);
}

static void usartConfig(void)
{
    uartDbgHandle.Instance = USART_DBG;
    uartDbgHandle.Init.BaudRate = 115200;
    uartDbgHandle.Init.WordLength = UART_WORDLENGTH_8B;
    uartDbgHandle.Init.StopBits = UART_STOPBITS_1;
    uartDbgHandle.Init.Parity = UART_PARITY_NONE;
    uartDbgHandle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    uartDbgHandle.Init.Mode = UART_MODE_TX;
    uartDbgHandle.Init.OverSampling = UART_OVERSAMPLING_16;

    if(HAL_UART_Init(&uartDbgHandle) != HAL_OK)
        abort();
}
#endif

void initDbgUSART(void)
{
#ifdef ENABLE_DEBUG_OUTPUT
    rccConfig();
    gpioConfig();
    usartConfig();
#endif
}
