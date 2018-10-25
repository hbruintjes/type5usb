#pragma once

#include "uart.h"
#include <usb/report.h>

extern "C" {
#include <usbdrv.h>
}

#include <stdint.h>

namespace keyboard {
	enum class command : uint8_t {
		// Commands to keyboard
		reset = 0x01,
		bell_on = 0x02,
		bell_off = 0x03,
		click_on = 0x0A,
		click_off = 0x0B,
		led_status = 0x0E,
		layout = 0x0F,
	};

	namespace response {
		// Responses from keyboard
		constexpr uint8_t idle = 0x7F;
		constexpr uint8_t layout = 0xFE; // followed by layout byte
		constexpr uint8_t reset = 0xFF;  // followed by 0x04, then make codes or IDLE. When failed, 0x7E 0x01
		constexpr uint8_t reset_ok = 0x04;
		constexpr uint8_t reset_fail1 = 0x7E;
		constexpr uint8_t reset_fail2 = 0x01;
	};

	enum class led : uint8_t {
		numlock = 0x01,
		compose = 0x02,
		scrolllock = 0x04,
		capslock = 0x08,
	};

	struct report_t {
		uint8_t modMask;
		uint8_t reserved;
		KeyUsage keys[6];
	};
	static_assert(sizeof(report_t) == 8, "Invalid report size");

	class keyboard {
		union {
			report_t report;
			unsigned char report_data[sizeof(report_t)];
		};
		enum class mode : uint8_t {
			reset,
			normal,
			error,
			layout,
			keyswap,
			macro_record,
		};

		mode m_mode;
		enum class keystate : uint8_t {
			clear,
			in_use,
			rollover,
			fn
		};
		keystate m_keystate;

		bool handle_keycode(uint8_t key);

		uint8_t keyOverride[128];
		uint8_t macroBuffer[64];
		uint8_t macroSize;

	public:
		keyboard() noexcept :
			report({0}), m_mode(mode::normal), m_keystate(keystate::clear),
			keyOverride{0}, macroBuffer{0}, macroSize(0)
		{
		}

		void init() {
			command(::keyboard::command::reset);
		}

		void command(::keyboard::command command) {
			uart::send(as_byte(command));
		}
		void command(::keyboard::command command, uint8_t payload) {
			uart::send(as_byte(command), payload);
		}

		bool poll_event();

		void set_led_report(unsigned char data);

		void send_report_intr() const {
			usbSetInterrupt(const_cast<unsigned char*>(report_data), sizeof(report_data));
		}

		uint8_t set_report_ptr(unsigned char* *ptr) const {
			*ptr = const_cast<unsigned char*>(report_data);
			return sizeof(report_data);
		}
	};
}
