//
// Created by harold on 23/10/18.
//

#include "Keyboard.h"
#include "keymap.h"

#include <stddef.h>

namespace keyboard {
	bool keyboard::poll_event() {
		if (!uart::poll()) {
			return false;
		}
		uint8_t c = uart::recv();
		switch (m_mode) {
			case mode::reset:
				if (c == response::reset_ok) {
					m_mode = mode::normal;
				} else if (c == response::reset_fail1) {
					for (size_t i = 0; i < 6; i++) {
						report.keys[i] = KeyUsage::ERROR_POST_FAIL;
					}
				} else if (c == response::reset_fail2) {
					m_mode = mode::error;
				}
				break;
			case mode::normal:
				if (c == response::layout) {
					m_mode = mode::layout;
				} else if (c == response::reset) {
					m_mode = mode::reset;
				} else if (c == response::idle) {
					report = {0};
					if (rollover) {
						rollover = false;
						command(::keyboard::command::click_off);
					}
					return true;
				} else if (rollover) {
					// Do nothing
				} else {
					// Parse keycode
					return handle_keycode(c);
				}
				break;
			case mode::error:
				break;
			case mode::layout:
				// c is layout
				m_mode = mode::normal;
				break;
		}
		return false;
	}

	bool keyboard::handle_keycode(uint8_t c) {
		bool is_break = (c >= 0x80);

		c &= 0x7F;
		auto key = keymap[c];
		if (key == KeyUsage::RESERVED) {
			return false;
		}

		if (key >= KeyUsage::LEFTCTRL && key <= KeyUsage::RIGHTGUI)
		{
			auto bit = as_byte(key) - as_byte(KeyUsage::LEFTCTRL);
			if (!is_break) {
				report.modMask |= 1 << bit;
			} else {
				report.modMask &= ~(1 << bit);
			}
		} else {
			if (!is_break) {
				size_t i;
				for (i = 0; i < 2; i++) {
					if (report.keys[i] == KeyUsage::RESERVED) {
						report.keys[i] = key;
						break;
					}
				}
				if (i == 2) {
					rollover = true;
					for (i = 0; i < 6; i++) {
						report.keys[i] = KeyUsage::ERROR_ROLLOVER;
					}
					command(::keyboard::command::click_on);
				}
			} else {
				for (size_t i = 0; i < 6; i++) {
					if (report.keys[i] == key) {
						report.keys[i] = KeyUsage::RESERVED;
						break;
					}
				}
			}

		}

		return true;
	}
}