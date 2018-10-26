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

	struct boot_report_t {
		uint8_t modMask = 0;
		uint8_t reserved = 0;
		KeyUsage keys[6] = {KeyUsage::RESERVED};
	};
	static_assert(sizeof(boot_report_t) == 8, "Invalid report size");

	enum class report_type : uint8_t {
		boot = 0,
		key = 1,
		media = 2,
		system = 3,
		none = 0xff
	};

	struct report_t {
		report_type report_id = report_type::key;
		union {
			boot_report_t boot_report;
			unsigned char boot_report_data[sizeof(boot_report_t)];
		};

	};

	struct media_report_t {
		report_type report_id = report_type::media;
		uint8_t keyMask = 0;
	};

	struct system_report_t {
		report_type report_id = report_type::system;
		uint8_t keyMask = 0;
	};

	class keyboard {
		union {
			report_t report;
			unsigned char report_data[sizeof(report_t)];
		};
		union {
			media_report_t media_report;
			unsigned char media_report_data[sizeof(media_report_t)];
		};
		union {
			system_report_t system_report;
			unsigned char system_report_data[sizeof(system_report_t)];
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
		report_type handle_keycode(uint8_t key);
		report_type handle_keycode_fn(uint8_t key);
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
			m_mode(mode::normal), m_keystate(keystate::clear),
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

		report_type poll_event();

		void set_led_report(unsigned char data);

		void send_report_intr(report_type type) const {
			if (m_protocol == protocol_boot && (type == report_type::boot || type == report_type::key)) {
				usbSetInterrupt(const_cast<unsigned char *>(report.boot_report_data), sizeof(report.boot_report_data));
			} else {
				usbSetInterrupt(const_cast<unsigned char *>(report_data), sizeof(report_data));
			}
		}

		uint8_t set_report_ptr(unsigned char* *ptr, uint8_t type, report_type id) const {
			if (type != 1) {
				return 0;
			}
			if (m_protocol == protocol_boot) {
				*ptr = const_cast<unsigned char*>(report.boot_report_data);
				return sizeof(report.boot_report_data);
			} else {
				switch(id) {
					default:
					case report_type::key:
						// Keyboard
						*ptr = const_cast<unsigned char*>(report_data);
						return sizeof(report_data);
					case report_type::media:
						// Media
						*ptr = const_cast<unsigned char*>(media_report_data);
						return sizeof(media_report_data);
					case report_type::system:
						// System
						*ptr = const_cast<unsigned char*>(system_report_data);
						return sizeof(system_report_data);
				}

			}
		}
	};
}
