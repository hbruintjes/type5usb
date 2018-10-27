#include "descriptor_kbd.h"

#include <avr/pgmspace.h>
#include <usbconfig.h>

/* USB report descriptor, size must match usbconfig.h */
extern "C" PROGMEM const char usbDescriptorHidReport[USB_CFG_HID_REPORT_DESCRIPTOR_LENGTH] = {
	USAGE_PAGE(UsagePage::GenericDesktop),
	USAGE(GenericDesktop::Keyboard),
	COLLECTION(Collection::Application),
	    REPORT_ID(1),
		USAGE_PAGE(UsagePage::Keyboard),
		// Report modifier keys
		REPORT_SIZE(1),
		REPORT_COUNT(8),
		USAGE_MIN(as_byte(KeyUsage::LEFTCTRL)),
		USAGE_MAX(as_byte(KeyUsage::RIGHTGUI)),
		LOGICAL_MIN(0),
		LOGICAL_MAX(1),
		INPUT(MainFlag::Data | MainFlag::Variable | MainFlag::Absolute),
		// Report keys
		REPORT_SIZE(8),
		REPORT_COUNT(6),
		USAGE_MIN(as_byte(KeyUsage::RESERVED)),
		USAGE_MAX(as_byte(KeyUsage::VOLUME_DOWN)),
		LOGICAL_MIN(as_byte(KeyUsage::RESERVED)),
		LOGICAL_MAX(as_byte(KeyUsage::VOLUME_DOWN)),
		INPUT(MainFlag::Data | MainFlag::Array | MainFlag::Absolute),
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
	END_COLLECTION(),
	USAGE_PAGE(UsagePage::Consumer),
	USAGE(Consumer::Control),
	COLLECTION(Collection::Application),
	    REPORT_ID(2),
		// Report media keys
		REPORT_SIZE(1),
		REPORT_COUNT(3),
		USAGE(Consumer::Mute),
		USAGE(Consumer::VolumeInc),
		USAGE(Consumer::VolumeDec),
		LOGICAL_MIN(0),
		LOGICAL_MAX(1),
		INPUT(MainFlag::Data | MainFlag::Variable | MainFlag::Absolute),
		// Reserved
		REPORT_SIZE(1),
		REPORT_COUNT(5),
		INPUT(MainFlag::Constant | MainFlag::Variable | MainFlag::Absolute),
	END_COLLECTION(),
	USAGE_PAGE(UsagePage::GenericDesktop),
	USAGE(GenericDesktop::SystemControl),
	COLLECTION(Collection::Application),
		REPORT_ID(3),
		// Report system keys
		REPORT_SIZE(1),
		REPORT_COUNT(2),
		USAGE(GenericDesktop::SystemPowerDown),
		USAGE(GenericDesktop::SystemSleep),
		LOGICAL_MIN(0),
		LOGICAL_MAX(1),
		INPUT(MainFlag::Data | MainFlag::Variable | MainFlag::Absolute),
		// Reserved
		REPORT_SIZE(1),
		REPORT_COUNT(6),
		INPUT(MainFlag::Constant | MainFlag::Variable | MainFlag::Absolute),
	END_COLLECTION(),
};
