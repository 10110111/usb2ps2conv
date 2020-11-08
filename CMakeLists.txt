cmake_minimum_required(VERSION 3.15.3)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/toolchain.cmake")

project(usb2ps2conv C CXX ASM)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "MinSizeRel" CACHE STRING "Build type (defaults to MinSizeRel)" FORCE)
endif()

option(ENABLE_LED "Enable status indication via the 4 LEDs on the Discovery board" ON)
if(ENABLE_LED)
    add_definitions(-DENABLE_LED)
endif()

set(sources
    src/led.c
    src/main.cpp
    src/config.c
    src/dbg-out.c
    src/syscalls.c
    src/hid-keybd.c
    src/usbh_conf.c
    src/scancodes2.c
    src/stm32f4xx_it.c
    src/system_stm32f4xx.c
    src/ps2-kbd-emulator.cpp
    src/startup_stm32f401xc.s
)

set(driverSources
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_hcd.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c
)

set(usbLibSources
    Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_core.c
    Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_ioreq.c
    Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_pipes.c
    Middlewares/ST/STM32_USB_Host_Library/Core/Src/usbh_ctlreq.c
    Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid.c
    Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_mouse.c
    Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_keybd.c
    Middlewares/ST/STM32_USB_Host_Library/Class/HID/Src/usbh_hid_parser.c
)

add_definitions(-DSTM32F401xC)
include_directories(
    src
    Drivers/STM32F4xx_HAL_Driver/Inc
    Drivers/CMSIS/Device/ST/STM32F4xx/Include
    Drivers/CMSIS/Include
    Middlewares/ST/STM32_USB_Host_Library/Core/Inc
    Middlewares/ST/STM32_USB_Host_Library/Class/HID/Inc
)

set(MCU -mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -mthumb-interwork)
add_compile_options(
    ${MCU}
    -fsingle-precision-constant
    -Wdouble-promotion

    -fno-strict-aliasing

    -fdata-sections
    -ffunction-sections

    -Wall
    -Werror=return-type -Werror=format
    $<$<COMPILE_LANGUAGE:C>:-Werror=implicit-function-declaration>
    $<$<COMPILE_LANGUAGE:C>:-Werror=incompatible-pointer-types>
)

add_link_options(
    ${MCU}
    -T${CMAKE_SOURCE_DIR}/STM32F401VCTx_FLASH.ld
    -nodefaultlibs
    -fno-exceptions
)

set(binary ${PROJECT_NAME}.elf)
add_executable(${binary} ${sources} ${driverSources} ${usbLibSources})

add_custom_target(${PROJECT_NAME}.bin ALL
    COMMAND ${CMAKE_OBJCOPY} -O binary ${binary} ${PROJECT_NAME}.bin
    DEPENDS ${binary})
add_custom_target(${PROJECT_NAME}.hex ALL
    COMMAND ${CMAKE_OBJCOPY} -O ihex ${binary} ${PROJECT_NAME}.hex
    DEPENDS ${binary})

# Print executable size
add_custom_command(TARGET ${binary} POST_BUILD
    COMMAND ${CMAKE_SIZE_UTIL} ${binary})

add_custom_target(flash DEPENDS ${PROJECT_NAME}.bin
    COMMAND ${CMAKE_FLASH_UTIL} write ${PROJECT_NAME}.bin 0x8000000)
