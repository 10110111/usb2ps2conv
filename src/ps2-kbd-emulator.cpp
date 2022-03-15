#include <stdio.h>
#include <stdlib.h>
#include "RingBuffer.hpp"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_conf.h"
#include "stm32f4xx_hal_tim.h"
#include "ps2-kbd-emulator.h"
#include "hid-keybd.h"
#include "util.h"

// Reference used: https://www.avrfreaks.net/sites/default/files/PS2%20Keyboard.pdf

// HW-configuration-dependent and protocol-constrained values
constexpr uint32_t QUADRUPLE_CLK_RATE=4*12500u; // Frequency must be 10..16.7 kHz (i.e. full period 60..100 us).
constexpr uint32_t DELAY_MS_BEFORE_SENDING_BAT_CODE=550; // Must be 500..750

#define DATA_GPIO_LETTER E
#define DATA_PIN_NUM 6
#define DATA_GPIO CONCAT(GPIO,DATA_GPIO_LETTER,)
#define DATA_PIN DATA_GPIO,DATA_PIN_NUM
#define DATA_PIN_ENABLE() CONCAT(__HAL_RCC_GPIO,DATA_GPIO_LETTER,_CLK_ENABLE())

#define CLK_GPIO_LETTER C
#define CLK_PIN_NUM 13
#define CLK_GPIO CONCAT(GPIO,CLK_GPIO_LETTER,)
#define CLK_PIN CLK_GPIO,CLK_PIN_NUM
#define CLK_PIN_ENABLE() CONCAT(__HAL_RCC_GPIO,CLK_GPIO_LETTER,_CLK_ENABLE())

enum HostCommand
{
    CMD_RESET=0xFF,
    CMD_RESEND=0xFE,
    CMD_SET_KEY_TYPE_MAKE=0xFD,
    CMD_SET_KEY_TYPE_MAKE_BREAK=0xFC,
    CMD_SET_KEY_TYPE_TYPEMATIC=0xFB,
    CMD_SET_ALL_KEYS_TYPEMATIC_MAKE_BREAK=0xFA,
    CMD_SET_ALL_KEYS_MAKE=0xF9,
    CMD_SET_ALL_KEYS_MAKE_BREAK=0xF8,
    CMD_SET_ALL_KEYS_TYPEMATIC=0xF7,
    CMD_SET_DEFAULT=0xF6,
    CMD_DISABLE=0xF5,
    CMD_ENABLE=0xF4,
    CMD_SET_TYPEMATIC_RATE=0xF3,
    CMD_READ_ID=0xF2,
    CMD_SET_SCAN_CODE_SET=0xF0,
    CMD_ECHO=0xEE,
    CMD_SET_LEDS=0xED,
};
enum Reply
{
    REPLY_RESEND=0xFE,
    REPLY_ACKNOWLEDGE=0xFA,
    REPLY_ECHO=0xEE,
    REPLY_BAT_SUCCESS=0xAA,
    REPLY_ID_BYTE_0=0xAB,
    REPLY_ID_BYTE_1=0x83,
};

static constexpr uint32_t repeatRatePeriodsInTicks[]=
{
    uint32_t(0.5 + QUADRUPLE_CLK_RATE / 30.0),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE / 26.7),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE / 24.0),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE / 21.8),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE / 20.7),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE / 18.5),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE / 17.1),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE / 16.0),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE / 15.0),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE / 13.3),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE / 12.0),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE / 10.9),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE / 10.0),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  9.2),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  8.6),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  8.0),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  7.5),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  6.7),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  6.0),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  5.5),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  5.0),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  4.6),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  4.3),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  4.0),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  3.7),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  3.3),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  3.0),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  2.7),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  2.5),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  2.3),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  2.1),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE /  2.0),
};
static_assert(sizeof repeatRatePeriodsInTicks / sizeof repeatRatePeriodsInTicks[0]==0x20);

static constexpr uint32_t repeatDelaysInTicks[]=
{
    uint32_t(0.5 + QUADRUPLE_CLK_RATE * 0.25),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE * 0.50),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE * 0.75),
    uint32_t(0.5 + QUADRUPLE_CLK_RATE * 1.00),
};
static_assert(sizeof repeatDelaysInTicks / sizeof repeatDelaysInTicks[0]==4);

volatile uint32_t autorepeatTickCounter=0;
uint32_t autorepeatPeriodInTicks=repeatRatePeriodsInTicks[0x0B];
uint32_t autorepeatDelayInTicks =repeatDelaysInTicks[1];
volatile bool kbdEnabled=true;
volatile bool kbdBusy=true; // Busy by default until we enter main loop
uint8_t lastSentByte=0xAA;

static void inMode(GPIO_TypeDef* gpio, const uint16_t pin)
{
    gpio->MODER &= ~(GPIO_MODER_MODER0<<2*pin); // clear the mode, this means input
}
static void outMode(GPIO_TypeDef* gpio, const uint16_t pin)
{
    auto moder=gpio->MODER;
    moder &= ~(GPIO_MODER_MODER0<<2*pin); // clear mode bits
    moder |= GPIO_MODER_MODER0_0<<2*pin; // set GP output mode
    gpio->MODER = moder;
}
static void high(GPIO_TypeDef* gpio, const uint16_t pin)
{
    inMode(gpio,pin);
}
static void low(GPIO_TypeDef* gpio, const uint16_t pin)
{
    outMode(gpio,pin);
    gpio->BSRR = 1u<<(pin+16); // set the pin low
}
static bool read(GPIO_TypeDef* gpio, const uint16_t pin)
{
    return gpio->IDR & 1u<<pin;
}

class BusDriver
{
public:
    enum class TransmissionStatus : uint8_t
    {
        Complete,
        Interrupted, // By Inhibit condition during sending or receiving, or if sender was busy with another byte
        InProgress,
        Failed, // Due to parity error when receiving
    };

private:
    // The bus is free if CLK and DATA are high for at least 50 us
    static constexpr uint8_t NUM_TICKS_TO_MARK_BUS_AS_FREE=1+50*QUADRUPLE_CLK_RATE/1000'000;
    volatile enum class State : uint8_t
    {
        WaitingForEvents,
        SendingByte_UpdateDATA,
        SendingByte_LowerCLK,
        SendingByte_RaiseCLK,
        SendingByte_WaitingBeforeRaisingCLK,
        ReadingHostByte_LowerCLK,
        ReadingHostByte_RaiseCLKAndReadDATA,
        ReadingHostByte_SendingAckBit_FinalCLKLowering,
        ReadingHostByte_SendingAckBit_FinalCLKRaise,
        ReadingHostByte_SendingAckBit_LowerCLK,
        ReadingHostByte_SendingAckBit_LowerDATA,
        ReadingHostByte_SendingAckBit_RaisingDATA,
        ReadingHostByte_SendingAckBit_WaitingBeforeFinalCLKRaise,
        ReadingHostByte_SendingAckBit_WaitingBeforeRaisingCLK,
        ReadingHostByte_WaitingBeforeLoweringCLK,
        ReadingHostByte_WaitingBeforeRaisingCLK,
    } nextState = State::WaitingForEvents;

    uint8_t numTicksBusFree=0; // Number of consecutive timer periods the bus has been found free
    enum class BusState : uint8_t
    {
        Free,
        Inhibit,     // CLK low, DATA high
        Low=Inhibit, // Both CLK and DATA are low; we consider this to be equivalent to Inhibit
        ReqToSend,   // CLK high, DATA low
        Freeing,     // Both CLK and DATA are high, but not for sufficiently long yet
    } busState=BusState::Low;

    union
    {
        uint8_t byteToSend;
        uint8_t byteReceived;
    };
    union
    {
        uint8_t parityToSend;
        uint8_t parityOfBitsReceived;
    };
    volatile bool needToSendByte=false;
    union
    {
        // Number of bits of current byte+start+stop+parity that have already been set on the DATA line
        uint8_t numBitsSent;
        uint8_t numBitsReceived;
    };
    // Number of times we've lowered CLK line
    volatile TransmissionStatus sendingStatus_   =TransmissionStatus::Complete;
    volatile TransmissionStatus receptionStatus_=TransmissionStatus::Complete;
    volatile bool byteReceivedAvailable_=false;


    void switchToByteSendState()
    {
        sendingStatus_=TransmissionStatus::InProgress;
        nextState=State::SendingByte_UpdateDATA;
        numBitsSent=0;
        parityToSend=1; // Odd parity. If all 8 bits are equal, XORing them with this will result in 1.
    }

    void switchToByteReceiveState()
    {
        receptionStatus_=TransmissionStatus::InProgress;
        nextState=State::ReadingHostByte_LowerCLK;
        numBitsReceived=1; // Start bit is already included in RTS state
        byteReceived=0;
        byteReceivedAvailable_=false;
        parityOfBitsReceived=1; // Odd parity. If all 8 bits are equal, XORing them with this will result in 1.
    }

public:
    void init()
    {
        CLK_PIN_ENABLE();
        DATA_PIN_ENABLE();
        // Disable internal pull-ups, their resistance is too large for our needs.
        // Moreover, they pull to 3.3V, while we need 5V.
        // External 10k resistors should be attached as pull-ups.
        CLK_GPIO ->PUPDR &= ~(1<< CLK_PIN_NUM);
        DATA_GPIO->PUPDR &= ~(1<<DATA_PIN_NUM);

        inMode(DATA_PIN);
        inMode(CLK_PIN);
    }

    TransmissionStatus sendingStatus() const { return sendingStatus_; }
    TransmissionStatus receptionStatus() const { return receptionStatus_; }
    uint8_t getByteReceived()
    {
        if(!byteReceivedAvailable_) return 0;
        byteReceivedAvailable_=false;
        return byteReceived;
    }
    bool byteReceivedAvailable() const { return byteReceivedAvailable_; }
    void clearReceptionStatus() { receptionStatus_=TransmissionStatus::Complete; }

    bool isIdle() const { return nextState==State::WaitingForEvents && !needToSendByte; }

    void sendByte(uint8_t byte)
    {
        USBH_UsrLog("sendByte(%02X)", (unsigned)byte);
        if(nextState!=State::WaitingForEvents || byteReceivedAvailable_)
        {
            sendingStatus_=TransmissionStatus::Interrupted;
            return;
        }

        __disable_irq();
        needToSendByte=true;
        byteToSend=byte;
        __enable_irq();
    }

    void handleISR()
    {
        switch(nextState)
        {
        case State::WaitingForEvents:
        {
            // Make sure we don't hold the bus
            high(CLK_PIN);
            high(DATA_PIN);
            // Check bus state
            const bool clk =read( CLK_PIN);
            const bool data=read(DATA_PIN);
            if(clk && data)
            {
                if(numTicksBusFree<NUM_TICKS_TO_MARK_BUS_AS_FREE)
                    ++numTicksBusFree;

                if(numTicksBusFree==NUM_TICKS_TO_MARK_BUS_AS_FREE)
                    busState=BusState::Free;
                else
                    busState=BusState::Freeing;
            }
            else
            {
                // Bus has become non-free
                numTicksBusFree=0;
                if(!clk && data)
                    busState=BusState::Inhibit;
                else if(clk && !data)
                    busState=BusState::ReqToSend;
                else
                    busState=BusState::Low;
            }

            if(busState==BusState::ReqToSend && !byteReceivedAvailable_) // Only read a new byte if previous one has been consumed
                switchToByteReceiveState();
            else if(busState==BusState::Free && needToSendByte)
                switchToByteSendState();
            break;
        }

        case State::SendingByte_UpdateDATA:
        {
            const uint8_t bit=byteToSend&1;
            if(numBitsSent==0)
            {
                // Start bit
                low(DATA_PIN);
            }
            else if(numBitsSent<9)
            {
                if(bit) high(DATA_PIN); else low(DATA_PIN);
                byteToSend >>= 1;
                parityToSend ^= bit;
            }
            else if(numBitsSent==9)
            {
                // Parity bit
                if(parityToSend) high(DATA_PIN); else low(DATA_PIN);
            }
            else if(numBitsSent==10)
            {
                // Stop bit
                high(DATA_PIN);
            }
            else if(numBitsSent==11)
            {
                sendingStatus_=TransmissionStatus::Complete;
                needToSendByte=false;
                nextState=State::WaitingForEvents;
                break;
            }
            ++numBitsSent;
            nextState=State::SendingByte_LowerCLK;
            break;
        }

        case State::SendingByte_LowerCLK:
            if(!read(CLK_PIN))
            {
                // Inhibit condition detected
                sendingStatus_=TransmissionStatus::Interrupted;
                needToSendByte=false;
                nextState=State::WaitingForEvents;
                break;
            }
            low(CLK_PIN);
            nextState=State::SendingByte_WaitingBeforeRaisingCLK;
            break;
        case State::SendingByte_WaitingBeforeRaisingCLK:
            nextState=State::SendingByte_RaiseCLK;
            break;
        case State::SendingByte_RaiseCLK:
            high(CLK_PIN);
            nextState=State::SendingByte_UpdateDATA;
            break;
        case State::ReadingHostByte_LowerCLK:
            if(byteReceivedAvailable_) break; // The byte hasn't been consumed yet.
            if(!read(CLK_PIN))
            {
                // Host aborted the transmission
                nextState=State::WaitingForEvents;
                break;
            }
            low(CLK_PIN);
            nextState=State::ReadingHostByte_WaitingBeforeRaisingCLK;
            break;
        case State::ReadingHostByte_WaitingBeforeRaisingCLK:
            nextState=State::ReadingHostByte_RaiseCLKAndReadDATA;
            break;
        case State::ReadingHostByte_RaiseCLKAndReadDATA:
        {
            high(CLK_PIN);
            const uint8_t bit=!!read(DATA_PIN);
            if(numBitsReceived<9)
            {
                // Data bit
                byteReceived = byteReceived>>1 | bit<<7;
                parityOfBitsReceived ^= bit;
            }
            else if(numBitsReceived==9)
            {
                // Parity bit
                parityOfBitsReceived ^= bit;
            }
            else if(numBitsReceived==10)
            {
                // Stop bit
                if(bit && !parityOfBitsReceived)
                {
                    receptionStatus_=TransmissionStatus::Complete;
                    byteReceivedAvailable_=true;
                }
                else
                {
                    receptionStatus_=TransmissionStatus::Failed;
                }
                nextState=State::ReadingHostByte_SendingAckBit_LowerDATA;
                break;
            }
            ++numBitsReceived;
            nextState=State::ReadingHostByte_WaitingBeforeLoweringCLK;
            break;
        }
        case State::ReadingHostByte_WaitingBeforeLoweringCLK:
            if(read(CLK_PIN))
                nextState=State::ReadingHostByte_LowerCLK;
            else
                nextState=State::WaitingForEvents; // Host aborted the transmission
            break;
        case State::ReadingHostByte_SendingAckBit_LowerDATA:
            low(DATA_PIN);
            nextState=State::ReadingHostByte_SendingAckBit_LowerCLK;
            break;
        case State::ReadingHostByte_SendingAckBit_LowerCLK:
            low(CLK_PIN);
            nextState=State::ReadingHostByte_SendingAckBit_WaitingBeforeRaisingCLK;
            break;
        case State::ReadingHostByte_SendingAckBit_WaitingBeforeRaisingCLK:
            high(CLK_PIN);
            nextState=State::ReadingHostByte_SendingAckBit_RaisingDATA;
            break;
        case State::ReadingHostByte_SendingAckBit_RaisingDATA:
            high(DATA_PIN);
            nextState=State::ReadingHostByte_SendingAckBit_FinalCLKLowering;
            break;
        case State::ReadingHostByte_SendingAckBit_FinalCLKLowering:
            low(CLK_PIN);
            nextState=State::ReadingHostByte_SendingAckBit_WaitingBeforeFinalCLKRaise;
            break;
        case State::ReadingHostByte_SendingAckBit_WaitingBeforeFinalCLKRaise:
            nextState=State::ReadingHostByte_SendingAckBit_FinalCLKRaise;
            break;
        case State::ReadingHostByte_SendingAckBit_FinalCLKRaise:
            high(CLK_PIN);
            // FIXME: we aren't clocking in until the host releases the DATA line (when this happens
            // to be needed, it's an error, but the protocol seems to require this from the device).
            nextState=State::WaitingForEvents;
            break;
        }
    }
} busDriver;

extern "C" void TIM3_IRQHandler()
{
    busDriver.handleISR();
    TIM3->SR &= ~TIM_IT_UPDATE;

    ++autorepeatTickCounter;
}

static void setLEDs(uint8_t state)
{
    setUSBKeyboardLEDs(state);
}

// Stores scan codes in the format: count=N|(modifiers...), byte1, byte2, ..., byteN
RingBuffer<32> keyboardBuffer;
uint8_t numBytesToReceiveInCurrentScanCode=0;
bool beginningOfCurrentScanCodeWasSkipped=false;

uint8_t sentBytesFromCurrentScanCode=0;
static void typeNextScanCode()
{
    if(keyboardBuffer.empty()) return;
    const auto count=keyboardBuffer[0];
    if(count+1u>keyboardBuffer.size()) return; // Haven't read enough bytes from the controlling keyboard
    if(!busDriver.isIdle()) return;

    if(sentBytesFromCurrentScanCode<count)
    {
        enum class State
        {
            Idle,
            WaitingForCompletion,
        };
        static State state=State::Idle;
        switch(state)
        {
        case State::Idle:
            busDriver.sendByte(keyboardBuffer[sentBytesFromCurrentScanCode+1]);
            state=State::WaitingForCompletion;
            return;
        case State::WaitingForCompletion:
            if(!busDriver.isIdle())
                return;
            state=State::Idle;
            const auto status=busDriver.sendingStatus();
            if(status==BusDriver::TransmissionStatus::Complete)
                ++sentBytesFromCurrentScanCode;
            if(status==BusDriver::TransmissionStatus::Interrupted)
                sentBytesFromCurrentScanCode=0; // must re-transmit the whole chunk
            if(sentBytesFromCurrentScanCode!=count)
                return; // Will continue in the next call
            break;
        }
    }
    sentBytesFromCurrentScanCode=0;
    // Remove the finished scan code atomically: we want to make sure the first
    // byte, if present, always denotes the length of the scan code, even in ISR.
    __disable_irq();
    for(uint8_t i=0;i<count+1;++i)
        keyboardBuffer.pop_front();
    __enable_irq();
}

static void clearKbdBuffer()
{
    if(numBytesToReceiveInCurrentScanCode)
        beginningOfCurrentScanCodeWasSkipped=true;
    keyboardBuffer.clear();
    sentBytesFromCurrentScanCode=0;
}

void PS2_Process()
{
    enum class KeyboardState
    {
        Initialization,
        DelayBeforeBAT,
        WaitingForCommands,
        SendingACK,
        SendingACK_WaitingForTransmissionEnd,
        EnablingKbd,
        DisablingKbd,
        SavingLastCommand,
        SendingBAT_WaitingForTransmissionEnd,
        ReplyingWithResend,
        ReplyingWithEcho,
        ResendingLastByte,
        SettingLEDs,
    };
    static KeyboardState kbdState=KeyboardState::Initialization;

    static uint8_t lastCommand=0;
    static uint8_t lastCmdToSetAfterAck=0;
    static KeyboardState stateToGoToAfterAck;
    static uint8_t setLEDsCmdArg;
    static uint32_t BATStartTimeMs;
    switch(kbdState)
    {
    case KeyboardState::Initialization:
        kbdEnabled=false;
        setLEDs(7);
        // We succeed in any case. Just need to wait 500-750 ms before sending success code.
        BATStartTimeMs=HAL_GetTick();
        kbdState=KeyboardState::DelayBeforeBAT;
        USBH_UsrLog("Starting BAT...");
        break;
    case KeyboardState::DelayBeforeBAT:
        if(HAL_GetTick() - BATStartTimeMs < DELAY_MS_BEFORE_SENDING_BAT_CODE)
            break;
        setLEDs(0);
        autorepeatPeriodInTicks=repeatRatePeriodsInTicks[0x0B];
        autorepeatDelayInTicks =repeatDelaysInTicks[1];
        busDriver.sendByte(REPLY_BAT_SUCCESS);
        kbdState=KeyboardState::SendingBAT_WaitingForTransmissionEnd;
        USBH_UsrLog("Ending BAT...");
        break;
    case KeyboardState::SendingBAT_WaitingForTransmissionEnd:
        if(!busDriver.isIdle())
            break;
        kbdEnabled=true;
        kbdBusy=false;
        kbdState=KeyboardState::WaitingForCommands;
        USBH_UsrLog("BAT done");
        break;
    case KeyboardState::WaitingForCommands:
        kbdBusy=false;
        if(busDriver.receptionStatus()==BusDriver::TransmissionStatus::Failed)
        {
            busDriver.clearReceptionStatus();
            kbdState=KeyboardState::ReplyingWithResend;
            break;
        }
        if(busDriver.byteReceivedAvailable())
        {
            // Request to send detected, accept the command
            const auto byte=busDriver.getByteReceived();
            USBH_UsrLog("Got byte from host: %02X", (unsigned)byte);

            if((lastCommand&0x80)!=0 && (byte&0x80)==0)
            {
                const auto arg=byte;
                const auto cmd=lastCommand;
                lastCommand=0;

                kbdState=KeyboardState::SendingACK;

                switch(cmd)
                {
                case CMD_SET_TYPEMATIC_RATE:
                    stateToGoToAfterAck=KeyboardState::WaitingForCommands;
                    autorepeatPeriodInTicks = repeatRatePeriodsInTicks[arg&0x1f];
                    autorepeatDelayInTicks  = repeatDelaysInTicks[arg>>6];
                    USBH_UsrLog("Handling CMD_SET_TYPEMATIC_RATE");
                    break;
                case CMD_SET_SCAN_CODE_SET:
                    // Ignore, we only support set 2
                    stateToGoToAfterAck=KeyboardState::WaitingForCommands;
                    USBH_UsrLog("CMD_SET_SCAN_CODE_SET is not supported, ignoring");
                    break;
                case CMD_SET_LEDS:
                    // Got the Set LEDs command, send it to the real keyboard.
                    stateToGoToAfterAck=KeyboardState::SettingLEDs;
                    setLEDsCmdArg=arg;
                    USBH_UsrLog("Handling CMD_SET_LEDS");
                    break;
                }
                return;
            }

            const auto cmd=byte;
            if(cmd!=CMD_RESEND)
            {
                kbdBusy=true;
                clearKbdBuffer();
            }

            switch(cmd)
            {
            case CMD_READ_ID:
                keyboardBuffer.push_back(3);
                keyboardBuffer.push_back(REPLY_ACKNOWLEDGE);
                keyboardBuffer.push_back(REPLY_ID_BYTE_0);
                keyboardBuffer.push_back(REPLY_ID_BYTE_1);
                break;
            case CMD_SET_TYPEMATIC_RATE:
            case CMD_SET_SCAN_CODE_SET:
            case CMD_SET_LEDS:
            {
                lastCmdToSetAfterAck=cmd;
                kbdState=KeyboardState::SendingACK;
                stateToGoToAfterAck=KeyboardState::SavingLastCommand;
                break;
            }
            case CMD_RESET:
                kbdState=KeyboardState::SendingACK;
                stateToGoToAfterAck=KeyboardState::Initialization;
                USBH_UsrLog("Handling CMD_RESET");
                break;
            case CMD_DISABLE:
                kbdState=KeyboardState::SendingACK;
                stateToGoToAfterAck=KeyboardState::DisablingKbd;
                USBH_UsrLog("Handling CMD_DISABLE");
                break;
            case CMD_ENABLE:
                kbdState=KeyboardState::SendingACK;
                stateToGoToAfterAck=KeyboardState::EnablingKbd;
                USBH_UsrLog("Handling CMD_ENABLE");
                break;
            case CMD_ECHO:
                kbdState=KeyboardState::ReplyingWithEcho;
                USBH_UsrLog("Handling CMD_ECHO");
                break;
            case CMD_RESEND:
                kbdState=KeyboardState::ResendingLastByte;
                USBH_UsrLog("Handling CMD_RESEND");
                break;
            case CMD_SET_KEY_TYPE_MAKE:
            case CMD_SET_KEY_TYPE_MAKE_BREAK:
            case CMD_SET_KEY_TYPE_TYPEMATIC:
            case CMD_SET_ALL_KEYS_TYPEMATIC_MAKE_BREAK:
            case CMD_SET_ALL_KEYS_MAKE:
            case CMD_SET_ALL_KEYS_MAKE_BREAK:
            case CMD_SET_ALL_KEYS_TYPEMATIC:
            case CMD_SET_DEFAULT:
                kbdState=KeyboardState::SendingACK;
                stateToGoToAfterAck=KeyboardState::WaitingForCommands;
                break;
            default:
                USBH_UsrLog("Failed to interpret command %02X, resending last sent byte", (unsigned)cmd);
                kbdState=KeyboardState::ResendingLastByte;
                break;
            }
        }
        else if(kbdEnabled && !keyboardBuffer.empty())
            typeNextScanCode();
        break;
    case KeyboardState::SendingACK:
        if(!busDriver.isIdle())
            break;
        busDriver.sendByte(REPLY_ACKNOWLEDGE);
        kbdState=KeyboardState::SendingACK_WaitingForTransmissionEnd;
        break;
    case KeyboardState::SendingACK_WaitingForTransmissionEnd:
        if(!busDriver.isIdle())
            break;
        if(busDriver.sendingStatus()==BusDriver::TransmissionStatus::Complete)
            kbdState=stateToGoToAfterAck;
        else
            kbdState=KeyboardState::WaitingForCommands;
        break;
    case KeyboardState::DisablingKbd:
        kbdEnabled=false;
        kbdState=KeyboardState::WaitingForCommands;
        break;
    case KeyboardState::EnablingKbd:
        kbdEnabled=true;
        kbdState=KeyboardState::WaitingForCommands;
        break;
    case KeyboardState::SavingLastCommand:
        lastCommand=lastCmdToSetAfterAck;
        kbdState=KeyboardState::WaitingForCommands;
        break;
    case KeyboardState::ReplyingWithResend:
        if(!busDriver.isIdle())
            break;
        busDriver.sendByte(REPLY_RESEND);
        kbdState=KeyboardState::WaitingForCommands;
        break;
    case KeyboardState::ReplyingWithEcho:
        if(!busDriver.isIdle())
            break;
        busDriver.sendByte(REPLY_ECHO);
        kbdState=KeyboardState::WaitingForCommands;
        break;
    case KeyboardState::ResendingLastByte:
        if(!busDriver.isIdle())
            break;
        busDriver.sendByte(lastSentByte);
        kbdState=KeyboardState::WaitingForCommands;
        break;
    case KeyboardState::SettingLEDs:
        setLEDs(setLEDsCmdArg);
        kbdState=KeyboardState::WaitingForCommands;
        break;
    }
}

void passByteToPS2(uint8_t data)
{
    USBH_UsrLog("pass byte to PS/2: %02X", (unsigned)data);
    // We get scan codes in the format {byteCount, byte1, byte2, ..., byteN}

    if(numBytesToReceiveInCurrentScanCode==0)
    {
        // New scan code begins, save length
        numBytesToReceiveInCurrentScanCode=data;
        if(kbdEnabled && !kbdBusy)
            beginningOfCurrentScanCodeWasSkipped=false;
    }
    else
    {
        --numBytesToReceiveInCurrentScanCode;
    }

    if(1+numBytesToReceiveInCurrentScanCode+keyboardBuffer.size()>keyboardBuffer.capacity())
    {
        // Avoid overflowing the buffer, since in this case we'll lose sync between scan code bytes and lengths
        beginningOfCurrentScanCodeWasSkipped=true;
        return;
    }

    // Only now can we check this and quit if needed, because the bookkeeping above is always required, or
    // buffer will become inconsistent due to unnoticed skipping of length byte.
    if(!kbdEnabled || kbdBusy)
    {
        if(numBytesToReceiveInCurrentScanCode)
            beginningOfCurrentScanCodeWasSkipped=true;
        return;
    }

    // Don't try to store partial data: at the very least scan code should be preceeded by its length.
    if(beginningOfCurrentScanCodeWasSkipped)
        return;

    keyboardBuffer.push_back(data);
}

static void initPS2ClockTimer()
{
    __TIM3_CLK_ENABLE();
    HAL_NVIC_EnableIRQ(TIM3_IRQn);

    TIM_HandleTypeDef tim={};
    tim.Instance=TIM3;
    tim.Init.Period=SystemCoreClock/QUADRUPLE_CLK_RATE-1;
    tim.Init.Prescaler=0;
    tim.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;
    tim.Init.CounterMode=TIM_COUNTERMODE_UP;
    tim.Init.AutoReloadPreload=TIM_AUTORELOAD_PRELOAD_DISABLE;
    if(HAL_TIM_Base_Init(&tim) != HAL_OK)
    {
        USBH_UsrLog("Failed to init PS/2 clock timer");
        abort();
    }
    else if(HAL_TIM_Base_Start_IT(&tim) != HAL_OK)
    {
        USBH_UsrLog("Failed to start PS/2 clock timer");
        abort();
    }
    else
    {
        USBH_UsrLog("PS/2 clock timer started");
    }
}

void PS2_Init()
{
    busDriver.init();
    initPS2ClockTimer();
}
