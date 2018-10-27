#include "keymap.h"

namespace keyboard {

	const PROGMEM KeyUsage keymap_flash[0x7F] =
#include "keymap.inc"
	;

	EEMEM KeyUsage keymap_eeprom[0x7F] =
#include "keymap.inc"
	;

	EEMEM uint8_t keymap_eeprom_version = 2;

}