#include "keymap.h"

extern const PROGMEM KeyUsage keymap[0x80] = {
	KeyUsage::RESERVED,
	KeyUsage::STOP,
	KeyUsage::VOLUME_DOWN,
	KeyUsage::AGAIN,
	KeyUsage::VOLUME_UP,
	KeyUsage::F1,
	KeyUsage::F2,
	KeyUsage::F10,
	KeyUsage::F3,
	KeyUsage::F11,
	KeyUsage::F4,
	KeyUsage::F12,
	KeyUsage::F5,
	KeyUsage::RIGHTALT,
	KeyUsage::F6,
	KeyUsage::ESCAPE, // Was the unlabeled key cap, now has escape cap
	KeyUsage::F7,
	KeyUsage::F8,
	KeyUsage::F9,
	KeyUsage::LEFTALT,
	KeyUsage::UP,
	KeyUsage::PAUSE,
	KeyUsage::PRINTSCREEN,
	KeyUsage::SCROLLLOCK,
	KeyUsage::LEFT,
	KeyUsage::RESERVED,//KeyUsage::PROPS,
	KeyUsage::UNDO,
	KeyUsage::DOWN,
	KeyUsage::RIGHT,
	KeyUsage::BACKQUOTE, // Was escape cap, now backquote cap
	KeyUsage::N1,
	KeyUsage::N2,
	KeyUsage::N3,
	KeyUsage::N4,
	KeyUsage::N5,
	KeyUsage::N6,
	KeyUsage::N7,
	KeyUsage::N8,
	KeyUsage::N9,
	KeyUsage::N0,
	KeyUsage::MINUS,
	KeyUsage::EQ,
	KeyUsage::RESERVED, // was backquote cap, now unlabeled (used as FN)
	KeyUsage::BACKSPACE,
	KeyUsage::INSERT,
	KeyUsage::MUTE,
	KeyUsage::NUMPAD_SLASH,
	KeyUsage::NUMPAD_ASTERISK,
	KeyUsage::POWER,
	KeyUsage::RESERVED,//KeyUsage::FRONT,
	KeyUsage::NUMPAD_PERIOD,
	KeyUsage::COPY,
	KeyUsage::HOME,
	KeyUsage::TAB,
	KeyUsage::Q,
	KeyUsage::W,
	KeyUsage::E,
	KeyUsage::R,
	KeyUsage::T,
	KeyUsage::Y,
	KeyUsage::U,
	KeyUsage::I,
	KeyUsage::O,
	KeyUsage::P,
	KeyUsage::LBRACKET,
	KeyUsage::RBRACKET,
	KeyUsage::DELETE,
	KeyUsage::COMPOSE,
	KeyUsage::NUMPAD_7,
	KeyUsage::NUMPAD_8,
	KeyUsage::NUMPAD_9,
	KeyUsage::NUMPAD_MINUS,
	KeyUsage::RESERVED,//KeyUsage::OPEN,
	KeyUsage::PASTE,
	KeyUsage::END,
	KeyUsage::RESERVED,
	KeyUsage::LEFTCTRL,
	KeyUsage::A,
	KeyUsage::S,
	KeyUsage::D,
	KeyUsage::F,
	KeyUsage::G,
	KeyUsage::H,
	KeyUsage::J,
	KeyUsage::K,
	KeyUsage::L,
	KeyUsage::SEMICOLON,
	KeyUsage::QUOTE,
	KeyUsage::BACKSLASH,
	KeyUsage::ENTER,
	KeyUsage::LEFT,
	KeyUsage::NUMPAD_4,
	KeyUsage::NUMPAD_5,
	KeyUsage::NUMPAD_6,
	KeyUsage::NUMPAD_0,
	KeyUsage::FIND,
	KeyUsage::PAGEUP,
	KeyUsage::CUT,
	KeyUsage::NUMLOCK,
	KeyUsage::LEFTSHIFT,
	KeyUsage::Z,
	KeyUsage::X,
	KeyUsage::C,
	KeyUsage::V,
	KeyUsage::B,
	KeyUsage::N,
	KeyUsage::M,
	KeyUsage::COMMA,
	KeyUsage::PERIOD,
	KeyUsage::SLASH,
	KeyUsage::RIGHTSHIFT,
	KeyUsage::NUMPAD_ENTER,
	KeyUsage::NUMPAD_1,
	KeyUsage::NUMPAD_2,
	KeyUsage::NUMPAD_3,
	KeyUsage::RESERVED,
	KeyUsage::RESERVED,
	KeyUsage::RESERVED,
	KeyUsage::HELP,
	KeyUsage::CAPSLOCK,
	KeyUsage::LEFTGUI,
	KeyUsage::SPACE,
	KeyUsage::RIGHTGUI,
	KeyUsage::PAGEDOWN,
	KeyUsage::RESERVED,
	KeyUsage::NUMPAD_PLUS,
	KeyUsage::RESERVED,
	KeyUsage::RESERVED,
};
