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

void USBH_UserProcess(USBH_HandleTypeDef* phost, uint8_t event)
{
    switch(event)
    {
    case HOST_USER_CONNECTION:
        USBH_UsrLog("USB device connected");
        ledOn(LED_ORANGE);
        break;
    case HOST_USER_DISCONNECTION:
        usbState = State::Idle;
        processingState = State::Idle;
        USBH_UsrLog("USB device disconnected");
        ledsOff();
        break;
    case HOST_USER_CLASS_ACTIVE:
        usbState = State::Ready;
        USBH_UsrLog("USB device ready");
        ledsOff();
        ledOn(LED_GREEN);
        break;
    case HOST_USER_UNRECOVERED_ERROR:
        usbState = State::Error;
        USBH_UsrLog("Unrecovered USB error");
        ledsOff();
        ledOn(LED_RED);
        break;
    case HOST_USER_SELECT_CONFIGURATION:
        USBH_UsrLog("Selecting USB configuration...");
        ledsOff();
        ledOn(LED_RED);
        ledOn(LED_ORANGE);
        ledOn(LED_GREEN);
        break;
    case HOST_USER_CLASS_SELECTED:
        USBH_UsrLog("USB class selected");
        ledsOff();
        ledOn(LED_RED);
        ledOn(LED_GREEN);
        break;
    default:
        USBH_UsrLog("Unknown USB event %u\n", (unsigned)event);
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
            USBH_UsrLog("Keyboard detected");
            if(USBH_HID_KeybdInit(phost) != USBH_OK)
            {
                USBH_UsrLog("Failed to init keyboard");
                processingState = State::Error;
            }
        }
        else if(hidType == HID_MOUSE)
            USBH_UsrLog("USB mouse detected. We don't support mice.");
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

    USBH_UsrLog("USB-to-PS/2 keyboard converter initialized");

    while(true)
    {
        PS2_Process();
        USBH_Process(&hUSBHost);
        if(usbState == State::Ready)
            HID_UserProcess(&hUSBHost);
    }
}
