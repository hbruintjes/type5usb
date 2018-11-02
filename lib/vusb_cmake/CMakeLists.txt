set(USB_CFG_IOPORTNAME "B" CACHE STRING "This is the port where the USB bus is connected. When you configure it to \"B\", the registers PORTB, PINB and DDRB will be used.")
set(USB_CFG_DMINUS_BIT "((char)3)" CACHE STRING "This is the bit number in USB_CFG_IOPORT where the USB D- line is connected.")
set(USB_CFG_DPLUS_BIT "((char)6)" CACHE STRING "This is the bit number in USB_CFG_IOPORT where the USB D+ line is connected.")
option(USB_CFG_CHECK_CRC "Define this to 1 if you want that the driver checks integrity of incoming data packets (CRC checks)." OFF)
set(USB_CFG_PULLUP_IOPORTNAME "" CACHE STRING "This constant defines the port on which the pullup resistor is connected.")
option(USB_CFG_HAVE_INTRIN_ENDPOINT "Define this to 1 if you want to compile a version with two endpoints" OFF)
option(USB_CFG_HAVE_INTRIN_ENDPOINT3 "Define this to 1 if you want to compile a version with three endpoints" OFF)
set(USB_CFG_EP3_NUMBER "3" CACHE STRING "If the so-called endpoint 3 is used, it can now be configured to any other endpoint number (except 0) with this macro.")
set(USB_INITIAL_DATATOKEN "" CACHE STRING "The above macro defines the startup condition for data toggling on the interrupt/bulk endpoints 1 and 3.")
option(USB_CFG_IMPLEMENT_HALT "Define this to 1 if you also want to implement the ENDPOINT_HALT feature for endpoint 1 (interrupt endpoint)." OFF)
option(USB_CFG_SUPPRESS_INTR_CODE "Define this to 1 if you want to declare interrupt-in endpoints, but don't want to send any data over them." OFF)
set(USB_CFG_INTR_POLL_INTERVAL "10" CACHE STRING "If you compile a version with endpoint 1 (interrupt-in), this is the poll interval.")
option(USB_CFG_IS_SELF_POWERED "Define this to 1 if the device has its own power supply." OFF)
set(USB_CFG_MAX_BUS_POWER "100" CACHE STRING "Set this variable to the maximum USB bus power consumption of your device.")
option(USB_CFG_IMPLEMENT_FN_WRITE "Set this to 1 if you want usbFunctionWrite() to be called for control-out transfers." OFF)
option(USB_CFG_IMPLEMENT_FN_READ "Set this to 1 if you need to send control replies which are generated \"on the fly\" when usbFunctionRead() is called." OFF)
option(USB_CFG_IMPLEMENT_FN_WRITEOUT "Define this to 1 if you want to use interrupt-out (or bulk out) endpoints." OFF)
option(USB_CFG_HAVE_FLOWCONTROL "Define this to 1 if you want flowcontrol over USB data." OFF)
set(USB_CFG_DRIVER_FLASH_PAGE "0" CACHE STRING "If the device has more than 64 kBytes of flash, define this to the 64 k page where the driver's constants (descriptors) are located.")
option(USB_CFG_LONG_TRANSFERS "Define this to 1 if you want to send/receive blocks of more than 254 bytes in a single control-in or control-out transfer." OFF)
option(USB_COUNT_SOF "Define this macro to 1 if you need the global variable \"usbSofCount\" which counts SOF packets." OFF)
option(USB_CFG_CHECK_DATA_TOGGLING "Define this macro to 1 if you want to filter out duplicate data packets sent by the host." OFF)
option(USB_CFG_HAVE_MEASURE_FRAME_LENGTH "Define this macro to 1 if you want the function usbMeasureFrameLength() compiled in." OFF)
option(USB_USE_FAST_CRC "The assembler module has two implementations for the CRC algorithm. One is faster, the other is smaller." OFF)

set(USB_CFG_VENDOR_NAME "obdev.at" CACHE STRING "Defines the vendor name returned by the USB device.")
if (USB_CFG_VENDOR_NAME)
    string(LENGTH "${USB_CFG_VENDOR_NAME}" USB_CFG_VENDOR_NAME_LEN)
    string(REGEX REPLACE "(.)" "'\\1', " USB_CFG_VENDOR_NAME "${USB_CFG_VENDOR_NAME}")
    string(REGEX REPLACE ", $" "" USB_CFG_VENDOR_NAME "${USB_CFG_VENDOR_NAME}")
endif ()

set(USB_CFG_DEVICE_NAME "Template" CACHE STRING "Defines the device name returned by the USB device.")
if (USB_CFG_DEVICE_NAME)
    string(LENGTH "${USB_CFG_DEVICE_NAME}" USB_CFG_DEVICE_NAME_LEN)
    string(REGEX REPLACE "(.)" "'\\1', " USB_CFG_DEVICE_NAME "${USB_CFG_DEVICE_NAME}")
    string(REGEX REPLACE ", $" "" USB_CFG_DEVICE_NAME "${USB_CFG_DEVICE_NAME}")
endif ()

set(USB_CFG_SERIAL_NUMBER "" CACHE STRING "Defines the serial number returned by the USB device.")
if (USB_CFG_SERIAL_NUMBER)
    string(LENGTH "${USB_CFG_SERIAL_NUMBER}" USB_CFG_SERIAL_NUMBER_LEN)
    string(REGEX REPLACE "(.)" "'\\1', " USB_CFG_SERIAL_NUMBER "${USB_CFG_SERIAL_NUMBER}")
    string(REGEX REPLACE ", $" "" USB_CFG_SERIAL_NUMBER "${USB_CFG_SERIAL_NUMBER}")
endif ()

set(USB_CFG_VENDOR_ID "0xc0, 0x16" CACHE STRING "USB vendor ID for the device, low byte first.")
set(USB_CFG_DEVICE_ID "0xdc, 0x05" CACHE STRING "This is the ID of the product, low byte first.")
set(USB_CFG_DEVICE_VERSION "0x00, 0x01" CACHE STRING "Version number of the device: Minor number first, then major number.")

set(USB_CFG_DEVICE_CLASS "0xff" CACHE STRING "Set to 0 if deferred to interface")
set(USB_CFG_DEVICE_SUBCLASS "0" CACHE STRING "")

set(USB_CFG_INTERFACE_CLASS "0" CACHE STRING "Define this to the length of the HID report descriptor, if you implement an HID device.")
set(USB_CFG_INTERFACE_SUBCLASS "0" CACHE STRING "Define this to the length of the HID report descriptor, if you implement an HID device.")
set(USB_CFG_INTERFACE_PROTOCOL "0" CACHE STRING "Define this to the length of the HID report descriptor, if you implement an HID device.")

set(USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH "" CACHE STRING "Define this to the length of the HID report descriptor, if you implement an HID device.")

set(USB_CFG_MCU_DESC [[/* #define USB_INTR_CFG            MCUCR */
/* #define USB_INTR_CFG_SET        ((1 << ISC00) | (1 << ISC01)) */
/* #define USB_INTR_CFG_CLR        0 */
/* #define USB_INTR_ENABLE         GIMSK */
/* #define USB_INTR_ENABLE_BIT     INT0 */
/* #define USB_INTR_PENDING        GIFR */
/* #define USB_INTR_PENDING_BIT    INTF0 */
/* #define USB_INTR_VECTOR         INT0_vect */]] CACHE STRING "Optional MCU Description.")

configure_file(usbconfig.h.in ${CMAKE_CURRENT_BINARY_DIR}/usbconfig.h @ONLY)

add_avr_library(vusb STATIC
    # Sources
    usbdrv.c
    usbdrvasm.S
    oddebug.c

    # Headers
    usbdrv.h
    ${CMAKE_CURRENT_BINARY_DIR}/usbconfig.h
    oddebug.h
    asmcommon.inc
    usbportability.h

    # Support
    usbdrvasm12.inc
    usbdrvasm15.inc
    usbdrvasm16.inc
    usbdrvasm18.inc
    usbdrvasm20.inc
    usbdrvasm128.inc
    usbdrvasm165.inc
    )

target_include_directories(vusb PUBLIC
    "${CMAKE_CURRENT_LIST_DIR}"
    "${CMAKE_CURRENT_BINARY_DIR}"
    )
target_compile_options(vusb PUBLIC -funsigned-char)