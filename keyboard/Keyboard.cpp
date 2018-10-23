//
// Created by harold on 23/10/18.
//

#include "Keyboard.h"
#include "keymap.h"

extern "C" {
#include <usbdrv.h>
}

#include <stddef.h>

namespace keyboard {
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
				for (size_t i = 0; i < 6; i++) {
					if (report.keys[i] == KeyUsage::RESERVED) {
						report.keys[i] = key;
						break;
					}
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

	void keyboard::send_report_intr() const {
		usbSetInterrupt(const_cast<unsigned char*>(report_data), sizeof(report_data));
	}
}