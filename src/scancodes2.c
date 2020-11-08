#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <usbh_hid_keybd.h>

#define KEY_MAX KEY_RIGHT_GUI+1
#define MAX_MAKE_CODE_LENGTH 2
#define MAX_BREAK_CODE_LENGTH 3
struct ScanCode
{
    uint8_t make[MAX_MAKE_CODE_LENGTH+1];
    uint8_t brek[MAX_BREAK_CODE_LENGTH+1];
};
const struct ScanCode scanCodes[KEY_MAX] =
{
    [KEY_ESCAPE]                            = {{1,0x76},              {2,0xF0,0x76}},
    [KEY_1_EXCLAMATION_MARK]                = {{1,0x16},              {2,0xF0,0x16}},
    [KEY_2_AT]                              = {{1,0x1E},              {2,0xF0,0x1E}},
    [KEY_3_NUMBER_SIGN]                     = {{1,0x26},              {2,0xF0,0x26}},
    [KEY_4_DOLLAR]                          = {{1,0x25},              {2,0xF0,0x25}},
    [KEY_5_PERCENT]                         = {{1,0x2E},              {2,0xF0,0x2E}},
    [KEY_6_CARET]                           = {{1,0x36},              {2,0xF0,0x36}},
    [KEY_7_AMPERSAND]                       = {{1,0x3D},              {2,0xF0,0x3D}},
    [KEY_8_ASTERISK]                        = {{1,0x3E},              {2,0xF0,0x3E}},
    [KEY_9_OPARENTHESIS]                    = {{1,0x46},              {2,0xF0,0x46}},
    [KEY_0_CPARENTHESIS]                    = {{1,0x45},              {2,0xF0,0x45}},
    [KEY_MINUS_UNDERSCORE]                  = {{1,0x4E},              {2,0xF0,0x4E}},
    [KEY_EQUAL_PLUS]                        = {{1,0x55},              {2,0xF0,0x55}},
    [KEY_BACKSPACE]                         = {{1,0x66},              {2,0xF0,0x66}},
    [KEY_TAB]                               = {{1,0x0D},              {2,0xF0,0x0D}},
    [KEY_Q]                                 = {{1,0x15},              {2,0xF0,0x15}},
    [KEY_W]                                 = {{1,0x1D},              {2,0xF0,0x1D}},
    [KEY_E]                                 = {{1,0x24},              {2,0xF0,0x24}},
    [KEY_R]                                 = {{1,0x2D},              {2,0xF0,0x2D}},
    [KEY_T]                                 = {{1,0x2C},              {2,0xF0,0x2C}},
    [KEY_Y]                                 = {{1,0x35},              {2,0xF0,0x35}},
    [KEY_U]                                 = {{1,0x3C},              {2,0xF0,0x3C}},
    [KEY_I]                                 = {{1,0x43},              {2,0xF0,0x43}},
    [KEY_O]                                 = {{1,0x44},              {2,0xF0,0x44}},
    [KEY_P]                                 = {{1,0x4D},              {2,0xF0,0x4D}},
    [KEY_OBRACKET_AND_OBRACE]               = {{1,0x54},              {2,0xF0,0x54}},
    [KEY_CBRACKET_AND_CBRACE]               = {{1,0x5B},              {2,0xF0,0x5B}},
    [KEY_ENTER]                             = {{1,0x5A},              {2,0xF0,0x5A}},
    [KEY_LEFTCONTROL]                       = {{1,0x14},              {2,0xF0,0x14}},
    [KEY_A]                                 = {{1,0x1C},              {2,0xF0,0x1C}},
    [KEY_S]                                 = {{1,0x1B},              {2,0xF0,0x1B}},
    [KEY_D]                                 = {{1,0x23},              {2,0xF0,0x23}},
    [KEY_F]                                 = {{1,0x2B},              {2,0xF0,0x2B}},
    [KEY_G]                                 = {{1,0x34},              {2,0xF0,0x34}},
    [KEY_H]                                 = {{1,0x33},              {2,0xF0,0x33}},
    [KEY_J]                                 = {{1,0x3B},              {2,0xF0,0x3B}},
    [KEY_K]                                 = {{1,0x42},              {2,0xF0,0x42}},
    [KEY_L]                                 = {{1,0x4B},              {2,0xF0,0x4B}},
    [KEY_SEMICOLON_COLON]                   = {{1,0x4C},              {2,0xF0,0x4C}},
    [KEY_SINGLE_AND_DOUBLE_QUOTE]           = {{1,0x52},              {2,0xF0,0x52}},
    [KEY_GRAVE_ACCENT_AND_TILDE]            = {{1,0x0E},              {2,0xF0,0x0E}},
    [KEY_LEFTSHIFT]                         = {{1,0x12},              {2,0xF0,0x12}},
    [KEY_BACKSLASH_VERTICAL_BAR]            = {{1,0x5D},              {2,0xF0,0x5D}},
// Don't be fooled by the name: it's actually the key mapped on US keyboards to "\|"
    [KEY_NONUS_NUMBER_SIGN_TILDE]           = {{1,0x5D},              {2,0xF0,0x5D}},
    [KEY_Z]                                 = {{1,0x1A},              {2,0xF0,0x1A}},
    [KEY_X]                                 = {{1,0x22},              {2,0xF0,0x22}},
    [KEY_C]                                 = {{1,0x21},              {2,0xF0,0x21}},
    [KEY_V]                                 = {{1,0x2A},              {2,0xF0,0x2A}},
    [KEY_B]                                 = {{1,0x32},              {2,0xF0,0x32}},
    [KEY_N]                                 = {{1,0x31},              {2,0xF0,0x31}},
    [KEY_M]                                 = {{1,0x3A},              {2,0xF0,0x3A}},
    [KEY_COMMA_AND_LESS]                    = {{1,0x41},              {2,0xF0,0x41}},
    [KEY_DOT_GREATER]                       = {{1,0x49},              {2,0xF0,0x49}},
    [KEY_SLASH_QUESTION]                    = {{1,0x4A},              {2,0xF0,0x4A}},
    [KEY_RIGHTSHIFT]                        = {{1,0x59},              {2,0xF0,0x59}},
    [KEY_KEYPAD_ASTERISK]                   = {{1,0x7C},              {2,0xF0,0x7C}},
    [KEY_LEFTALT]                           = {{1,0x11},              {2,0xF0,0x11}},
    [KEY_SPACEBAR]                          = {{1,0x29},              {2,0xF0,0x29}},
    [KEY_CAPS_LOCK]                         = {{1,0x58},              {2,0xF0,0x58}},
    [KEY_F1]                                = {{1,0x05},              {2,0xF0,0x05}},
    [KEY_F2]                                = {{1,0x06},              {2,0xF0,0x06}},
    [KEY_F3]                                = {{1,0x04},              {2,0xF0,0x04}},
    [KEY_F4]                                = {{1,0x0C},              {2,0xF0,0x0C}},
    [KEY_F5]                                = {{1,0x03},              {2,0xF0,0x03}},
    [KEY_F6]                                = {{1,0x0B},              {2,0xF0,0x0B}},
    [KEY_F7]                                = {{1,0x83},              {2,0xF0,0x83}},
    [KEY_F8]                                = {{1,0x0A},              {2,0xF0,0x0A}},
    [KEY_F9]                                = {{1,0x01},              {2,0xF0,0x01}},
    [KEY_F10]                               = {{1,0x09},              {2,0xF0,0x09}},
    [KEY_KEYPAD_NUM_LOCK_AND_CLEAR]         = {{1,0x77},              {2,0xF0,0x77}},
    [KEY_SCROLL_LOCK]                       = {{1,0x7E},              {2,0xF0,0x7E}},
    [KEY_KEYPAD_7_HOME]                     = {{1,0x6C},              {2,0xF0,0x6C}},
    [KEY_KEYPAD_8_UP_ARROW]                 = {{1,0x75},              {2,0xF0,0x75}},
    [KEY_KEYPAD_9_PAGEUP]                   = {{1,0x7D},              {2,0xF0,0x7D}},
    [KEY_KEYPAD_MINUS]                      = {{1,0x7B},              {2,0xF0,0x7B}},
    [KEY_KEYPAD_4_LEFT_ARROW]               = {{1,0x6B},              {2,0xF0,0x6B}},
    [KEY_KEYPAD_5]                          = {{1,0x73},              {2,0xF0,0x73}},
    [KEY_KEYPAD_6_RIGHT_ARROW]              = {{1,0x74},              {2,0xF0,0x74}},
    [KEY_KEYPAD_PLUS]                       = {{1,0x79},              {2,0xF0,0x79}},
    [KEY_KEYPAD_1_END]                      = {{1,0x69},              {2,0xF0,0x69}},
    [KEY_KEYPAD_2_DOWN_ARROW]               = {{1,0x72},              {2,0xF0,0x72}},
    [KEY_KEYPAD_3_PAGEDN]                   = {{1,0x7A},              {2,0xF0,0x7A}},
    [KEY_KEYPAD_0_INSERT]                   = {{1,0x70},              {2,0xF0,0x70}},
    [KEY_KEYPAD_DECIMAL_SEPARATOR_DELETE]   = {{1,0x71},              {2,0xF0,0x71}},
    [KEY_F11]                               = {{1,0x78},              {2,0xF0,0x78}},
    [KEY_F12]                               = {{1,0x07},              {2,0xF0,0x07}},
    [KEY_KEYPAD_ENTER]                      = {{2,0xE0,0x5A},         {3,0xE0,0xF0,0x5A}},
    [KEY_RIGHTCONTROL]                      = {{2,0xE0,0x14},         {3,0xE0,0xF0,0x14}},

    // KP(/) has Shift-dependent scan code
    [KEY_KEYPAD_SLASH]                      = {{},                    {}},

    // Special handling for PrtScr/SysRq:
    //   Shift,Ctrl,Alt released: E0 12 E0 7C;
    //   Alt held, regardless of Shift/Ctrl state: 84;
    //   Shift/Ctrl held, Alt released: E0 7C;
    [KEY_SYSREQ]                            = {{},                    {}},
    [KEY_PRINTSCREEN]                       = {{},                    {}},

    [KEY_RIGHTALT]                          = {{2,0xE0,0x11},         {3,0xE0,0xF0,0x11}},

    // Block with Shift+NumLock-dependent scan code
    [KEY_HOME]                              = {{},                    {}},
    [KEY_UPARROW]                           = {{},                    {}},
    [KEY_PAGEUP]                            = {{},                    {}},
    [KEY_LEFTARROW]                         = {{},                    {}},
    [KEY_RIGHTARROW]                        = {{},                    {}},
    [KEY_END]                               = {{},                    {}},
    [KEY_DOWNARROW]                         = {{},                    {}},
    [KEY_PAGEDOWN]                          = {{},                    {}},
    [KEY_INSERT]                            = {{},                    {}},
    [KEY_DELETE]                            = {{},                    {}},
    // End block

    [KEY_MUTE]                              = {{2,0xE0,0x23},         {3,0xE0,0xF0,0x23}},
    [KEY_VOLUME_DOWN]                       = {{2,0xE0,0x21},         {3,0xE0,0xF0,0x21}},
    [KEY_VOLUME_UP]                         = {{2,0xE0,0x32},         {3,0xE0,0xF0,0x32}},
    [KEY_POWER]                             = {{0,},                  {0,}},
    [KEY_KEYPAD_EQUAL]                      = {{0,},                  {0,}},
    [KEY_KEYPAD_PLUSMINUS]                  = {{0,},                  {0,}},

    // Pause/Break has Ctrl-dependent scan code
    [KEY_PAUSE]                             = {{},                    {}},

    [KEY_KEYPAD_COMMA]                      = {{0,},                  {0,}},
    [KEY_LEFT_GUI]                          = {{2,0xE0,0x1F},         {3,0xE0,0xF0,0x1F}}, // WinLogo
    [KEY_RIGHT_GUI]                         = {{2,0xE0,0x27},         {3,0xE0,0xF0,0x27}}, // WinLogo
    [KEY_APPLICATION]                       = {{2,0xE0,0x2F},         {3,0xE0,0xF0,0x2F}}, // App Menu
    [KEY_STOP]                              = {{0,},                  {0,}},
    [KEY_AGAIN]                             = {{0,},                  {0,}},
    [KEY_UNDO]                              = {{0,},                  {0,}},
    [KEY_COPY]                              = {{0,},                  {0,}},
    [KEY_PASTE]                             = {{0,},                  {0,}},
    [KEY_FIND]                              = {{0,},                  {0,}},
    [KEY_CUT]                               = {{0,},                  {0,}},
    [KEY_HELP]                              = {{0,},                  {0,}},
// FIXME: what HID usages correspond to these commented out keys?
//    [KEY_CALC]                              = {{2,0xE0,0x2B},         {3,0xE0,0xF0,0x2B}},
//    [KEY_SLEEP]                             = {{2,0xE0,0x3F},         {3,0xE0,0xF0,0x3F}},
//    [KEY_WAKEUP]                            = {{2,0xE0,0x5E},         {3,0xE0,0xF0,0x5E}},
//    [KEY_MAIL]                              = {{2,0xE0,0x48},         {3,0xE0,0xF0,0x48}},
//    [KEY_BOOKMARKS]                         = {{2,0xE0,0x18},         {3,0xE0,0xF0,0x18}},
//    [KEY_COMPUTER]                          = {{2,0xE0,0x40},         {3,0xE0,0xF0,0x40}},
//    [KEY_BACK]                              = {{2,0xE0,0x38},         {3,0xE0,0xF0,0x38}},
//    [KEY_FORWARD]                           = {{2,0xE0,0x30},         {3,0xE0,0xF0,0x30}},
//    [KEY_NEXTSONG]                          = {{2,0xE0,0x4D},         {3,0xE0,0xF0,0x4D}},
//    [KEY_PLAYPAUSE]                         = {{2,0xE0,0x34},         {3,0xE0,0xF0,0x34}},
//    [KEY_PREVIOUSSONG]                      = {{2,0xE0,0x15},         {3,0xE0,0xF0,0x15}},
//    [KEY_STOPCD]                            = {{2,0xE0,0x3B},         {3,0xE0,0xF0,0x3B}},
//    [KEY_HOMEPAGE]                          = {{2,0xE0,0x3A},         {3,0xE0,0xF0,0x3A}},
//    [KEY_REFRESH]                           = {{2,0xE0,0x20},         {3,0xE0,0xF0,0x20}},
    [KEY_F13]                               = {{0,},                  {0,}},
    [KEY_F14]                               = {{0,},                  {0,}},
    [KEY_F15]                               = {{0,},                  {0,}},
    [KEY_F16]                               = {{0,},                  {0,}},
    [KEY_F17]                               = {{0,},                  {0,}},
    [KEY_F18]                               = {{0,},                  {0,}},
    [KEY_F19]                               = {{0,},                  {0,}},
    [KEY_F20]                               = {{0,},                  {0,}},
    [KEY_F21]                               = {{0,},                  {0,}},
    [KEY_F22]                               = {{0,},                  {0,}},
    [KEY_F23]                               = {{0,},                  {0,}},
    [KEY_F24]                               = {{0,},                  {0,}},
// FIXME: what HID usages correspond to these commented out keys?
//    [KEY_SEARCH]                            = {{2,0xE0,0x10},         {3,0xE0,0xF0,0x10}},
    [KEY_CANCEL]                            = {{0,},                  {0,}},
//    [KEY_MEDIA]                             = {{2,0xE0,0x50},         {3,0xE0,0xF0,0x50}},
    [KEY_SELECT]                            = {{0,},                  {0,}},
    [KEY_CLEAR]                             = {{0,},                  {0,}},

    [KEY_NONUS_BACK_SLASH_VERTICAL_BAR]     = {{1,0x61},              {2,0xF0,0x61}},
};

const uint8_t* keyToMakeCode(unsigned key, bool ctrl, bool shift, bool alt, bool numLockLED, bool autorepeat)
{
    if(key >= sizeof scanCodes/sizeof*scanCodes) return NULL;

#define SHIFT_NUMLOCK_DEPENDENT_MAKE_CODE(key,scancode)                     \
    case key:                                                               \
        if(shift && !numLockLED && !autorepeat)                             \
        {                                                                   \
            static const uint8_t code[]={5, 0xE0,0xF0,0x12,0xE0,scancode};  \
            return code;                                                    \
        }                                                                   \
        else if(!shift && numLockLED && !autorepeat)                        \
        {                                                                   \
            static const uint8_t code[]={4, 0xE0,0x12,0xE0,scancode};       \
            return code;                                                    \
        }                                                                   \
        else                                                                \
        {                                                                   \
            static const uint8_t code[]={2, 0xE0,scancode};                 \
            return code;                                                    \
        }

    switch(key)
    {
    case KEY_PRINTSCREEN:
    case KEY_SYSREQ:
        if(alt)
        {
            static const uint8_t code[]={1, 0x84};
            return code;
        }
        else if(ctrl||shift)
        {
            static const uint8_t code[]={2, 0xE0,0x7C};
            return code;
        }
        else
        {
            static const uint8_t code[]={4, 0xE0,0x12,0xE0,0x7C};
            return code;
        }
    SHIFT_NUMLOCK_DEPENDENT_MAKE_CODE(KEY_HOME,0x6C)
    SHIFT_NUMLOCK_DEPENDENT_MAKE_CODE(KEY_UPARROW,0x75)
    SHIFT_NUMLOCK_DEPENDENT_MAKE_CODE(KEY_PAGEUP,0x7D)
    SHIFT_NUMLOCK_DEPENDENT_MAKE_CODE(KEY_LEFTARROW,0x6B)
    SHIFT_NUMLOCK_DEPENDENT_MAKE_CODE(KEY_RIGHTARROW,0x74)
    SHIFT_NUMLOCK_DEPENDENT_MAKE_CODE(KEY_END,0x69)
    SHIFT_NUMLOCK_DEPENDENT_MAKE_CODE(KEY_DOWNARROW,0x72)
    SHIFT_NUMLOCK_DEPENDENT_MAKE_CODE(KEY_PAGEDOWN,0x7A)
    SHIFT_NUMLOCK_DEPENDENT_MAKE_CODE(KEY_INSERT,0x70)
    SHIFT_NUMLOCK_DEPENDENT_MAKE_CODE(KEY_DELETE,0x71)
    SHIFT_NUMLOCK_DEPENDENT_MAKE_CODE(KEY_KEYPAD_SLASH,0x4A)
    case KEY_PAUSE:
        if(ctrl)
        {
            static const uint8_t code[]={5, 0xE0,0x7E,0xE0,0xF0,0x7E};
            return code;
        }
        else
        {
            static const uint8_t code[]={8, 0xE1,0x14,0x77,0xE1,0xF0,0x14,0xF0,0x77};
            return code;
        }
    }
    return scanCodes[key].make;
}
const uint8_t* keyToBreakCode(unsigned key, bool ctrl, bool shift, bool alt, bool numLockLED)
{
    if(key >= sizeof scanCodes/sizeof*scanCodes) return NULL;

#define SHIFT_NUMLOCK_DEPENDENT_BREAK_CODE(key,scancode)                        \
    case key:                                                                   \
        if(shift && !numLockLED)                                                \
        {                                                                       \
            static const uint8_t code[]={5, 0xE0,0xF0,scancode,0xE0,0x12};      \
            return code;                                                        \
        }                                                                       \
        else if(!shift && numLockLED)                                           \
        {                                                                       \
            static const uint8_t code[]={6, 0xE0,0xF0,scancode,0xE0,0xF0,0x12}; \
            return code;                                                        \
        }                                                                       \
        else                                                                    \
        {                                                                       \
            static const uint8_t code[]={3, 0xE0,0xF0,scancode};                \
            return code;                                                        \
        }

    switch(key)
    {
    case KEY_PRINTSCREEN:
    case KEY_SYSREQ:
        if(alt)
        {
            static const uint8_t code[]={2, 0xF0,0x84};
            return code;
        }
        else if(ctrl||shift)
        {
            static const uint8_t code[]={3, 0xE0,0xF0,0x7C};
            return code;
        }
        else
        {
            static const uint8_t code[]={6, 0xE0,0xF0,0x7C,0xE0,0xF0,0x12};
            return code;
        }
    SHIFT_NUMLOCK_DEPENDENT_BREAK_CODE(KEY_HOME,0x6C)
    SHIFT_NUMLOCK_DEPENDENT_BREAK_CODE(KEY_UPARROW  ,0x75)
    SHIFT_NUMLOCK_DEPENDENT_BREAK_CODE(KEY_PAGEUP,0x7D)
    SHIFT_NUMLOCK_DEPENDENT_BREAK_CODE(KEY_LEFTARROW,0x6B)
    SHIFT_NUMLOCK_DEPENDENT_BREAK_CODE(KEY_RIGHTARROW,0x74)
    SHIFT_NUMLOCK_DEPENDENT_BREAK_CODE(KEY_END,0x69)
    SHIFT_NUMLOCK_DEPENDENT_BREAK_CODE(KEY_DOWNARROW,0x72)
    SHIFT_NUMLOCK_DEPENDENT_BREAK_CODE(KEY_PAGEDOWN,0x7A)
    SHIFT_NUMLOCK_DEPENDENT_BREAK_CODE(KEY_INSERT,0x70)
    SHIFT_NUMLOCK_DEPENDENT_BREAK_CODE(KEY_DELETE,0x71)
    SHIFT_NUMLOCK_DEPENDENT_BREAK_CODE(KEY_KEYPAD_SLASH,0x4A)
    case KEY_PAUSE:
        return (const uint8_t*)"";
    }
    return scanCodes[key].brek;
}
