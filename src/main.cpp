#include "stm32f4xx_hal.h"
#include "usbh_core.h"
#include "usbh_hid.h"
#include "config.h"
#include "dbg-out.h"
#include "led.h"
#include "hid-keybd.h"
#include "ps2-kbd-emulator.h"

enum class State : uint8_t
{
    Ready,
    Error,
    Idle,
};

static volatile State usbState = State::Idle;
static volatile State processingState = State::Idle;

void USBH_UserProcess(USBH_HandleTypeDef* phost, uint8_t id)
{
    switch(id)
    {
    case HOST_USER_CONNECTION:
        printf("USB device connected\n");
        ledOn(LED_ORANGE);
        break;
    case HOST_USER_DISCONNECTION:
        usbState = State::Idle;
        processingState = State::Idle;
        printf("USB device disconnected\n");
        ledsOff();
        break;
    case HOST_USER_CLASS_ACTIVE:
        usbState = State::Ready;
        printf("USB device ready\n");
        ledsOff();
        ledOn(LED_GREEN);
        break;
    case HOST_USER_UNRECOVERED_ERROR:
        usbState = State::Error;
        printf("Unrecovered USB error\n");
        ledsOff();
        ledOn(LED_RED);
        break;
    case HOST_USER_SELECT_CONFIGURATION:
        printf("Selecting USB configuration...\n");
        ledsOff();
        ledOn(LED_RED);
        ledOn(LED_ORANGE);
        ledOn(LED_GREEN);
        break;
    case HOST_USER_CLASS_SELECTED:
        printf("USB class selected\n");
        ledsOff();
        ledOn(LED_RED);
        ledOn(LED_GREEN);
        break;
    default:
        ledsOff();
        ledOn(LED_RED);
        ledOn(LED_ORANGE);
        break;
    }
}

void HID_UserProcess(USBH_HandleTypeDef *phost)
{
    static HID_TypeTypeDef hidType;
    switch(processingState)
    {
    case State::Idle:
        hidType = USBH_HID_GetDeviceType(phost);
        if(hidType == HID_KEYBOARD)
        {
            printf("Keyboard detected\n");
            if(USBH_HID_KeybdInit(phost) != USBH_OK)
            {
                printf("Failed to init keyboard\n");
                processingState = State::Error;
            }
        }
        else if(hidType == HID_MOUSE)
            printf("USB mouse detected. We don't support mice.\n");
        processingState = State::Ready;
        break;
    case State::Ready:
        if(hidType == HID_KEYBOARD)
            HID_Keybd_UserProcess(phost);
        break;
    case State::Error:
        break;
    }
}

int main(void)
{
    HAL_Init();
    ledInit();
    SystemClock_Config();
    initDbgUSART();

    if(SysTick_Config(SystemCoreClock / 1000))
        abort();

    PS2_Init();

    USBH_HandleTypeDef hUSBHost;
    USBH_Init(&hUSBHost, USBH_UserProcess, 0);
    USBH_RegisterClass(&hUSBHost, USBH_HID_CLASS);
    USBH_Start(&hUSBHost);

    printf("USB-to-PS/2 keyboard converter initialized\n");

    while(true)
    {
        PS2_Process();
        USBH_Process(&hUSBHost);
        if(usbState == State::Ready)
            HID_UserProcess(&hUSBHost);
    }
}
