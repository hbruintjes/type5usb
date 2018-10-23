//
// Created by harold on 23/10/18.
//
#include "uart.h"

#include <avr/io.h>
#include <avr/interrupt.h>

namespace uart {
	static ring_buffer<8> rx_buffer = ring_buffer<8>();
	static ring_buffer<8> tx_buffer = ring_buffer<8>();

	void init(uint16_t baudrate, uint8_t sampling) {
		// Reset uart
		LINCR = _BV(LSWRES);

		// INTR on RX and TX
		LINENIR = _BV(LENRXOK);// | _BV(LENTXOK);

		LINBTR = /*_BV(LDISR) | */static_cast<uint8_t>(sampling);

		// LINBRR  = ((SYS_CLK /(BAUDRATE*LINBTR))-1)&0x0FFF;
		LINBRR = static_cast<uint16_t>(((F_CPU*10/(baudrate*sampling)+5)/10) - 1) & 0x0FFF;

		// enable UART Full Duplex mode
		LINCR = _BV(LENA) | _BV(LCMD0) | _BV(LCMD1) | _BV(LCMD2);
	}

	bool poll() {
		return !rx_buffer.empty();
	}

	bool full() {
		return rx_buffer.full();
	}

	uint8_t recv() {
		return rx_buffer.pop();
	}

	bool ready() {
		return !tx_buffer.full();
	}

	bool send(uint8_t c1, uint8_t c2) {
		if (tx_buffer.free() < 2) {
			return false;
		}

		tx_buffer.push(c1);
		tx_buffer.push(c2);
		if (!(LINENIR & _BV(LENTXOK))) {
			LINENIR |= _BV(LENTXOK);
			LINDAT = tx_buffer.pop();
		}
		return true;
	}

	bool send(uint8_t c) {
		if (tx_buffer.full()) {
			return false;
		}

		tx_buffer.push(c);
		if (!(LINENIR & _BV(LENTXOK))) {
			LINENIR |= _BV(LENTXOK);
			LINDAT = tx_buffer.pop();
		}
		return true;
	}
}

ISR(LIN_TC_vect) {
	if (LINSIR & _BV(LRXOK)) {
		if (!uart::rx_buffer.full()) {
			uart::rx_buffer.push(LINDAT);
		} else {
			// Drop value
			uint8_t c = LINDAT;
		}
	}
	if (LINSIR & _BV(LTXOK)) {
		if (!uart::tx_buffer.empty()) {
			LINDAT = uart::tx_buffer.pop();
		} else {
			LINENIR &= ~_BV(LENTXOK);
		}
	}
}