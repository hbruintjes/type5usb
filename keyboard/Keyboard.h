#pragma once

#include "uart.h"
#include <usb/report.h>

extern "C" {
#include <usbdrv.h>
}

#include <stdint.h>
#include <util/delay.h>
#include <avr/eeprom.h>

namespace keyboard {
	template<typename T, size_t N>
	struct array {
		T m_val[N];

		T* begin() {
			return &(m_val[0]);
		}
		T const* cbegin() const {
			return &(m_val[0]);
		}
		T* end() {
			return &(m_val[N]);
		}
		T const* cend() const {
			return &(m_val[N]);
		}
		T& operator[](size_t i) {
			return m_val[i];
		}
		T const& operator[](size_t i) const {
			return m_val[i];
		}

		size_t size() const {
			return N;
		}
	};
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

	enum class report_type : uint8_t {
		boot = 0,
		key = 1,
		media = 2,
		system = 3,
		none = 0xff
	};

	struct boot_report_t {
		uint8_t modMask = 0;
		uint8_t reserved = 0;
		array<KeyUsage,6> keys = {{KeyUsage::RESERVED}};
	};
	static_assert(sizeof(boot_report_t) == 8, "Invalid report size");

	struct key_report_t {
		report_type report_id = report_type::key;
		uint8_t modMask = 0;
		array<KeyUsage,6> keys = {{KeyUsage::RESERVED}};
	};
	static_assert(sizeof(key_report_t) == 8, "Invalid report size");

	struct led_report_t {
		report_type report_id = report_type::key;
		uint8_t ledmask = 0;
	};
	static_assert(sizeof(led_report_t) == 2, "Invalid report size");

	struct media_report_t {
		report_type report_id = report_type::media;
		uint8_t keyMask = 0;
	};
	static_assert(sizeof(media_report_t) == 2, "Invalid report size");

	struct system_report_t {
		report_type report_id = report_type::system;
		uint8_t keyMask = 0;
	};
	static_assert(sizeof(system_report_t) == 2, "Invalid report size");

	static constexpr uint8_t macro_large = 127;
	static constexpr uint8_t macro_small = 63;
	extern EEMEM uint8_t macro1[macro_large];
	extern EEMEM uint8_t macro1_size;
	extern EEMEM uint8_t macro2[macro_large];
	extern EEMEM uint8_t macro2_size;
	extern EEMEM uint8_t macro3[macro_small];
	extern EEMEM uint8_t macro3_size;
	extern EEMEM uint8_t macro4[macro_small];
	extern EEMEM uint8_t macro4_size;

	class keyhandler {
		union {
			boot_report_t boot_report;
			unsigned char boot_report_data[sizeof(boot_report_t)];
		};
		union {
			key_report_t key_report;
			unsigned char key_report_data[sizeof(key_report_t)];
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
			off,
			reset,
			normal,
			fn,
			error,
			layout,
			keyswap1,
			keyswap2,
			macro_record,
			macro_save,
			morse,
		};

		mode m_mode;
		enum class keystate : uint8_t {
			clear,
			in_use,
			rollover,
		};
		keystate m_keystate;

		union {
			uint8_t m_curOverride;
			uint8_t m_macroSize;
		};
		uint8_t m_macroBuffer[macro_large];
		uint8_t m_ledState;
		uint8_t m_protocol;

		void check_config();
		void clear_config();
		report_type handle_keycode(uint8_t key);
		report_type handle_keycode_fn(uint8_t key);
		void handle_morsecode(uint8_t key);

		report_type play_macro();

		void print_stack();

		template<uint16_t ms>
		void beep() const {
			command(::keyboard::command::bell_on);
			_delay_ms(ms);
			command(::keyboard::command::bell_off);
		}

		void reset_led() {
			command(::keyboard::command::led_status, m_ledState);
		}

		void set_ledstate(uint8_t state) {
			m_ledState = state;
			command(::keyboard::command::led_status, m_ledState);
		}
		void set_led(::keyboard::led led) {
			command(::keyboard::command::led_status, m_ledState | static_cast<uint8_t>(led));
		}

		void command(::keyboard::command command) const {
			uart::send(as_byte(command));
		}
		void command(::keyboard::command command, uint8_t payload) const {
			uart::send(as_byte(command), payload);
		}

		report_type press(KeyUsage key);
		report_type release(KeyUsage key);

	public:
		static constexpr uint8_t protocol_report = 1;
		static constexpr uint8_t protocol_boot = 0;

		keyhandler() noexcept :
			boot_report_data{0}, key_report_data{0}, media_report_data{0}, system_report_data{0},
			m_mode(mode::off), m_keystate(keystate::clear),
			m_curOverride(0), m_macroBuffer{0},
			m_ledState(0), m_protocol(protocol_report)
		{
			key_report.report_id = report_type::key;
			media_report.report_id = report_type::media;
			system_report.report_id = report_type::system;
		}

		void init() {
			// B0: keyboard. B1: LED
			DDRB |= _BV(PORTB0) | _BV(PORTB1);
			check_config();
			//command(::keyboard::command::reset);
		}

		void enable() {
			if (m_mode == mode::off) {
				m_mode = mode::reset; // keyboard performs self-test on powerup
				m_keystate = keystate::clear;
				PORTB |= _BV(PORTB0);
			}
		}

		void disable() {
			if (m_mode != mode::off) {
				m_mode = mode::off;
				PORTB &= ~_BV(PORTB0);
			}
		}

		uint8_t& get_protocol() {
			return m_protocol;
		}

		void set_protocol(uint8_t protocol) {
			if (protocol == protocol_report) {
				m_protocol = protocol;
				PORTB &= ~_BV(PORTB1);
			} else if (protocol == protocol_boot) {
				m_protocol = protocol;
				PORTB |= _BV(PORTB1);
			}
		}

		void poll_event();

		void set_led_report(unsigned char data);

		void send_report_intr(report_type type);

		void update_boot_report() {
			boot_report.modMask = key_report.modMask;
			boot_report.keys = key_report.keys;
		}

		uint8_t set_report_ptr(unsigned char* *ptr, uint8_t main_type, report_type type) {
			if (main_type != 1) {
				//return 0;
			}
			if (m_protocol == protocol_boot) {
				update_boot_report();
				*ptr = const_cast<unsigned char*>(boot_report_data);
				return sizeof(boot_report_data);
			} else {
				switch(type) {
					default:
					case report_type::key:
						// Keyboard
						*ptr = const_cast<unsigned char*>(key_report_data);
						return sizeof(key_report_data);
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
