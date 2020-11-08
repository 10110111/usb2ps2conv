#include <stdbool.h>
#include "usbh_core.h"
#include "usbh_hid_keybd.h"
#include "scancodes2.h"
#include "ps2-kbd-emulator.h"

void emitScanCode(const uint8_t* code)
{
    const unsigned N=code[0];
    if(N==0) return;
    for(unsigned n=0; n<=N; ++n)
        passByteToPS2(code[n]);
}

typedef enum
{
    KS_UP,
    KS_DOWN,
    KS_AUTOREPEAT,
} KeyState;

typedef struct
{
    unsigned long leds;
    bool ctrl, shift, alt;
    bool ledsUpdated;
} EmulatedKeyboardState;
EmulatedKeyboardState emuState = {.leds=0, .ctrl=false, .shift=false, .alt=false, .ledsUpdated=false};

void setUSBKeyboardLEDs(uint8_t leds)
{
    emuState.leds=leds;
    emuState.ledsUpdated=true;
}

void processUSBKeyboardEvent(const uint8_t key, const KeyState state)
{
    const bool numLockLED = emuState.leds&1;

    const uint8_t* scancode=NULL;
    switch(state)
    {
    case KS_UP:
        if(key==KEY_LEFTCONTROL || key==KEY_RIGHTCONTROL)
            emuState.ctrl=false;
        else if(key==KEY_RIGHTSHIFT || key==KEY_LEFTSHIFT)
            emuState.shift=false;
        else if(key==KEY_LEFTALT || key==KEY_RIGHTALT)
            emuState.alt=false;
        scancode=keyToBreakCode(key, emuState.ctrl, emuState.shift, emuState.alt, numLockLED);
        break;
    case KS_DOWN:
    case KS_AUTOREPEAT:
        if(key==KEY_LEFTCONTROL || key==KEY_RIGHTCONTROL)
            emuState.ctrl=true;
        else if(key==KEY_RIGHTSHIFT || key==KEY_LEFTSHIFT)
            emuState.shift=true;
        else if(key==KEY_LEFTALT || key==KEY_RIGHTALT)
            emuState.alt=true;
        scancode=keyToMakeCode(key, emuState.ctrl, emuState.shift, emuState.alt, numLockLED, state==KS_AUTOREPEAT);
        break;
    }
    emitScanCode(scancode);
}

typedef struct
{
    uint8_t lctrl : 1;
    uint8_t lshift : 1;
    uint8_t lalt : 1;
    uint8_t lgui : 1;
    uint8_t rctrl : 1;
    uint8_t rshift : 1;
    uint8_t ralt : 1;
    uint8_t rgui : 1;
    uint8_t reserved;
    uint8_t keys[6];
} USBKeyboardReport;

#define KEY_BUF_SIZE (6+8) // 6 for keys[], 8 for the ones in the bitmap
static uint8_t pressedKeysUSB[KEY_BUF_SIZE];

void addPressedKey(uint8_t *keys, const uint8_t key)
{
    for(unsigned n=0; n<KEY_BUF_SIZE; ++n)
    {
        if(keys[n]) continue;

        keys[n]=key;
        return;
    }
}

void keyPressed(const uint8_t key)
{
    addPressedKey(pressedKeysUSB, key);
    processUSBKeyboardEvent(key, KS_DOWN);
}

void keyReleased(const uint8_t key)
{
    for(unsigned n=0; n<KEY_BUF_SIZE; ++n)
    {
        if(pressedKeysUSB[n]!=key)
            continue;
        pressedKeysUSB[n]=0;
        break;
    }
    processUSBKeyboardEvent(key, KS_UP);
}

static void doSetLEDs(USBH_HandleTypeDef *phost)
{
    enum
    {
        PS2_SCROLL_LOCK=1,
        PS2_NUM_LOCK   =2,
        PS2_CAPS_LOCK  =4,
    };
    enum
    {
        HID_NUM_LOCK   =1,
        HID_CAPS_LOCK  =2,
        HID_SCROLL_LOCK=4,
    };

    uint8_t state=0;
    if(emuState.leds & PS2_SCROLL_LOCK)
        state |= HID_SCROLL_LOCK;
    if(emuState.leds & PS2_NUM_LOCK)
        state |= HID_NUM_LOCK;
    if(emuState.leds & PS2_CAPS_LOCK)
        state |= HID_CAPS_LOCK;

    enum
    {
        REPORT_INPUT=0x01,
        REPORT_OUTPUT=0x02,
        REPORT_FEATURE=0x03,
    };

    printf("Setting LEDs: NUM %s, CAPS %s, SCROLL %s\n",
           state & HID_NUM_LOCK ? "on" : "off",
           state & HID_CAPS_LOCK ? "on" : "off",
           state & HID_SCROLL_LOCK ? "on" : "off");

    USBH_StatusTypeDef result;
    do
    {
        result=USBH_HID_SetReport(phost, REPORT_OUTPUT, 0, &state, 1);
    }
    while(result==USBH_BUSY);
    if(result!=USBH_OK)
        printf("Failed to Set_Report: error %u\n", (unsigned)result);
}

void HID_Keybd_UserProcess(USBH_HandleTypeDef *phost)
{
    if(emuState.ledsUpdated)
    {
        doSetLEDs(phost);
        emuState.ledsUpdated=false;
    }

    HID_HandleTypeDef*const hidHandle = (HID_HandleTypeDef*)phost->pActiveClass->pData;
    USBKeyboardReport report;
    if(USBH_HID_FifoRead(&hidHandle->fifo, &report, hidHandle->length) !=  hidHandle->length)
        return;

    printf("Keyboard report: 0x%08lx%08lx\n", ((uint32_t*)&report)[1], *(uint32_t*)&report);

    uint8_t currentPressedKeys[KEY_BUF_SIZE]={0};
    memcpy(&currentPressedKeys, report.keys, sizeof report.keys);
    if(report.lctrl)
        addPressedKey(currentPressedKeys, KEY_LEFTCONTROL);
    if(report.lshift)
        addPressedKey(currentPressedKeys, KEY_LEFTSHIFT);
    if(report.lalt)
        addPressedKey(currentPressedKeys, KEY_LEFTALT);
    if(report.lgui)
        addPressedKey(currentPressedKeys, KEY_LEFT_GUI);
    if(report.rctrl)
        addPressedKey(currentPressedKeys, KEY_RIGHTCONTROL);
    if(report.rshift)
        addPressedKey(currentPressedKeys, KEY_RIGHTSHIFT);
    if(report.ralt)
        addPressedKey(currentPressedKeys, KEY_RIGHTALT);
    if(report.rgui)
        addPressedKey(currentPressedKeys, KEY_RIGHT_GUI);

    for(unsigned n=0; n<KEY_BUF_SIZE; ++n)
    {
        const uint8_t currentKey=currentPressedKeys[n];
        if(!memchr(pressedKeysUSB, currentKey, KEY_BUF_SIZE))
            keyPressed(currentKey);
    }

    for(unsigned n=0; n<KEY_BUF_SIZE; ++n)
    {
        const uint8_t oldKey=pressedKeysUSB[n];
        if(!memchr(currentPressedKeys, oldKey, KEY_BUF_SIZE))
            keyReleased(oldKey);
    }

    memcpy(pressedKeysUSB, currentPressedKeys, KEY_BUF_SIZE);
}
