#pragma once

#include <stdint.h>

template<int>
struct tagsize;

template<>
struct tagsize<0> {
	static constexpr unsigned value = 0;
};
template<>
struct tagsize<1> {
	static constexpr unsigned value = 1;
};
template<>
struct tagsize<2> {
	static constexpr unsigned value = 2;
};
template<>
struct tagsize<4> {
	static constexpr unsigned value = 3;
};

template<uint8_t ... Args>
constexpr uint8_t mk_tag(int code) noexcept {
	return static_cast<uint8_t>(code | tagsize<sizeof...(Args)>::value);
}

#define MK_TAG(val, ...)    mk_tag<__VA_ARGS__>(val), ##__VA_ARGS__

/* ------------------------------------------------------------------------- */
// Main item tags
#define INPUT(...)          MK_TAG(0b1000'0000, ##__VA_ARGS__)
#define OUTPUT(...)         MK_TAG(0b1001'0000, ##__VA_ARGS__)
#define FEATURE(...)        MK_TAG(0b1011'0000, ##__VA_ARGS__)
#define COLLECTION(...)     MK_TAG(0b1010'0000, ##__VA_ARGS__)
#define END_COLLECTION()    MK_TAG(0b1100'0000)

// Global item tags
#define USAGE_PAGE(...)   MK_TAG(0b0000'0100, ##__VA_ARGS__)
#define LOGICAL_MIN(...)  MK_TAG(0b0001'0100, ##__VA_ARGS__)
#define LOGICAL_MAX(...)  MK_TAG(0b0010'0100, ##__VA_ARGS__)
#define PHYSICAL_MIN(...) MK_TAG(0b0011'0100, ##__VA_ARGS__)
#define PHYSICAL_MAX(...) MK_TAG(0b0100'0100, ##__VA_ARGS__)
#define REPORT_SIZE(...)  MK_TAG(0b0111'0100, ##__VA_ARGS__)
#define REPORT_ID(...)    MK_TAG(0b1000'0100, ##__VA_ARGS__)
#define REPORT_COUNT(...) MK_TAG(0b1001'0100, ##__VA_ARGS__)

// Local item tags
#define USAGE(...)     MK_TAG(0b0000'1000, ##__VA_ARGS__)
#define USAGE_MIN(...) MK_TAG(0b0001'1000, ##__VA_ARGS__)
#define USAGE_MAX(...) MK_TAG(0b0010'1000, ##__VA_ARGS__)

template <typename E>
constexpr uint8_t as_byte(E e) noexcept
{
	return static_cast<uint8_t>(e);
}

namespace MainFlag {
	constexpr uint8_t Constant = 0x01, Data = 0x00;
	constexpr uint8_t Variable = 0x02, Array = 0x00;
	constexpr uint8_t Relative = 0x04, Absolute = 0x00;
	constexpr uint8_t Wrap = 0x08, NoWrap = 0x00;
	constexpr uint8_t Nonlinear = 0x10, Linear = 0x00;
	constexpr uint8_t NoPreferred = 0x20, Preferred = 0x00;
	constexpr uint8_t NullState = 0x40, NoNullState = 0x00;
	constexpr uint8_t Volatile = (uint8_t)0x80, NonVolatile = 0x00;
};

namespace Collection {
	constexpr uint8_t Physical = 0x00;
	constexpr uint8_t Application = 0x01;
	constexpr uint8_t Logical = 0x02;
	constexpr uint8_t Report = 0x03;
	constexpr uint8_t NamedArray = 0x04;
	constexpr uint8_t UsageSwitch = 0x05;
	constexpr uint8_t UsageModifier = 0x06;
}

namespace UsagePage {
	constexpr uint8_t Undefined = 0x00;
	constexpr uint8_t GenericDesktop = 0x01;
	constexpr uint8_t SimulationControls = 0x02;
	constexpr uint8_t VRControls = 0x03;
	constexpr uint8_t SportControls = 0x04;
	constexpr uint8_t GameControls = 0x05;
	constexpr uint8_t GenericDeviceControls = 0x06;
	constexpr uint8_t Keyboard = 0x07;
	constexpr uint8_t LEDs = 0x08;
	constexpr uint8_t Button = 0x09;
	constexpr uint8_t Ordinal = 0x0A;
};

namespace GenericDesktop {
	constexpr uint8_t Undefined = 0x00;
	constexpr uint8_t Pointer = 0x01;
	constexpr uint8_t Mouse = 0x02;

	constexpr uint8_t JoyStick = 0x04;
	constexpr uint8_t GamePad = 0x05;
	constexpr uint8_t Keyboard = 0x06;
	constexpr uint8_t Keypad = 0x07;
	constexpr uint8_t MultiaxisController = 0x08;

	constexpr uint8_t X = 0x30;
	constexpr uint8_t Y = 0x31;
	constexpr uint8_t Z = 0x32;

	constexpr uint8_t Wheel = 0x38;
};

enum class KeyUsage : uint8_t {
	RESERVED = 0x00,
	ERROR_ROLLOVER = 0x01,
	ERROR_POST_FAIL = 0x02,
	ERROR_UNDEFINED = 0x03,
	A = 0x04,
	B = 0x05,
	C = 0x06,
	D = 0x07,
	E = 0x08,
	F = 0x09,
	G = 0x0A,
	H = 0x0B,
	I = 0x0C,
	J = 0x0D,
	K = 0x0E,
	L = 0x0F,
	M = 0x10,
	N = 0x11,
	O = 0x12,
	P = 0x13,
	Q = 0x14,
	R = 0x15,
	S = 0x16,
	T = 0x17,
	U = 0x18,
	V = 0x19,
	W = 0x1A,
	X = 0x1B,
	Y = 0x1C,
	Z = 0x1D,
	N1 = 0x1E,
	N2 = 0x1F,
	N3 = 0x20,
	N4 = 0x21,
	N5 = 0x22,
	N6 = 0x23,
	N7 = 0x24,
	N8 = 0x25,
	N9 = 0x26,
	N0 = 0x27,
	ENTER = 0x28,
	ESCAPE = 0x29,
	BACKSPACE = 0x2A,
	TAB = 0x2B,
	SPACE = 0x2C,
	MINUS = 0x2D,
	EQ = 0x2E,
	LBRACKET = 0x2F,
	RBRACKET = 0x30,
	BACKSLASH = 0x31,
	POUND = 0x32, //non-US
	SEMICOLON = 0x33,
	QUOTE = 0x34,
	BACKQUOTE = 0x35,
	COMMA = 0x36,
	PERIOD = 0x37,
	SLASH = 0x38,
	CAPSLOCK = 0x39,
	F1 = 0x3A,
	F2 = 0x3B,
	F3 = 0x3C,
	F4 = 0x3D,
	F5 = 0x3E,
	F6 = 0x3F,
	F7 = 0x40,
	F8 = 0x41,
	F9 = 0x42,
	F10 = 0x43,
	F11 = 0x44,
	F12 = 0x45,
	PRINTSCREEN = 0x46,
	SCROLLLOCK = 0x47,
	PAUSE = 0x48,
	INSERT = 0x49,
	HOME = 0x4A,
	PAGEUP = 0x4B,
	DELETE = 0x4C,
	END = 0x4D,
	PAGEDOWN = 0x4E,
	RIGHT = 0x4F,
	LEFT = 0x50,
	DOWN = 0x51,
	UP = 0x52,
	NUMLOCK = 0x53,
	NUMPAD_SLASH = 0x54,
	NUMPAD_ASTERISK = 0x55,
	NUMPAD_MINUS = 0x56,
	NUMPAD_PLUS = 0x57,
	NUMPAD_ENTER = 0x58,
	NUMPAD_1 = 0x59,
	NUMPAD_2 = 0x5A,
	NUMPAD_3 = 0x5B,
	NUMPAD_4 = 0x5C,
	NUMPAD_5 = 0x5D,
	NUMPAD_6 = 0x5E,
	NUMPAD_7 = 0x5F,
	NUMPAD_8 = 0x60,
	NUMPAD_9 = 0x61,
	NUMPAD_0 = 0x62,
	NUMPAD_PERIOD = 0x63,
	BACKSLASH_INT = 0x64, //non-US
	COMPOSE = 0x65,
	POWER = 0x66,
	KEYPAD_EQ = 0x67,
	F13 = 0x68,
	F14 = 0x69,
	F15 = 0x6A,
	F16 = 0x6B,
	F17 = 0x6C,
	F18 = 0x6D,
	F19 = 0x6E,
	F20 = 0x6F,
	F21 = 0x70,
	F22 = 0x71,
	F23 = 0x72,
	F24 = 0x73,
	EXECUTE = 0x74,
	HELP = 0x75,
	MENU = 0x76,
	SELECT = 0x77,
	STOP = 0x78,
	AGAIN = 0x79,
	UNDO = 0x7A,
	CUT = 0x7B,
	COPY = 0x7C,
	PASTE = 0x7D,
	FIND = 0x7E,
	MUTE = 0x7F,
	VOLUME_UP = 0x80,
	VOLUME_DOWN = 0x81,
	// 0x82-0x86
	// 0x87-0x98 International
	// 0x00-0xDD
	// 0xDE-0xDF Reserved
	LEFTCTRL = 0xE0,
	LEFTSHIFT = 0xE1,
	LEFTALT = 0xE2,
	LEFTGUI = 0xE3,
	RIGHTCTRL = 0xE4,
	RIGHTSHIFT = 0xE5,
	RIGHTALT = 0xE6,
	RIGHTGUI = 0xE7,
};

enum class LEDUsage : uint8_t {
	UNDEFINED = 0x00,
	NUMLOCK = 0x01,
	CAPSLOCK,
	SCROLLLOCK,
	COMPOSE,
	KANA,
	POWER,
	SHIFT,
	DND,
	MUTE
};
