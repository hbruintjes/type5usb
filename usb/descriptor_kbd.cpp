#include "descriptor_kbd.h"

#include <avr/pgmspace.h>
#include <usbconfig.h>

/* USB report descriptor, size must match usbconfig.h */
extern "C" PROGMEM const char usbDescriptorHidReport[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
		USAGE_PAGE(UsagePage::GenericDesktop),
		USAGE(GenericDesktop::Keyboard),
		COLLECTION(Collection::Application),
		USAGE_PAGE(UsagePage::Keyboard),
		// Report modifier keys
		REPORT_SIZE(1),
		REPORT_COUNT(8),
		USAGE_MIN(as_byte(KeyUsage::LEFTCTRL)),
		USAGE_MAX(as_byte(KeyUsage::RIGHTGUI)),
		LOGICAL_MIN(0),
		LOGICAL_MAX(1),
		INPUT(MainFlag::Data | MainFlag::Variable | MainFlag::Absolute),
		// Reserved
		REPORT_SIZE(1),
		REPORT_COUNT(8),
		INPUT(MainFlag::Constant | MainFlag::Variable | MainFlag::Absolute),
		// Status LEDs
		REPORT_SIZE(1),
		REPORT_COUNT(5),
		USAGE_PAGE(UsagePage::LEDs),
		USAGE_MIN(as_byte(LEDUsage::NUMLOCK)),
		USAGE_MAX(as_byte(LEDUsage::KANA)),
		OUTPUT(MainFlag::Data | MainFlag::Variable | MainFlag::Absolute),
		// Padding
		REPORT_SIZE(1),
		REPORT_COUNT(3),
		OUTPUT(MainFlag::Constant | MainFlag::Variable | MainFlag::Absolute),
		USAGE_PAGE(UsagePage::Keyboard),
		// Report keys
		REPORT_SIZE(8),
		REPORT_COUNT(6),
		USAGE_MIN(as_byte(KeyUsage::RESERVED)),
		USAGE_MAX(as_byte(KeyUsage::VOLUME_DOWN)),
		LOGICAL_MIN(as_byte(KeyUsage::RESERVED)),
		LOGICAL_MAX(as_byte(KeyUsage::VOLUME_DOWN)),
		INPUT(MainFlag::Data | MainFlag::Array | MainFlag::Absolute),
		END_COLLECTION()
};
