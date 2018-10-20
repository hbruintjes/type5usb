##########################################################################
# "THE ANY BEVERAGE-WARE LICENSE" (Revision 42 - based on beer-ware
# license):
# <dev@layer128.net> wrote this file. As long as you retain this notice
# you can do whatever you want with this stuff. If we meet some day, and
# you think this stuff is worth it, you can buy me a be(ve)er(age) in
# return. (I don't like beer much.)
#
# Matthias Kleemann
##########################################################################

##########################################################################
# The toolchain requires some variables set.
#
# AVR_MCU (default: atmega8)
#     the type of AVR the application is built for
# AVR_L_FUSE (NO DEFAULT)
#     the LOW fuse value for the MCU used
# AVR_H_FUSE (NO DEFAULT)
#     the HIGH fuse value for the MCU used
# AVR_UPLOADTOOL (default: avrdude)
#     the application used to upload to the MCU
#     NOTE: The toolchain is currently quite specific about
#           the commands used, so it needs tweaking.
# AVR_UPLOADTOOL_PORT (default: usb)
#     the port used for the upload tool, e.g. usb
# AVR_PROGRAMMER (default: avrispmkII)
#     the programmer hardware used, e.g. avrispmkII
##########################################################################

##########################################################################
# options
##########################################################################
option(WITH_MCU "Add the mCU type to the target file name." ON)

##########################################################################
# executables in use
##########################################################################
find_program(AVR_CC avr-gcc)
find_program(AVR_CXX avr-g++)
find_program(AVR_OBJCOPY avr-objcopy)
find_program(AVR_SIZE_TOOL avr-size)
find_program(AVR_OBJDUMP avr-objdump)

##########################################################################
# toolchain starts with defining mandatory variables
##########################################################################
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)
set(CMAKE_C_COMPILER ${AVR_CC})
set(CMAKE_CXX_COMPILER ${AVR_CXX})

##########################################################################
# Identification
##########################################################################
set(AVR 1)

##########################################################################
# some necessary tools and variables for AVR builds, which may not
# defined yet
# - AVR_UPLOADTOOL
# - AVR_UPLOADTOOL_PORT
# - AVR_PROGRAMMER
# - AVR_MCU
# - AVR_SIZE_ARGS
##########################################################################

# default upload tool
if(NOT AVR_UPLOADTOOL)
   set(
      AVR_UPLOADTOOL avrdude
      CACHE STRING "Set default upload tool: avrdude"
   )
   find_program(AVR_UPLOADTOOL avrdude)
endif(NOT AVR_UPLOADTOOL)

# default upload tool port
if(NOT AVR_UPLOADTOOL_PORT)
   set(
      AVR_UPLOADTOOL_PORT usb
      CACHE STRING "Set default upload tool port: usb"
   )
endif(NOT AVR_UPLOADTOOL_PORT)

# default programmer (hardware)
if(NOT AVR_PROGRAMMER)
   set(
      AVR_PROGRAMMER avrispmkII
      CACHE STRING "Set default programmer hardware model: avrispmkII"
   )
endif(NOT AVR_PROGRAMMER)

# default MCU (chip)
if(NOT AVR_MCU)
   set(
      AVR_MCU atmega8
      CACHE STRING "Set default MCU: atmega8 (see 'avr-gcc --target-help' for valid values)"
   )
endif(NOT AVR_MCU)

#default avr-size args
if(NOT AVR_SIZE_ARGS)
   if(APPLE)
      set(AVR_SIZE_ARGS -B)
   else(APPLE)
      set(AVR_SIZE_ARGS -C;--mcu=${AVR_MCU})
   endif(APPLE)
endif(NOT AVR_SIZE_ARGS)

##########################################################################
# check build types:
# - Debug
# - Release
# - RelWithDebInfo
#
# Release is chosen, because of some optimized functions in the
# AVR toolchain, e.g. _delay_ms().
##########################################################################
if(NOT ((CMAKE_BUILD_TYPE MATCHES Release) OR
        (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo) OR
        (CMAKE_BUILD_TYPE MATCHES Debug) OR
        (CMAKE_BUILD_TYPE MATCHES MinSizeRel)))
   set(
      CMAKE_BUILD_TYPE Release
      CACHE STRING "Choose cmake build type: Debug Release RelWithDebInfo MinSizeRel"
      FORCE
   )
endif(NOT ((CMAKE_BUILD_TYPE MATCHES Release) OR
           (CMAKE_BUILD_TYPE MATCHES RelWithDebInfo) OR
           (CMAKE_BUILD_TYPE MATCHES Debug) OR
           (CMAKE_BUILD_TYPE MATCHES MinSizeRel)))

##########################################################################

##########################################################################
# target file name add-on
##########################################################################
if(WITH_MCU)
   set(MCU_TYPE_FOR_FILENAME "-${AVR_MCU}")
else(WITH_MCU)
   set(MCU_TYPE_FOR_FILENAME "")
endif(WITH_MCU)

##########################################################################
# add_avr_executable
# - IN_VAR: EXECUTABLE_NAME
#
# Creates targets and dependencies for AVR toolchain, building an
# executable.
##########################################################################
function(add_avr_executable EXECUTABLE_NAME)
   add_executable(${EXECUTABLE_NAME} ${ARGN})

   # set file names
   set(OUTPUT_NAME ${EXECUTABLE_NAME}${MCU_TYPE_FOR_FILENAME})
   set_target_properties(${EXECUTABLE_NAME} PROPERTIES OUTPUT_NAME ${EXECUTABLE_NAME}${MCU_TYPE_FOR_FILENAME})
   set(hex_file "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/${OUTPUT_NAME}.hex")
   set(map_file "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/${OUTPUT_NAME}.map")
   set(eeprom_image "$<TARGET_FILE_DIR:${EXECUTABLE_NAME}>/${OUTPUT_NAME}-eeprom.hex")

   target_compile_options(${EXECUTABLE_NAME} PRIVATE "-mmcu=${AVR_MCU}")
   target_compile_definitions(${EXECUTABLE_NAME} PRIVATE F_CPU=${MCU_SPEED})
   target_link_libraries(${EXECUTABLE_NAME} PRIVATE "-mmcu=${AVR_MCU}" -Wl,--gc-sections -mrelax "-Wl,-Map,${OUTPUT_NAME}.map")

   add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
      COMMAND ${AVR_OBJCOPY} -j .text -j .data -O ihex
        $<TARGET_FILE:${EXECUTABLE_NAME}> "${hex_file}"
      COMMAND ${AVR_SIZE_TOOL} ${AVR_SIZE_ARGS} $<TARGET_FILE:${EXECUTABLE_NAME}>
   )

   # eeprom
   add_custom_command(TARGET ${EXECUTABLE_NAME} POST_BUILD
      COMMAND ${AVR_OBJCOPY} -j .eeprom --set-section-flags=.eeprom=alloc,load
        --change-section-lma .eeprom=0 --no-change-warnings -O ihex
        $<TARGET_FILE:${EXECUTABLE_NAME}> "${eeprom_image}"
   )

   # clean
   set_property(DIRECTORY "" APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES "${map_file}")

   # upload - with avrdude
   add_custom_target(
      upload_${EXECUTABLE_NAME}
      ${AVR_UPLOADTOOL} ${AVR_UPLOADTOOL_OPTIONS} -p ${AVR_MCU} -c ${AVR_PROGRAMMER}
         -U "flash:w:${hex_file}"
         -P ${AVR_UPLOADTOOL_PORT}
      DEPENDS ${EXECUTABLE_NAME}
      COMMENT "Uploading ${hex_file} to ${AVR_MCU} using ${AVR_PROGRAMMER}"
   )

   # upload eeprom only - with avrdude
   # see also bug http://savannah.nongnu.org/bugs/?40142
   add_custom_target(
      upload_eeprom_${EXECUTABLE_NAME}
      ${AVR_UPLOADTOOL} ${AVR_UPLOADTOOL_OPTIONS} -p ${AVR_MCU} -c ${AVR_PROGRAMMER}
         -U "eeprom:w:${eeprom_image}"
         -P ${AVR_UPLOADTOOL_PORT}
      DEPENDS ${EXECUTABLE_NAME}
      COMMENT "Uploading ${eeprom_image} to ${AVR_MCU} using ${AVR_PROGRAMMER}"
   )

   # disassemble
   add_custom_target(
      disassemble_${EXECUTABLE_NAME}
      ${AVR_OBJDUMP} -h -S ${elf_file} > ${EXECUTABLE_NAME}.lst
      DEPENDS ${elf_file}
   )

endfunction(add_avr_executable)

function(add_avr_library LIBRARY_NAME)
   add_library(${LIBRARY_NAME} ${ARGN})

   target_compile_options(${LIBRARY_NAME} PRIVATE "-mmcu=${AVR_MCU}")
   target_compile_definitions(${LIBRARY_NAME} PRIVATE F_CPU=${MCU_SPEED})
   target_link_libraries(${LIBRARY_NAME} PRIVATE "-mmcu=${AVR_MCU}")
endfunction(add_avr_library)
