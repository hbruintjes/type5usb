//
// Created by harold on 23/10/18.
//

#include "Keyboard.h"
#include "keymap.h"

#include <avr/eeprom.h>
#include <avr/wdt.h>
#include <stddef.h>
#include <util/delay.h>

#define DIM(x) (sizeof(x)/sizeof(x[0]))

extern "C" {
	uint16_t StackCount(void);
}

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
	EEMEM uint8_t macro1[macro_large];
	EEMEM uint8_t macro1_size = 0;
	EEMEM uint8_t macro2[macro_large];
	EEMEM uint8_t macro2_size = 0;
	EEMEM uint8_t macro3[macro_small];
	EEMEM uint8_t macro3_size = 0;
	EEMEM uint8_t macro4[macro_small];
	EEMEM uint8_t macro4_size = 0;

	KeyUsage read_keymap(uint8_t c) {
		if (c > DIM(keyboard::keymap_eeprom)) {
			return KeyUsage::RESERVED;
		}
		return static_cast<KeyUsage>(eeprom_read_byte(reinterpret_cast<uint8_t*>(keyboard::keymap_eeprom + c)));
	}

	void keyhandler::check_config() {
		auto version = eeprom_read_byte(&keyboard::keymap_eeprom_version);
		if (version != keyboard::keymap_version) {
			clear_config();
		}
	}

	void keyhandler::clear_config() {
		eeprom_update_byte(&keyboard::keymap_eeprom_version, ~keyboard::keymap_version);
		for (uint8_t i = 0; i < DIM(keyboard::keymap_eeprom); i++) {
			uint8_t key = pgm_read_byte_near(keymap_flash + i);
			eeprom_update_byte(reinterpret_cast<uint8_t*>(keyboard::keymap_eeprom + i), key);
		}
		eeprom_update_byte(&keyboard::keymap_eeprom_version, keyboard::keymap_version);
		eeprom_update_byte(&macro1_size, 0);
		eeprom_update_byte(&macro2_size, 0);
		eeprom_update_byte(&macro3_size, 0);
		eeprom_update_byte(&macro4_size, 0);
	}

	void keyhandler::poll_event() {
		if (!uart::poll()) {
			return;
		}
		uint8_t c = uart::recv();

		// Handle keyboard response codes
		if (c == response::layout) {
			m_mode = mode::layout;
		} else if (c == response::reset) {
			m_mode = mode::reset;
			return;
		}

		switch (m_mode) {
			case mode::off:
				// Uhhh, what?
				return;
			case mode::reset:
				if (c == response::reset_ok) {
					command(::keyboard::command::led_status, m_ledState);
					m_mode = mode::normal;
				} else if (c == response::reset_fail1) {
					for (auto& key: key_report.keys) {
						key = KeyUsage::ERROR_POST_FAIL;
					}
				} else if (c == response::reset_fail2) {
					m_mode = mode::error;
				}
				return;
			case mode::normal:
			case mode::macro_record:
				// Handle keyboard response codes
				if (c == response::idle) {
					if (m_keystate != keystate::clear) {
						key_report.modMask = 0;
						for (auto& key: key_report.keys) {
							key = KeyUsage::RESERVED;
						}
						if (m_keystate == keystate::rollover) {
							command(::keyboard::command::click_off);
						}
						m_keystate = keystate::clear;
						send_report_intr(report_type::key);
						return;
					}
					return;
				}

				if (c == ::keyboard::keys::fn) {
					if (m_mode == mode::macro_record) {
						command(::keyboard::command::click_off);
						reset_led();
						m_mode = mode::normal;
					} else if (m_keystate == keystate::clear) {
						m_mode = mode::fn;
						set_led(::keyboard::led::compose);
					}
					// FN never acts as a regular key
					return;
				}

				send_report_intr(handle_keycode(c));
				return;
			case mode::fn:
				if (c == response::idle) {
					reset_led();
					m_mode = mode::normal;
				} else {
					send_report_intr(handle_keycode_fn(c));
				}
				return;
			case mode::morse:
				if (c == ::keyboard::keys::fn) {
					reset_led();
					m_mode = mode::normal;
				} else {
					handle_morsecode(c);
				}
				return;
			case mode::keyswap1:
				if (c == ::keyboard::keys::fn) {
					reset_led();
					m_mode = mode::normal;
				} else if (c < response::idle) {
					m_curOverride = c;
					m_mode = mode::keyswap2;
				}
				return;
			case mode::keyswap2:
				if (c < response::idle) {
					reset_led();
					m_mode = mode::normal;
					if (c != ::keyboard::keys::fn) {
						// Read original code from flash
						auto keyUsage = static_cast<KeyUsage>(pgm_read_word_near(keymap_flash + m_curOverride));
						eeprom_update_byte(reinterpret_cast<uint8_t*>(keymap_eeprom + c), static_cast<uint8_t>(keyUsage));
						beep<50>();
					}
				}
				return;
			case mode::macro_save:
				if (c < response::idle) {
					bool ok = false;
					switch (c) {
						case ::keyboard::keys::n1:
						case ::keyboard::keys::f9:
							if (m_macroSize <= DIM(macro1)) {
								eeprom_update_block(m_macroBuffer, macro1, m_macroSize);
								eeprom_update_byte(&macro1_size, m_macroSize);
								ok = true;
							}
							break;
						case ::keyboard::keys::n2:
						case ::keyboard::keys::f10:
							if (m_macroSize <= DIM(macro2)) {
								eeprom_update_block(m_macroBuffer, macro2, m_macroSize);
								eeprom_update_byte(&macro2_size, m_macroSize);
								ok = true;
							}
							break;
						case ::keyboard::keys::n3:
						case ::keyboard::keys::f11:
							if (m_macroSize <= DIM(macro3)) {
								eeprom_update_block(m_macroBuffer, macro3, m_macroSize);
								eeprom_update_byte(&macro3_size, m_macroSize);
								ok = true;
							}
							break;
						case ::keyboard::keys::n4:
						case ::keyboard::keys::f12:
							if (m_macroSize <= DIM(macro4)) {
								eeprom_update_block(m_macroBuffer, macro4, m_macroSize);
								eeprom_update_byte(&macro4_size, m_macroSize);
								ok = true;
							}
							break;
						default:
							break;
					}

					if (ok) {
						beep<50>();
					} else {
						beep<50>();
						_delay_ms(50);
						beep<50>();
					}
					reset_led();
					m_mode = mode::normal;
				}
				return;
			case mode::error:
				return;
			case mode::layout:
				// c is layout
				m_mode = mode::normal;
				return;
		}
	}

	report_type keyhandler::press(KeyUsage key) {
		if (key >= KeyUsage::LEFTCTRL && key <= KeyUsage::RIGHTGUI)
		{
			auto bit = as_byte(key) - as_byte(KeyUsage::LEFTCTRL);
			key_report.modMask |= 1 << bit;
			m_keystate = keystate::in_use;
			return report_type::key;
		} else if (key >= KeyUsage::MUTE && key <= KeyUsage::VOLUME_DOWN) {
			auto bit = as_byte(key) - as_byte(KeyUsage::MUTE);
			media_report.keyMask |= 1 << bit;
			return report_type::media;
		} else if (key == KeyUsage::POWER) {
			// Since sleep & power are the same key (but shifted)
			// simply overwrite value, do not OR
			if (key_report.modMask & 0b00100010) {
				// power
				system_report.keyMask = 0b01;
			} else {
				// sleep
				system_report.keyMask = 0b10;
			}
			/*
			auto bit = as_byte(key) - as_byte(KeyUsage::POWER);
			system_report.keyMask |= 1 << bit;
			 */
			return report_type::system;
		}
		if (key >= KeyUsage::RESERVED && key <= KeyUsage::VOLUME_DOWN &&
		  m_keystate != keystate::rollover) {
			size_t i;
			for (auto& rkey: key_report.keys) {
				if (rkey == KeyUsage::RESERVED) {
					rkey = key;
					break;
				}
			}
			if (i == key_report.keys.size()) {
				m_keystate = keystate::rollover;
				for (auto& rkey: key_report.keys) {
					rkey = KeyUsage::ERROR_ROLLOVER;
				}
				command(::keyboard::command::click_on);
			} else {
				m_keystate = keystate::in_use;
			}
			return report_type::key;
		}
		return report_type::none;
	}

	report_type keyhandler::release(KeyUsage key) {
		if (key >= KeyUsage::LEFTCTRL && key <= KeyUsage::RIGHTGUI) {
			auto bit = as_byte(key) - as_byte(KeyUsage::LEFTCTRL);
			key_report.modMask &= ~(1 << bit);
		} else if (key >= KeyUsage::MUTE && key <= KeyUsage::VOLUME_DOWN) {
			auto bit = as_byte(key) - as_byte(KeyUsage::MUTE);
			media_report.keyMask &= ~(1 << bit);
			return report_type::media;
		} else if (key == KeyUsage::POWER) {
			system_report.keyMask = 0;
			/* Since both sleep & power are the same key, release both since
			 * shift may be released beforehand
			auto bit = as_byte(key) - as_byte(KeyUsage::POWER);
			system_report.keyMask &= ~(1 << bit);
			 */
			return report_type::system;
		} else if (key >= KeyUsage::RESERVED && key <= KeyUsage::VOLUME_DOWN &&
		  m_keystate != keystate::rollover) {
			for (auto& rkey: key_report.keys) {
				if (rkey == key) {
					rkey = KeyUsage::RESERVED;
					break;
				}
			}
		}

		if (m_keystate != keystate::clear) {
			bool clear = true;
			for (auto const& rkey: key_report.keys) {
				if (rkey != KeyUsage::RESERVED) {
					clear = false;
					break;
				}
			}
			if (clear && key_report.modMask == 0) {
				m_keystate = keystate::clear;
			}
		}

		return report_type::key;
	}

	report_type keyhandler::handle_keycode(uint8_t c) {
		uint8_t break_bit = (c & static_cast<uint8_t>(0x80));
		c &= 0x7F;

		auto key = read_keymap(c);
		if (key == KeyUsage::RESERVED) {
			return report_type::none;
		}

		report_type type;
		if (!break_bit) {
			type = press(key);
		} else {
			type = release(key);
		}

		if (type == report_type::key && m_mode == mode::macro_record) {
			m_macroBuffer[m_macroSize] = (c | break_bit);
			m_macroSize++;
			if (m_macroSize == DIM(m_macroBuffer)) {
				command(::keyboard::command::click_off);
				reset_led();
				m_mode = mode::normal;
				beep<150>();
			}
		}

		return type;
	}

	report_type keyhandler::handle_keycode_fn(uint8_t c) {
		if ((c & 0x80) != 0) {
			//TODO: check if shifted key, return true if so
			return report_type::none;
		}
		if (c == ::keyboard::keys::help) {
			m_mode = mode::morse;
			beep<150>();
		} else if (c == ::keyboard::keys::cut) {
			m_mode = mode::keyswap1;
			beep<150>();
		} else if (c == ::keyboard::keys::escape) {
			clear_config();
			beep<150>();
		} else if (c == ::keyboard::keys::copy) {
			m_mode = mode::macro_record;
			m_macroSize = 0;
			command(::keyboard::command::click_on);
		} else if (c == ::keyboard::keys::paste) {
			m_mode = mode::macro_save;
		} else if (c == ::keyboard::keys::n1 || c == ::keyboard::keys::f9) {
			m_macroSize = eeprom_read_byte(&macro1_size);
			eeprom_read_block(m_macroBuffer, macro1, m_macroSize);
			return play_macro();
		} else if (c == ::keyboard::keys::n2 || c == ::keyboard::keys::f10) {
			m_macroSize = eeprom_read_byte(&macro2_size);
			eeprom_read_block(m_macroBuffer, macro2, m_macroSize);
			return play_macro();
		} else if (c == ::keyboard::keys::n3 || c == ::keyboard::keys::f11) {
			m_macroSize = eeprom_read_byte(&macro3_size);
			eeprom_read_block(m_macroBuffer, macro3, m_macroSize);
			return play_macro();
		} else if (c == ::keyboard::keys::n4 || c == ::keyboard::keys::f12) {
			m_macroSize = eeprom_read_byte(&macro4_size);
			eeprom_read_block(m_macroBuffer, macro4, m_macroSize);
			return play_macro();
		} else if (c == ::keyboard::keys::again || c == ::keyboard::keys::insert) {
			return play_macro();
		} else if (c == ::keyboard::keys::stop) {
			print_stack();
		}

		return report_type::none;
	}

	void keyhandler::print_stack() {
		auto count = StackCount();

		key_report.modMask = count;
		send_report_intr(report_type::key);
		key_report.modMask = count >> 8;
		send_report_intr(report_type::key);

		key_report.modMask = SPL;
		send_report_intr(report_type::key);
		key_report.modMask = SPH;
		send_report_intr(report_type::key);

		key_report.modMask = 0;
		send_report_intr(report_type::key);
	}

	report_type keyhandler::play_macro() {
		// Playback of macro buffer
		for(uint8_t i = 0; i < m_macroSize; i++) {
			handle_keycode(m_macroBuffer[i]);
			send_report_intr(report_type::key);
		}
		// Send all keys up
		if (m_keystate != keystate::clear) {
			key_report.modMask = 0;
			for (uint8_t i = 0; i < sizeof(key_report.keys); i++) {
				key_report.keys[i] = KeyUsage::RESERVED;
			}
			m_keystate = keystate::clear;
			return report_type::key;
		}

		return report_type::none;
	}

	void keyhandler::handle_morsecode(uint8_t c) {
		if ((c & 0x80) != 0) {
			return;
		}
		c &= 0x7F;

		// USB keycodes are nicely arranged
		auto key = static_cast<KeyUsage>(pgm_read_byte_near(keymap_flash + c));
		if (key >= KeyUsage::A && key <= KeyUsage::N0) {
			c = as_byte(key) - as_byte(KeyUsage::A);
		} else {
			return;
		}
		auto code = static_cast<uint8_t>(pgm_read_byte_near(morse::codes + c));
		uint8_t len = code >> 5;
		code <<= 3;
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
	}

	void keyhandler::set_led_report(unsigned char data) {
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
		set_ledstate(ledStatus);
	}

	void keyhandler::send_report_intr(report_type type) {
		if (type == report_type::none) {
			return;
		}

		while(!usbInterruptIsReady()) {
			usbPoll();
		}

		if (m_protocol == protocol_boot && (type == report_type::boot || type == report_type::key)) {
			update_boot_report();
			usbSetInterrupt(const_cast<unsigned char *>(boot_report_data), sizeof(boot_report_data));
		} else {
			switch(type) {
				default:
				case report_type::key:
					// Keyboard
					usbSetInterrupt(const_cast<unsigned char *>(key_report_data), sizeof(key_report_data));
					break;
				case report_type::media:
					// Media
					usbSetInterrupt(const_cast<unsigned char *>(media_report_data), sizeof(media_report_data));
					break;
				case report_type::system:
					usbSetInterrupt(const_cast<unsigned char *>(system_report_data), sizeof(system_report_data));
					break;
			}
		}
	}
}