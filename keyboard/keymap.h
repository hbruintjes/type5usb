#pragma once

#include "usb/report.h"

#include <avr/eeprom.h>
#include <avr/pgmspace.h>
#include <stdint.h>

namespace keyboard {
	extern const PROGMEM KeyUsage keymap_flash[0x7F];
	extern EEMEM KeyUsage keymap_eeprom[0x7F];

	constexpr uint8_t keymap_version = 2;
	extern EEMEM uint8_t keymap_eeprom_version;

	namespace keys {
		constexpr uint8_t help = 0x76;

		constexpr uint8_t escape = 0x0f; // was fn/unlabeled

		constexpr uint8_t f1 = 0x05;
		constexpr uint8_t f2 = 0x06;
		constexpr uint8_t f3 = 0x08;
		constexpr uint8_t f4 = 0x0a;
		constexpr uint8_t f5 = 0x0c;
		constexpr uint8_t f6 = 0x0e;
		constexpr uint8_t f7 = 0x10;
		constexpr uint8_t f8 = 0x11;
		constexpr uint8_t f9 = 0x12;
		constexpr uint8_t f10 = 0x07;
		constexpr uint8_t f11 = 0x09;
		constexpr uint8_t f12 = 0x0b;

		constexpr uint8_t pr_sc = 0x16;
		constexpr uint8_t scroll_lock = 0x17;
		constexpr uint8_t pause = 0x15;

		constexpr uint8_t vol_mute = 0x2d;
		constexpr uint8_t vol_down = 0x02;
		constexpr uint8_t vol_up = 0x04;
		constexpr uint8_t power = 0x30;

		constexpr uint8_t stop = 0x01;
		constexpr uint8_t again = 0x03;
		constexpr uint8_t props = 0x19;
		constexpr uint8_t undo = 0x1a;
		constexpr uint8_t front = 0x31;
		constexpr uint8_t copy = 0x33;
		constexpr uint8_t open = 0x48;
		constexpr uint8_t paste = 0x49;
		constexpr uint8_t find = 0x5f;
		constexpr uint8_t cut = 0x61;

		constexpr uint8_t backtick = 0x1d; // was escape
		constexpr uint8_t n1 = 0x1e;
		constexpr uint8_t n2 = 0x1f;
		constexpr uint8_t n3 = 0x20;
		constexpr uint8_t n4 = 0x21;
		constexpr uint8_t n5 = 0x22;
		constexpr uint8_t n6 = 0x23;
		constexpr uint8_t n7 = 0x24;
		constexpr uint8_t n8 = 0x25;
		constexpr uint8_t n9 = 0x26;
		constexpr uint8_t n0 = 0x27;
		constexpr uint8_t minus = 0x28;
		constexpr uint8_t eq = 0x29;
		constexpr uint8_t backslash = 0x58;
		constexpr uint8_t fn = 0x2a; // was backtick
		constexpr uint8_t tab = 0x35;
		constexpr uint8_t q = 0x36;
		constexpr uint8_t w = 0x37;
		constexpr uint8_t e = 0x38;
		constexpr uint8_t r = 0x39;
		constexpr uint8_t t = 0x3a;
		constexpr uint8_t y = 0x3b;
		constexpr uint8_t u = 0x3c;
		constexpr uint8_t i = 0x3d;
		constexpr uint8_t o = 0x3e;
		constexpr uint8_t p = 0x3f;
		constexpr uint8_t lbracket = 0x40;
		constexpr uint8_t rbracket = 0x41;
		constexpr uint8_t backspace = 0x2b;
		constexpr uint8_t control = 0x4c;
		constexpr uint8_t a = 0x4d;
		constexpr uint8_t s = 0x4e;
		constexpr uint8_t d = 0x4f;
		constexpr uint8_t f = 0x50;
		constexpr uint8_t g = 0x51;
		constexpr uint8_t h = 0x52;
		constexpr uint8_t j = 0x53;
		constexpr uint8_t k = 0x54;
		constexpr uint8_t l = 0x55;
		constexpr uint8_t semicolon = 0x56;
		constexpr uint8_t quote = 0x57;
		constexpr uint8_t enter = 0x59;
		constexpr uint8_t shift_left = 0x63;
		constexpr uint8_t z = 0x64;
		constexpr uint8_t x = 0x65;
		constexpr uint8_t c = 0x66;
		constexpr uint8_t v = 0x67;
		constexpr uint8_t b = 0x68;
		constexpr uint8_t n = 0x69;
		constexpr uint8_t m = 0x6a;
		constexpr uint8_t comma = 0x6b;
		constexpr uint8_t period = 0x6c;
		constexpr uint8_t slash = 0x6d;
		constexpr uint8_t shift_right = 0x6e;
		constexpr uint8_t caps_lock = 0x77;
		constexpr uint8_t alt = 0x13;
		constexpr uint8_t triangle_right = 0x78;
		constexpr uint8_t space_bar = 0x79;
		constexpr uint8_t triangle_left = 0x7a;
		constexpr uint8_t compose = 0x43;
		constexpr uint8_t graph = 0x0d;

		constexpr uint8_t insert = 0x2c;
		constexpr uint8_t home = 0x34;
		constexpr uint8_t page_up = 0x60;
		constexpr uint8_t del = 0x42;
		constexpr uint8_t end = 0x4a;
		constexpr uint8_t page_down = 0x7b;

		constexpr uint8_t num_lock = 0x62;
		constexpr uint8_t num_slash = 0x2e;
		constexpr uint8_t num_times = 0x2f;
		constexpr uint8_t num_minus = 0x47;
		constexpr uint8_t num_home = 0x44;
		constexpr uint8_t num_cur_up = 0x45;
		constexpr uint8_t num_pgup = 0x46;
		constexpr uint8_t num_plus = 0x7d;
		constexpr uint8_t num_cur_left = 0x5b;
		constexpr uint8_t num_n5 = 0x5c;
		constexpr uint8_t num_cur_right = 0x5d;
		constexpr uint8_t num_end = 0x70;
		constexpr uint8_t num_cur_dn = 0x71;
		constexpr uint8_t num_pgdn = 0x72;
		constexpr uint8_t num_enter = 0x5a;
		constexpr uint8_t num_ins = 0x5e;
		constexpr uint8_t num_del = 0x32;

		constexpr uint8_t cur_up = 0x14;
		constexpr uint8_t cur_left = 0x18;
		constexpr uint8_t cur_down = 0x1b;
		constexpr uint8_t cur_right = 0x1c;

		constexpr uint8_t special = help;
	}
}