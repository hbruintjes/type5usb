//
// Created by harold on 23/10/18.
//

#include "Keyboard.h"
#include "keymap.h"

#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <stddef.h>
#include <util/delay.h>

namespace morse {
	extern const PROGMEM uint8_t codes[36] = {
			0b010'01000, //a
			0b100'10000, //b
			0b100'10100, //c
			0b011'10000, //d

			0b001'00000, //e
			0b100'00100, //f
			0b011'11000, //g
			0b100'00000, //h
			0b010'00000, //i
			0b100'01110, //j
			0b011'10100, //k
			0b100'01000, //l
			0b010'11000, //m
			0b010'10000, //n
			0b011'11100, //o
			0b100'01100, //p
			0b100'11010, //q
			0b011'01000, //r
			0b011'00000, //s
			0b001'10000, //t
			0b011'00100, //u
			0b100'00010, //v
			0b011'01100, //w
			0b100'10010, //x
			0b100'10110, //y
			0b100'11000, //z

			0b101'01111, //1
			0b101'00111, //2
			0b101'00011, //3
			0b101'00001, //4
			0b101'00000, //5
			0b101'10000, //6
			0b101'11000, //7
			0b101'11100, //8
			0b101'11110, //9
			0b101'11111, //0
	};
}

namespace keyboard {
	constexpr uint8_t keymap_version = 2;
	void keyboard::load_overrides() {
		if (eeprom_read_byte(reinterpret_cast<uint8_t*>(keymapSize)) == keymap_version) {
			// Seems OK
			eeprom_read_block(m_keyMap, reinterpret_cast<uint8_t*>(0), keymapSize);
		} else {
			clear_overrides();
		}
	}

	void keyboard::clear_overrides() {
		eeprom_update_byte(reinterpret_cast<uint8_t*>(keymapSize), ~keymap_version);
		for (uint8_t i = 0; i < keymapSize; i++) {
			uint8_t key = pgm_read_word_near(keymap + i);
			m_keyMap[i] = static_cast<KeyUsage>(key);
		}
		eeprom_update_block(m_keyMap, reinterpret_cast<uint8_t*>(0), keymapSize);
		eeprom_update_byte(reinterpret_cast<uint8_t*>(keymapSize), keymap_version);
	}

	report_type keyboard::poll_event() {
		if (!uart::poll()) {
			return report_type::none;
		}
		uint8_t c = uart::recv();

		// Handle keyboard response codes
		if (c == response::layout) {
			m_mode = mode::layout;
		} else if (c == response::reset) {
			return report_type::none;
			m_mode = mode::reset;
			return report_type::none;
		}

		switch (m_mode) {
			case mode::reset:
				if (c == response::reset_ok) {
					command(::keyboard::command::led_status, m_ledState);
					m_mode = mode::normal;
				} else if (c == response::reset_fail1) {
					for (size_t i = 0; i < 6; i++) {
						key_report.keys[i] = KeyUsage::ERROR_POST_FAIL;
					}
				} else if (c == response::reset_fail2) {
					m_mode = mode::error;
				}
				break;
			case mode::normal:
				// Handle keyboard response codes
				if (c == response::idle) {
					if (m_keystate != keystate::clear) {
						key_report.modMask = 0;
						for (size_t i = 0; i < 6; i++) {
							key_report.keys[i] = KeyUsage::RESERVED;
						}
						if (m_keystate == keystate::rollover) {
							command(::keyboard::command::click_off);
						}
						m_keystate = keystate::clear;
						return report_type::key;
					} else {
						// Avoid reporting empty too often
						return report_type::none;
					}
				}

				// Base action on what keys are depressed
				switch (m_keystate) {
					case keystate::clear:
						if (c == ::keyboard::keys::fn) {
							m_mode = mode::fn;
							toggle_led(::keyboard::led::compose);
							break;
						}
						// Fall-through
					case keystate::rollover:
						// Handled by handle_keycode, modifiers may still be
						// reported
					default: // clear or in_use
						return handle_keycode(c);
				}
				break;
			case mode::fn:
				if (c == response::idle) {
					m_mode = mode::normal;
					toggle_led(::keyboard::led::compose);
				} else {
					return handle_keycode_fn(c);
				}
				break;
			case mode::morse:
				if (c == ::keyboard::keys::fn) {
					m_mode = mode::normal;
					toggle_led(::keyboard::led::compose);
				} else {
					handle_morsecode(c);
				}
				break;
			case mode::keyswap1:
				if (c == ::keyboard::keys::fn) {
					toggle_led(::keyboard::led::compose);
					m_mode = mode::normal;
				} else if (c < response::idle) {
					m_curOverride = c;
					m_mode = mode::keyswap2;
				}
				break;
			case mode::keyswap2:
				if (c < response::idle) {
					toggle_led(::keyboard::led::compose);
					m_mode = mode::normal;
					if (c != ::keyboard::keys::fn) {
						// Read original code from flash
						auto keyUsage = static_cast<KeyUsage>(pgm_read_word_near(keymap + m_curOverride));
						m_keyMap[c] = keyUsage;
						eeprom_update_byte(reinterpret_cast<uint8_t*>(c), static_cast<uint8_t>(keyUsage));
						beep<50>();
					}
				}
				break;
			case mode::error:
				break;
			case mode::layout:
				// c is layout
				m_mode = mode::normal;
				break;
			case mode::macro_record:
				break;
		}
		return report_type::none;
	}

	report_type keyboard::handle_keycode(uint8_t c) {
		bool is_break = (c & 0x80) != 0;
		c &= 0x7F;

		auto key = m_keyMap[c];
		if (key == KeyUsage::RESERVED) {
			return report_type::none;
		}

		if (!is_break && m_keystate != keystate::rollover) {
			m_keystate = keystate::in_use;
		}

		if (key >= KeyUsage::LEFTCTRL && key <= KeyUsage::RIGHTGUI)
		{
			auto bit = as_byte(key) - as_byte(KeyUsage::LEFTCTRL);
			if (!is_break) {
				key_report.modMask |= 1 << bit;
			} else {
				key_report.modMask &= ~(1 << bit);
			}
		} else if (key >= KeyUsage::MUTE && key <= KeyUsage::VOLUME_DOWN) {
			auto bit = as_byte(key) - as_byte(KeyUsage::MUTE);
			if (!is_break) {
				media_report.keyMask |= 1 << bit;
			} else {
				media_report.keyMask &= ~(1 << bit);
			}
			return report_type::media;
		} else if (key == KeyUsage::POWER) {
			auto bit = as_byte(key) - as_byte(KeyUsage::POWER);
			if (!is_break) {
				system_report.keyMask |= 1 << bit;
			} else {
				media_report.keyMask &= ~(1 << bit);
			}
			return report_type::system;
		} else if (m_keystate != keystate::rollover) {
			if (!is_break) {
				size_t i;
				for (i = 0; i < 6; i++) {
					if (key_report.keys[i] == KeyUsage::RESERVED) {
						key_report.keys[i] = key;
						break;
					}
				}
				if (i == 6) {
					m_keystate = keystate::rollover;
					for (i = 0; i < 6; i++) {
						key_report.keys[i] = KeyUsage::ERROR_ROLLOVER;
					}
					command(::keyboard::command::click_on);
				}
			} else {
				command(::keyboard::command::click_off);
				for (size_t i = 0; i < 6; i++) {
					if (key_report.keys[i] == key) {
						key_report.keys[i] = KeyUsage::RESERVED;
						break;
					}
				}
			}

		}

		return report_type::key;
	}

	report_type keyboard::handle_keycode_fn(uint8_t c) {
		if ((c & 0x80) != 0) {
			//TODO: check if shifted key, return true if so
			return report_type::none;
		}
		if (c == ::keyboard::keys::help) {
			m_mode = mode::morse;
			beep<150>();
		} else if (c == ::keyboard::keys::copy) {
			m_mode = mode::keyswap1;
			beep<150>();
		} else if (c == ::keyboard::keys::undo) {
			clear_overrides();
			beep<150>();
		}

		return report_type::none;
	}
	void keyboard::handle_morsecode(uint8_t c) {
		if ((c & 0x80) != 0) {
			return;
		}
		c &= 0x7F;

		// USB keycodes are nicely arranged
		auto key = static_cast<KeyUsage>(pgm_read_byte_near(keymap + c));
		if (key >= KeyUsage::A && key <= KeyUsage::N0) {
			c = as_byte(key) - as_byte(KeyUsage::A);
		} else {
			return;
		}
		auto code = static_cast<uint8_t>(pgm_read_byte_near(morse::codes + c));
		uint8_t len = code >> 5;
		code <<= 3;
		wdt_disable();
		while(len > 0) {
			if (code & 0x80) {
				// long
				beep<150>();
			} else {
				// short
				beep<75>();
			}
			_delay_ms(75);

			code <<= 1;
			len--;
		}
		_delay_ms(150);
		wdt_enable(WDTO_1S);
	}

	void keyboard::set_led_report(unsigned char data) {
		// Map LED bits to keyboard
		uint8_t ledStatus = 0;
		if (data & _BV(0)) {
			ledStatus |= as_byte(led::numlock);
		}
		if (data & _BV(1)) {
			ledStatus |= as_byte(led::capslock);
		}
		if (data & _BV(2)) {
			ledStatus |= as_byte(led::scrolllock);
		}
		if (data & _BV(3)) {
			ledStatus |= as_byte(led::compose);
		}
		set_led(ledStatus);
	}
}