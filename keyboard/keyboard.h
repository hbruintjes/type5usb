#pragma once

#include <stdint.h>

#include "usb/report.h"

extern const KeyUsage keymap[0x80];

namespace Keyboard {

	namespace Command {
		// Commands to keyboard
		constexpr uint8_t RESET = 0x01;
		constexpr uint8_t BELL_ON = 0x02;
		constexpr uint8_t BELL_OFF = 0x03;
		constexpr uint8_t CLICK_ON = 0x0A;
		constexpr uint8_t CLICK_OFF = 0x0B;
		constexpr uint8_t LED_STATUS = 0x0E;
		constexpr uint8_t LAYOUT = 0x0F;
	}
	namespace Response {
		// Responses from keyboard
		constexpr uint8_t IDLE = 0x7F;
		constexpr uint8_t LAYOUT_RESP = 0xFE; // followed by layout byte
		constexpr uint8_t RESET_RESP = 0xFF; // followed by 0x04, then make codes or IDLE. When failed, 0x7E 0x01
	};

	namespace LED {
		constexpr uint8_t NUMLOCK = 0x01;
		constexpr uint8_t COMPOSE = 0x02;
		constexpr uint8_t SCROLLLOCK = 0x04;
		constexpr uint8_t CAPSLOCK = 0x08;
	}

	enum class LEDStatus{
		OFF,
		BLINK,
		ON
	};
}