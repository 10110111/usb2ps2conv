#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

const uint8_t* keyToMakeCode(unsigned key, bool ctrl, bool shift, bool alt, bool numLockLED, bool autorepeat);
const uint8_t* keyToBreakCode(unsigned key, bool ctrl, bool shift, bool alt, bool numLockLED);

#ifdef __cplusplus
}
#endif
