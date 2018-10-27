cmake_minimum_required(VERSION 3.9)

set(AVR_UPLOADTOOL avrdude)
set(AVR_PROGRAMMER usbtiny)
set(AVR_UPLOADTOOL_PORT usb)
set(AVR_UPLOADTOOL_OPTIONS -C ${CMAKE_CURRENT_SOURCE_DIR}/avrdude.conf)

set(AVR_MCU attiny167)
set(AVR_E_FUSE 0xfe)
set(AVR_H_FUSE 0xdd)
set(AVR_L_FUSE 0xdf)

project(Keyboard C CXX ASM)

include(gcc-avr-util.cmake)

##########################################################################
# needs to be defined for AVR toolchain
##########################################################################
set(MCU_SPEED "16000000UL")

##########################################################################
# some cmake cross-compile necessities
##########################################################################
if(DEFINED ENV{AVR_FIND_ROOT_PATH})
    set(CMAKE_FIND_ROOT_PATH $ENV{AVR_FIND_ROOT_PATH})
else(DEFINED ENV{AVR_FIND_ROOT_PATH})
    if(EXISTS "/opt/local/avr")
      set(CMAKE_FIND_ROOT_PATH "/opt/local/avr")
    elseif(EXISTS "/usr/avr")
      set(CMAKE_FIND_ROOT_PATH "/usr/avr")
    elseif(EXISTS "/usr/lib/avr")
      set(CMAKE_FIND_ROOT_PATH "/usr/lib/avr")
    elseif(EXISTS "/usr/local/CrossPack-AVR")
      set(CMAKE_FIND_ROOT_PATH "/usr/local/CrossPack-AVR")
    else(EXISTS "/opt/local/avr")
      message(FATAL_ERROR "Please set AVR_FIND_ROOT_PATH in your environment.")
    endif(EXISTS "/opt/local/avr")
endif(DEFINED ENV{AVR_FIND_ROOT_PATH})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
# not added automatically, since CMAKE_SYSTEM_NAME is "generic"
set(CMAKE_SYSTEM_INCLUDE_PATH "${CMAKE_FIND_ROOT_PATH}/include")
set(CMAKE_SYSTEM_LIBRARY_PATH "${CMAKE_FIND_ROOT_PATH}/lib")

##########################################################################
# set compiler options for build types
##########################################################################
if(CMAKE_BUILD_TYPE MATCHES Release)
   set(CMAKE_C_FLAGS_RELEASE "-Os")
   set(CMAKE_CXX_FLAGS_RELEASE "-Os")
endif(CMAKE_BUILD_TYPE MATCHES Release)

if(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
   set(CMAKE_C_FLAGS_RELWITHDEBINFO "-Os -save-temps -g -gdwarf-3 -gstrict-dwarf")
   set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Os -save-temps -g -gdwarf-3 -gstrict-dwarf")
endif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)

if(CMAKE_BUILD_TYPE MATCHES Debug)
   set(CMAKE_C_FLAGS_DEBUG "-O0 -save-temps -g -gdwarf-3 -gstrict-dwarf")
   set(CMAKE_CXX_FLAGS_DEBUG "-O0 -save-temps -g -gdwarf-3 -gstrict-dwarf")
endif(CMAKE_BUILD_TYPE MATCHES Debug)

##########################################################################
# compiler options for all build types
##########################################################################
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -pedantic -pedantic-errors")

set(USB_CFG_IOPORTNAME "B")
set(USB_CFG_DMINUS_BIT "3")
set(USB_CFG_DPLUS_BIT "6")
set(USB_CFG_HAVE_INTRIN_ENDPOINT ON)
set(USB_CFG_INTR_POLL_INTERVAL 1)
set(USB_CFG_MAX_BUS_POWER "200")
set(USB_CFG_DEVICE_NAME "Keyboard emu")
set(USB_CFG_DEVICE_CLASS 0)
set(USB_CFG_INTERFACE_CLASS 3)
set(USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH 119)
set(USB_CFG_IMPLEMENT_FN_WRITE ON)

add_subdirectory(lib/v-usb/usbdrv)

add_avr_executable(keyboard
    keyboard.cpp
    stack.c

    usb/descriptor_kbd.cpp
    usb/report.h
    usb/descriptor.h
    usb/descriptor_kbd.h

    keyboard/keymap.cpp
    keyboard/keymap.h
    keyboard/Keyboard.h
    keyboard/Keyboard.cpp
    keyboard/uart.h
    keyboard/uart.cpp
    )

target_link_libraries(keyboard PRIVATE vusb)
target_include_directories(keyboard PRIVATE "${CMAKE_CURRENT_LIST_DIR}")
set_target_properties(keyboard PROPERTIES CXX_STANDARD 14)
