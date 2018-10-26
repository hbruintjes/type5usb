#pragma once

#include "uart.h"
#include <usb/report.h>

extern "C" {
#include <usbdrv.h>
}

#include <stdint.h>
#include <util/delay.h>

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
			fn,
			error,
			layout,
			keyswap1,
			keyswap2,
			macro_record,
			morse,
		};

		mode m_mode;
		enum class keystate : uint8_t {
			clear,
			in_use,
			rollover,
		};
		keystate m_keystate;

		static constexpr uint8_t keymapSize = 128;
		KeyUsage m_keyMap[keymapSize];
		KeyUsage m_curOverride;
		uint8_t m_macroBuffer[64];
		uint8_t m_macroSize;
		uint8_t m_ledState;
		uint8_t m_protocol;

		void load_overrides();
		void clear_overrides();
		bool handle_keycode(uint8_t key);
		bool handle_keycode_fn(uint8_t key);
		void handle_morsecode(uint8_t key);

		template<uint16_t ms>
		void beep() {
			command(::keyboard::command::bell_on);
			_delay_ms(ms);
			command(::keyboard::command::bell_off);
		}

		void set_led(uint8_t state) {
			m_ledState = state;
			command(::keyboard::command::led_status, m_ledState);
		}
		void toggle_led(::keyboard::led led) {
			m_ledState ^= static_cast<uint8_t>(led);
			command(::keyboard::command::led_status, m_ledState);
		}

	public:
		static constexpr uint8_t protocol_report = 0;
		static constexpr uint8_t protocol_boot = 1;

		keyboard() noexcept :
			report({0}), m_mode(mode::normal), m_keystate(keystate::clear),
			m_keyMap{KeyUsage::RESERVED}, m_curOverride(KeyUsage::RESERVED),
			m_macroBuffer{0}, m_macroSize(0),
			m_ledState(0), m_protocol(protocol_report)
		{
		}

		void init() {
			command(::keyboard::command::reset);
			load_overrides();
		}

		uint8_t& get_protocol() {
			return m_protocol;
		}

		void set_protocol(uint8_t protocol) {
			if (protocol == protocol_report || protocol == protocol_boot) {
				m_protocol = protocol;
			}
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
			if (m_protocol == protocol_boot) {
				return 8; // Fixed report size and layout
			} else {
				return sizeof(report_data);
			}
		}
	};
}
