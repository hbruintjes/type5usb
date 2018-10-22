#include "report.h"
#include <usbconfig.h>

/* USB report descriptor, size must match usbconfig.h */
extern "C" PROGMEM const char usbDescriptorHidReport[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
	USAGE_PAGE(UsagePage::GenericDesktop),
	USAGE(GenericDesktop::Mouse),
	COLLECTION(Collection::Application),
		USAGE(GenericDesktop::Pointer),
		COLLECTION(Collection::Physical),
			USAGE_PAGE(UsagePage::Button),
			// Report buttons 1-3
			USAGE_MIN(0x01),
			USAGE_MAX(0x03),
			// Buttons toggle 0,1
			LOGICAL_MIN(0x00),
			LOGICAL_MAX(0x01),
			REPORT_COUNT(0x03),
			REPORT_SIZE(0x01),
			INPUT(MainFlag::Data | MainFlag::Variable | MainFlag::Absolute),

			REPORT_COUNT(0x01),
			REPORT_SIZE(0x05),
			INPUT(MainFlag::Constant | MainFlag::Variable | MainFlag::Absolute),

			USAGE_PAGE(UsagePage::GenericDesktop),
			// Report 3 axis
			USAGE(GenericDesktop::X),
			USAGE(GenericDesktop::Y),
			USAGE(GenericDesktop::Wheel),
			// Rel value -127..127
			LOGICAL_MIN((char)0x81),
			LOGICAL_MAX(0x7F),
			REPORT_SIZE(0x08),
			REPORT_COUNT(0x03),
			INPUT(MainFlag::Data | MainFlag::Variable | MainFlag::Relative),
		END_COLLECTION(),
	END_COLLECTION()
};

/* This is the same report descriptor as seen in a Logitech mouse. The data
 * described by this descriptor consists of 4 bytes:
 *      .  .  .  .  . B2 B1 B0 .... one byte with mouse button states
 *     X7 X6 X5 X4 X3 X2 X1 X0 .... 8 bit signed relative coordinate x
 *     Y7 Y6 Y5 Y4 Y3 Y2 Y1 Y0 .... 8 bit signed relative coordinate y
 *     W7 W6 W5 W4 W3 W2 W1 W0 .... 8 bit signed relative coordinate wheel
 */
struct report_t {
	unsigned char   buttonMask;
	signed char    dx;
	signed char    dy;
	signed char    dWheel;
};
